#include "DgpsLogic.h"
#include <cmath>
#include <cstring>

namespace {
static constexpr uint8_t REQUIRED_FLAGS = DGPS_FLAG_FIX_VALID | DGPS_FLAG_ORIGIN_LOCKED | DGPS_FLAG_FILTER_READY | DGPS_FLAG_HDOP_GOOD | DGPS_FLAG_UTC_VALID | DGPS_FLAG_FIX_FRESH;
static constexpr uint16_t MAX_BASE_HDOP_CENTI = 300;
static constexpr uint16_t MAX_BASE_FIX_AGE_MS = 5000;
static constexpr uint8_t MIN_BASE_SATS = 5;
static constexpr uint16_t MIN_ORIGIN_SAMPLES = 5;
static constexpr uint16_t MIN_FILTER_SAMPLES = 1;
static constexpr uint32_t MAX_CORRECTION_AGE_MS = 5000;
static constexpr double EARTH_RADIUS_M = 6378137.0;
static constexpr uint32_t FULL_CONFIDENCE_AGE_MS = 2000;

double clampVerticalWeight(double weight) {
  if (weight < 0.0) return 0.0;
  if (weight > 1.0) return 1.0;
  return weight;
}

static_assert(sizeof(DgpsCorrectionPayloadV1) == 96, "DgpsCorrectionPayloadV1 size must match protocol spec");

void latLonDeltaMeters(double lat0, double lon0, double lat1, double lon1, double& eastM, double& northM) {
  const double dLat = (lat1 - lat0) * PI / 180.0;
  const double dLon = (lon1 - lon0) * PI / 180.0;
  const double lat0Rad = lat0 * PI / 180.0;
  northM = dLat * EARTH_RADIUS_M;
  eastM = dLon * EARTH_RADIUS_M * cos(lat0Rad);
}
}

uint32_t dgpsCrc32Bytes(const uint8_t* data, size_t len) {
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc & 1u) ? (crc >> 1u) ^ 0xEDB88320u : (crc >> 1u);
    }
  }
  return ~crc;
}

bool handleReceivedCorrection(const uint8_t* buf, size_t len, uint32_t nowMillis, float rssi, float snr, RoverCorrectionState& state, DgpsPacketStats& stats) {
  if (len != sizeof(DgpsCorrectionPayloadV1)) { stats.badLength++; return false; }
  DgpsCorrectionPayloadV1 pkt{};
  memcpy(&pkt, buf, sizeof(pkt));
  if (pkt.magic0 != 'D' || pkt.magic1 != 'G' || pkt.version != 1 || pkt.type != 1) { stats.badHeader++; return false; }
  if (pkt.payloadBytes != sizeof(DgpsCorrectionPayloadV1) - sizeof(pkt.crc32)) { stats.badPayloadSize++; return false; }
  uint32_t computed = dgpsCrc32Bytes(reinterpret_cast<const uint8_t*>(&pkt), sizeof(DgpsCorrectionPayloadV1) - sizeof(pkt.crc32));
  if (computed != pkt.crc32) { stats.crcFail++; state.packetsCrcFail++; return false; }
  if ((pkt.flags & REQUIRED_FLAGS) != REQUIRED_FLAGS) { stats.badFlags++; return false; }
  const bool qualityOk = !(pkt.hdopCenti > MAX_BASE_HDOP_CENTI || pkt.fixAgeMs > MAX_BASE_FIX_AGE_MS || pkt.satsUsed < MIN_BASE_SATS || pkt.originSamples < MIN_ORIGIN_SAMPLES || pkt.filterSamples < MIN_FILTER_SAMPLES);

  if (state.packetsReceived > 0) {
    const uint32_t diff = pkt.seq - state.lastSeq;
    if (diff == 0) { stats.duplicatePackets++; state.packetsDuplicate++; return false; }
    if (diff >= 0x80000000u) { stats.outOfOrderPackets++; state.packetsOutOfOrder++; return false; }
    if (diff > 1) { uint32_t missed = diff - 1; stats.missedPackets += missed; state.packetsMissed += missed; }
  }

  if (!state.baseOriginKnown || pkt.originSeq != state.lastPacket.originSeq) {
    state.baseOriginChanged = true;
    state.baseOriginKnown = true;
    state.baseOriginLatDeg = static_cast<double>(pkt.originLatE7) / 10000000.0;
    state.baseOriginLonDeg = static_cast<double>(pkt.originLonE7) / 10000000.0;
    state.baseOriginAltM = static_cast<double>(pkt.originAltCm) / 100.0;
    state.roverOriginLocked = false;
  } else {
    state.baseOriginChanged = false;
  }

  state.lastPacket = pkt;
  state.hasRecentPacket = true;
  state.lastPacketQualityOk = qualityOk;
  state.lastReceiveMillis = nowMillis;
  state.lastSeq = pkt.seq;
  state.lastRssi = rssi;
  state.lastSnr = snr;
  state.hasValidCorrection = qualityOk;
  state.packetsReceived++;
  if (!qualityOk) {
    stats.badQuality++;
    return false;
  }
  return true;
}

