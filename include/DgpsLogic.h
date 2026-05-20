#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "Types.h"

#pragma pack(push, 1)
struct DgpsCorrectionPayloadV1 {
  uint8_t magic0;
  uint8_t magic1;
  uint8_t version;
  uint8_t type;
  uint8_t flags;
  uint8_t reserved0;
  uint32_t seq;
  uint32_t originSeq;
  uint32_t txMillis;
  uint16_t gpsYear;
  uint8_t gpsMonth;
  uint8_t gpsDay;
  uint8_t gpsHour;
  uint8_t gpsMinute;
  uint8_t gpsSecond;
  uint8_t gpsCentisecond;
  int32_t originLatE7;
  int32_t originLonE7;
  int32_t originAltCm;
  int32_t rawErrECm;
  int32_t rawErrNCm;
  int32_t rawErrUCm;
  int32_t filtErrECm;
  int32_t filtErrNCm;
  int32_t filtErrUCm;
  int16_t rateECmPerSec;
  int16_t rateNCmPerSec;
  int16_t rateUCmPerSec;
  uint16_t hdopCenti;
  uint16_t fixAgeMs;
  uint8_t satsUsed;
  uint8_t satsView;
  uint16_t avgCn0X10;
  uint16_t cn0Count;
  uint32_t visibleSatSig;
  uint32_t usedSatSig;
  uint16_t originSamples;
  uint16_t filterSamples;
  uint16_t payloadBytes;
  uint32_t crc32;
};
#pragma pack(pop)

static constexpr uint8_t DGPS_FLAG_FIX_VALID = 1 << 0;
static constexpr uint8_t DGPS_FLAG_ORIGIN_LOCKED = 1 << 1;
static constexpr uint8_t DGPS_FLAG_FILTER_READY = 1 << 2;
static constexpr uint8_t DGPS_FLAG_ORIGIN_AVERAGED = 1 << 3;
static constexpr uint8_t DGPS_FLAG_HDOP_GOOD = 1 << 4;
static constexpr uint8_t DGPS_FLAG_UTC_VALID = 1 << 5;
static constexpr uint8_t DGPS_FLAG_FIX_FRESH = 1 << 6;

struct DgpsPacketStats {
  uint32_t badLength = 0;
  uint32_t badHeader = 0;
  uint32_t badPayloadSize = 0;
  uint32_t crcFail = 0;
  uint32_t badFlags = 0;
  uint32_t badQuality = 0;
  uint32_t duplicatePackets = 0;
  uint32_t outOfOrderPackets = 0;
  uint32_t missedPackets = 0;
};

enum class DgpsQualityState {
  NO_ROVER_FIX,
  NO_BASE,
  BASE_STALE,
  BASE_BAD,
  DGPS_DEGRADED,
  DGPS_GOOD,
  DGPS_BEST,
};

struct RoverCorrectionState {
  bool hasRecentPacket = false;
  bool hasValidCorrection = false;
  bool lastPacketQualityOk = false;
  bool baseOriginKnown = false;
  bool baseOriginChanged = false;
  DgpsCorrectionPayloadV1 lastPacket{};
  uint32_t lastReceiveMillis = 0;
  uint32_t lastSeq = 0;
  uint32_t packetsReceived = 0;
  uint32_t packetsMissed = 0;
  uint32_t packetsDuplicate = 0;
  uint32_t packetsOutOfOrder = 0;
  uint32_t packetsCrcFail = 0;
  float lastRssi = 0;
  float lastSnr = 0;
  double baseOriginLatDeg = 0;
  double baseOriginLonDeg = 0;
  double baseOriginAltM = 0;
  int32_t predictedErrECm = 0;
  int32_t predictedErrNCm = 0;
  int32_t predictedErrUCm = 0;
  int32_t roverLiveECm = 0;
  int32_t roverLiveNCm = 0;
  int32_t roverLiveUCm = 0;
  int32_t roverCorrectedECm = 0;
  int32_t roverCorrectedNCm = 0;
  int32_t roverCorrectedUCm = 0;
  bool roverOriginLocked = false;
  int32_t roverOriginECm = 0;
  int32_t roverOriginNCm = 0;
  int32_t roverOriginUCm = 0;
};

struct RoverDriftState {
  int32_t driftECm = 0;
  int32_t driftNCm = 0;
  int32_t driftUCm = 0;
  double horizontalDistanceCm = 0;
  double bearingDeg = 0;
};

uint32_t dgpsCrc32Bytes(const uint8_t* data, size_t len);
bool handleReceivedCorrection(const uint8_t* buf, size_t len, uint32_t nowMillis, float rssi, float snr, RoverCorrectionState& state, DgpsPacketStats& stats);
bool computeCorrectedRoverPosition(const GpsFix& roverFix, uint32_t nowMillis, RoverCorrectionState& state, double verticalCorrectionWeight = 0.5);
DgpsQualityState evaluateDgpsQuality(const GpsFix& roverFix, uint32_t nowMillis, const RoverCorrectionState& state);
bool lockRoverOrigin(RoverCorrectionState& state);
void clearRoverOrigin(RoverCorrectionState& state);
bool computeRoverDrift(const RoverCorrectionState& state, RoverDriftState& drift);
