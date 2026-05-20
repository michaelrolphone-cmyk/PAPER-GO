#include "DgpsApiLogic.h"

String dgpsQualityStateLabel(DgpsQualityState state) {
  switch (state) {
    case DgpsQualityState::NO_ROVER_FIX: return "NO_ROVER_FIX";
    case DgpsQualityState::NO_BASE: return "NO_BASE";
    case DgpsQualityState::BASE_STALE: return "BASE_STALE";
    case DgpsQualityState::BASE_BAD: return "BASE_BAD";
    case DgpsQualityState::DGPS_DEGRADED: return "DGPS_DEGRADED";
    case DgpsQualityState::DGPS_GOOD: return "DGPS_GOOD";
    case DgpsQualityState::DGPS_BEST: return "DGPS_BEST";
  }
  return "NO_BASE";
}

String buildDgpsStatusJson(const GpsFix& rawFix, const GpsFix& activeFix, bool usingDgps, DgpsQualityState quality, uint32_t ageMs, const RoverCorrectionState& state, const DgpsPacketStats& stats) {
  String j = "{";
  j += "\"usingDgps\":" + String(usingDgps ? "true" : "false");
  j += ",\"quality\":\"" + dgpsQualityStateLabel(quality) + "\"";
  j += ",\"dgpsAgeMs\":" + String(ageMs);
  j += ",\"raw\":{\"valid\":" + String(rawFix.valid ? "true" : "false") + ",\"lat\":" + String(rawFix.lat, 6) + ",\"lon\":" + String(rawFix.lon, 6) + ",\"hdop\":" + String(rawFix.hdop, 2) + ",\"sats\":" + String(rawFix.sats) + "}";
  j += ",\"active\":{\"valid\":" + String(activeFix.valid ? "true" : "false") + ",\"lat\":" + String(activeFix.lat, 6) + ",\"lon\":" + String(activeFix.lon, 6) + ",\"hdop\":" + String(activeFix.hdop, 2) + ",\"sats\":" + String(activeFix.sats) + "}";
  j += ",\"packets\":{\"received\":" + String(state.packetsReceived) + ",\"missed\":" + String(state.packetsMissed) + ",\"duplicate\":" + String(state.packetsDuplicate) + ",\"outOfOrder\":" + String(state.packetsOutOfOrder) + ",\"crcFail\":" + String(state.packetsCrcFail) + "}";
  j += ",\"radio\":{\"rssi\":" + String(state.lastRssi, 1) + ",\"snr\":" + String(state.lastSnr, 1) + "}";
  j += "}";
  return j;
}

String buildDgpsPacketJson(const RoverCorrectionState& state, uint32_t ageMs) {
  const auto& p = state.lastPacket;
  String j = "{";
  j += "\"hasRecentPacket\":" + String(state.hasRecentPacket ? "true" : "false");
  j += ",\"hasValidCorrection\":" + String(state.hasValidCorrection ? "true" : "false");
  j += ",\"seq\":" + String(p.seq);
  j += ",\"originSeq\":" + String(p.originSeq);
  j += ",\"ageMs\":" + String(ageMs);
  j += ",\"origin\":{\"lat\":" + String(state.baseOriginLatDeg, 7) + ",\"lon\":" + String(state.baseOriginLonDeg, 7) + ",\"altM\":" + String(state.baseOriginAltM, 2) + "}";
  j += ",\"filtErrCm\":{\"e\":" + String(p.filtErrECm) + ",\"n\":" + String(p.filtErrNCm) + ",\"u\":" + String(p.filtErrUCm) + "}";
  j += ",\"predErrCm\":{\"e\":" + String(state.predictedErrECm) + ",\"n\":" + String(state.predictedErrNCm) + ",\"u\":" + String(state.predictedErrUCm) + "}";
  j += ",\"base\":{\"satsUsed\":" + String(p.satsUsed) + ",\"hdopCenti\":" + String(p.hdopCenti) + "}";
  j += ",\"ratesCmPerSec\":{\"e\":" + String(p.rateECmPerSec) + ",\"n\":" + String(p.rateNCmPerSec) + ",\"u\":" + String(p.rateUCmPerSec) + "}";
  j += "}";
  return j;
}