bool computeCorrectedRoverPosition(const GpsFix& roverFix, uint32_t nowMillis, RoverCorrectionState& state, double verticalCorrectionWeight) {
  if (!state.hasValidCorrection || !state.baseOriginKnown || !roverFix.valid) return false;
  if (roverFix.ageMs > MAX_BASE_FIX_AGE_MS || roverFix.hdop > 3.0 || roverFix.sats < MIN_BASE_SATS || roverFix.epoch == 0) return false;
  uint32_t ageMs = nowMillis - state.lastReceiveMillis;
  if (ageMs > MAX_CORRECTION_AGE_MS) return false;

  double eastM = 0.0, northM = 0.0;
  latLonDeltaMeters(state.baseOriginLatDeg, state.baseOriginLonDeg, roverFix.lat, roverFix.lon, eastM, northM);
  int32_t liveECm = static_cast<int32_t>(round(eastM * 100.0));
  int32_t liveNCm = static_cast<int32_t>(round(northM * 100.0));
  int32_t liveUCm = static_cast<int32_t>(round((roverFix.altM - state.baseOriginAltM) * 100.0));
  double ageSec = static_cast<double>(ageMs) / 1000.0;

  int32_t predErrECm = static_cast<int32_t>(round(state.lastPacket.filtErrECm + state.lastPacket.rateECmPerSec * ageSec));
  int32_t predErrNCm = static_cast<int32_t>(round(state.lastPacket.filtErrNCm + state.lastPacket.rateNCmPerSec * ageSec));
  const double boundedWeight = clampVerticalWeight(verticalCorrectionWeight);
  int32_t predErrUCm = static_cast<int32_t>(round((state.lastPacket.filtErrUCm + state.lastPacket.rateUCmPerSec * ageSec) * boundedWeight));

  state.roverLiveECm = liveECm; state.roverLiveNCm = liveNCm; state.roverLiveUCm = liveUCm;
  state.predictedErrECm = predErrECm; state.predictedErrNCm = predErrNCm; state.predictedErrUCm = predErrUCm;
  state.roverCorrectedECm = liveECm - predErrECm;
  state.roverCorrectedNCm = liveNCm - predErrNCm;
  state.roverCorrectedUCm = liveUCm - predErrUCm;
  return true;
}

DgpsQualityState evaluateDgpsQuality(const GpsFix& roverFix, uint32_t nowMillis, const RoverCorrectionState& state) {
  if (!roverFix.valid || roverFix.epoch == 0 || roverFix.ageMs > MAX_BASE_FIX_AGE_MS || roverFix.hdop > 3.0 || roverFix.sats < MIN_BASE_SATS) {
    return DgpsQualityState::NO_ROVER_FIX;
  }
  if (!state.hasRecentPacket || !state.baseOriginKnown) return DgpsQualityState::NO_BASE;

  const uint32_t ageMs = nowMillis - state.lastReceiveMillis;
  if (ageMs > MAX_CORRECTION_AGE_MS) return DgpsQualityState::BASE_STALE;

  const DgpsCorrectionPayloadV1& p = state.lastPacket;
  if (!state.lastPacketQualityOk) {
    return DgpsQualityState::BASE_BAD;
  }

  const bool best = (p.flags & DGPS_FLAG_ORIGIN_AVERAGED) != 0 &&
                    p.originSamples >= 30 &&
                    p.filterSamples >= 7 &&
                    p.hdopCenti <= 150 &&
                    p.satsUsed >= 8 &&
                    ageMs <= FULL_CONFIDENCE_AGE_MS;
  if (best) return DgpsQualityState::DGPS_BEST;
  if (ageMs <= FULL_CONFIDENCE_AGE_MS) return DgpsQualityState::DGPS_GOOD;
  return DgpsQualityState::DGPS_DEGRADED;
}

bool lockRoverOrigin(RoverCorrectionState& state) {
  if (!state.hasValidCorrection || !state.baseOriginKnown) return false;
  state.roverOriginECm = state.roverCorrectedECm;
  state.roverOriginNCm = state.roverCorrectedNCm;
  state.roverOriginUCm = state.roverCorrectedUCm;
  state.roverOriginLocked = true;
  return true;
}

void clearRoverOrigin(RoverCorrectionState& state) {
  state.roverOriginLocked = false;
  state.roverOriginECm = 0;
  state.roverOriginNCm = 0;
  state.roverOriginUCm = 0;
}

bool computeRoverDrift(const RoverCorrectionState& state, RoverDriftState& drift) {
  if (!state.roverOriginLocked) return false;
  drift.driftECm = state.roverCorrectedECm - state.roverOriginECm;
  drift.driftNCm = state.roverCorrectedNCm - state.roverOriginNCm;
  drift.driftUCm = state.roverCorrectedUCm - state.roverOriginUCm;
  drift.horizontalDistanceCm = sqrt(static_cast<double>(drift.driftECm) * drift.driftECm + static_cast<double>(drift.driftNCm) * drift.driftNCm);
  const double rawBearing = atan2(static_cast<double>(drift.driftECm), static_cast<double>(drift.driftNCm)) * 180.0 / PI;
  drift.bearingDeg = rawBearing < 0.0 ? rawBearing + 360.0 : rawBearing;
  return true;
}
