#include <unity.h>
#include <string.h>
#include "DgpsLogic.h"

static DgpsCorrectionPayloadV1 makeValidPkt(uint32_t seq = 1, uint32_t originSeq = 55) {
  DgpsCorrectionPayloadV1 p{};
  p.magic0 = 'D'; p.magic1 = 'G'; p.version = 1; p.type = 1;
  p.flags = DGPS_FLAG_FIX_VALID | DGPS_FLAG_ORIGIN_LOCKED | DGPS_FLAG_FILTER_READY | DGPS_FLAG_HDOP_GOOD | DGPS_FLAG_UTC_VALID | DGPS_FLAG_FIX_FRESH;
  p.seq = seq; p.originSeq = originSeq; p.originLatE7 = 377749000; p.originLonE7 = -1224194000; p.originAltCm = 1500;
  p.filtErrECm = 100; p.filtErrNCm = -200; p.filtErrUCm = 40;
  p.rateECmPerSec = 10; p.rateNCmPerSec = 20; p.rateUCmPerSec = 30;
  p.hdopCenti = 90; p.fixAgeMs = 250; p.satsUsed = 9; p.originSamples = 40; p.filterSamples = 8;
  p.payloadBytes = sizeof(DgpsCorrectionPayloadV1) - sizeof(p.crc32);
  p.crc32 = dgpsCrc32Bytes(reinterpret_cast<const uint8_t*>(&p), sizeof(DgpsCorrectionPayloadV1) - sizeof(p.crc32));
  return p;
}

void test_accepts_valid_packet_and_updates_state() {
  RoverCorrectionState state{}; DgpsPacketStats stats{};
  auto p = makeValidPkt();
  bool ok = handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p), sizeof(p), 1000, -90.0f, 8.2f, state, stats);
  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_TRUE(state.hasValidCorrection);
  TEST_ASSERT_EQUAL_UINT32(1, state.packetsReceived);
  TEST_ASSERT_EQUAL_UINT32(1, state.lastSeq);
  TEST_ASSERT_FLOAT_WITHIN(0.00001, 37.7749, state.baseOriginLatDeg);
}

void test_rejects_crc_failure() {
  RoverCorrectionState state{}; DgpsPacketStats stats{};
  auto p = makeValidPkt(); p.crc32 ^= 0xABCD;
  bool ok = handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p), sizeof(p), 1000, -91.0f, 7.0f, state, stats);
  TEST_ASSERT_FALSE(ok);
  TEST_ASSERT_EQUAL_UINT32(1, stats.crcFail);
  TEST_ASSERT_FALSE(state.hasValidCorrection);
}

void test_sequence_tracking_and_duplicate_rejection() {
  RoverCorrectionState state{}; DgpsPacketStats stats{};
  auto p1 = makeValidPkt(10, 1);
  auto p2 = makeValidPkt(13, 1);
  auto dup = makeValidPkt(13, 1);
  TEST_ASSERT_TRUE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p1), sizeof(p1), 1000, -89.0f, 9.0f, state, stats));
  TEST_ASSERT_TRUE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p2), sizeof(p2), 1100, -88.0f, 8.8f, state, stats));
  TEST_ASSERT_EQUAL_UINT32(2, state.packetsMissed);
  TEST_ASSERT_FALSE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&dup), sizeof(dup), 1200, -88.0f, 8.8f, state, stats));
  TEST_ASSERT_EQUAL_UINT32(1, state.packetsDuplicate);
}

void test_compute_corrected_rover_position() {
  RoverCorrectionState state{}; DgpsPacketStats stats{};
  auto p = makeValidPkt(20, 7);
  TEST_ASSERT_TRUE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p), sizeof(p), 1000, -86.0f, 9.5f, state, stats));

  GpsFix fix{};
  fix.valid = true; fix.lat = 37.7750; fix.lon = -122.4193; fix.altM = 16.0;
  fix.hdop = 1.2; fix.sats = 10; fix.ageMs = 200; fix.epoch = 1700000000;

  TEST_ASSERT_TRUE(computeCorrectedRoverPosition(fix, 2500, state, 0.5));
  TEST_ASSERT_TRUE(state.roverLiveECm != 0 || state.roverLiveNCm != 0);
  TEST_ASSERT_EQUAL_INT32(state.roverLiveECm - state.predictedErrECm, state.roverCorrectedECm);
  TEST_ASSERT_EQUAL_INT32(state.roverLiveNCm - state.predictedErrNCm, state.roverCorrectedNCm);
}

void test_sequence_rollover_is_accepted() {
  RoverCorrectionState state{}; DgpsPacketStats stats{};
  auto p1 = makeValidPkt(0xFFFFFFFEu, 1);
  auto p2 = makeValidPkt(0xFFFFFFFFu, 1);
  auto p3 = makeValidPkt(0u, 1);
  TEST_ASSERT_TRUE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p1), sizeof(p1), 1000, -80.0f, 7.0f, state, stats));
  TEST_ASSERT_TRUE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p2), sizeof(p2), 1100, -80.0f, 7.0f, state, stats));
  TEST_ASSERT_TRUE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p3), sizeof(p3), 1200, -80.0f, 7.0f, state, stats));
  TEST_ASSERT_EQUAL_UINT32(0, state.packetsOutOfOrder);
}

void test_vertical_weight_is_clamped_and_origin_drift_works() {
  RoverCorrectionState state{}; DgpsPacketStats stats{};
  auto p = makeValidPkt(100, 7);
  TEST_ASSERT_TRUE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p), sizeof(p), 1000, -86.0f, 9.5f, state, stats));
  GpsFix fix{};
  fix.valid = true; fix.lat = 37.7750; fix.lon = -122.4193; fix.altM = 16.0;
  fix.hdop = 1.2; fix.sats = 10; fix.ageMs = 200; fix.epoch = 1700000000;
  TEST_ASSERT_TRUE(computeCorrectedRoverPosition(fix, 2500, state, 2.5));
  TEST_ASSERT_EQUAL_INT32(85, state.predictedErrUCm);
  TEST_ASSERT_TRUE(lockRoverOrigin(state));
  state.roverCorrectedECm += 10;
  state.roverCorrectedNCm -= 20;
  state.roverCorrectedUCm += 5;
  RoverDriftState drift{};
  TEST_ASSERT_TRUE(computeRoverDrift(state, drift));
  TEST_ASSERT_EQUAL_INT32(10, drift.driftECm);
  TEST_ASSERT_EQUAL_INT32(-20, drift.driftNCm);
  TEST_ASSERT_TRUE(drift.bearingDeg >= 0.0 && drift.bearingDeg < 360.0);
  clearRoverOrigin(state);
  TEST_ASSERT_FALSE(computeRoverDrift(state, drift));
}

void test_quality_state_transitions() {
  RoverCorrectionState state{}; DgpsPacketStats stats{};
  GpsFix fix{};
  fix.valid = true; fix.lat = 37.7750; fix.lon = -122.4193; fix.altM = 16.0;
  fix.hdop = 1.2; fix.sats = 10; fix.ageMs = 200; fix.epoch = 1700000000;

  TEST_ASSERT_EQUAL(static_cast<int>(DgpsQualityState::NO_BASE), static_cast<int>(evaluateDgpsQuality(fix, 2500, state)));

  auto p = makeValidPkt(200, 9);
  p.flags |= DGPS_FLAG_ORIGIN_AVERAGED;
  p.hdopCenti = 120;
  p.satsUsed = 9;
  p.originSamples = 40;
  p.filterSamples = 8;
  p.crc32 = dgpsCrc32Bytes(reinterpret_cast<const uint8_t*>(&p), sizeof(DgpsCorrectionPayloadV1) - sizeof(p.crc32));

  TEST_ASSERT_TRUE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p), sizeof(p), 1000, -86.0f, 9.5f, state, stats));
  TEST_ASSERT_EQUAL(static_cast<int>(DgpsQualityState::DGPS_BEST), static_cast<int>(evaluateDgpsQuality(fix, 2500, state)));
  TEST_ASSERT_EQUAL(static_cast<int>(DgpsQualityState::DGPS_DEGRADED), static_cast<int>(evaluateDgpsQuality(fix, 4501, state)));
  TEST_ASSERT_EQUAL(static_cast<int>(DgpsQualityState::BASE_STALE), static_cast<int>(evaluateDgpsQuality(fix, 7001, state)));

  fix.valid = false;
  TEST_ASSERT_EQUAL(static_cast<int>(DgpsQualityState::NO_ROVER_FIX), static_cast<int>(evaluateDgpsQuality(fix, 2500, state)));
}

void test_bad_quality_packet_updates_base_state_without_valid_correction() {
  RoverCorrectionState state{}; DgpsPacketStats stats{};
  auto p = makeValidPkt(300, 4);
  p.hdopCenti = 450;
  p.crc32 = dgpsCrc32Bytes(reinterpret_cast<const uint8_t*>(&p), sizeof(DgpsCorrectionPayloadV1) - sizeof(p.crc32));

  TEST_ASSERT_FALSE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p), sizeof(p), 1000, -90.0f, 5.0f, state, stats));
  TEST_ASSERT_TRUE(state.hasRecentPacket);
  TEST_ASSERT_FALSE(state.hasValidCorrection);
  TEST_ASSERT_FALSE(state.lastPacketQualityOk);
  TEST_ASSERT_EQUAL_UINT32(1, stats.badQuality);

  GpsFix fix{};
  fix.valid = true; fix.lat = 37.7750; fix.lon = -122.4193; fix.altM = 16.0;
  fix.hdop = 1.2; fix.sats = 10; fix.ageMs = 200; fix.epoch = 1700000000;
  TEST_ASSERT_EQUAL(static_cast<int>(DgpsQualityState::BASE_BAD), static_cast<int>(evaluateDgpsQuality(fix, 1500, state)));
}

void test_large_forward_jump_counts_missed_not_out_of_order() {
  RoverCorrectionState state{}; DgpsPacketStats stats{};
  auto p1 = makeValidPkt(1u, 1u);
  auto p2 = makeValidPkt(0x7FFFFFFFu, 1u);
  TEST_ASSERT_TRUE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p1), sizeof(p1), 1000, -90.0f, 5.0f, state, stats));
  TEST_ASSERT_TRUE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p2), sizeof(p2), 1100, -90.0f, 5.0f, state, stats));
  TEST_ASSERT_EQUAL_UINT32(0, state.packetsOutOfOrder);
  TEST_ASSERT_EQUAL_UINT32(0x7FFFFFFDu, state.packetsMissed);
}

void test_backward_sequence_is_out_of_order() {
  RoverCorrectionState state{}; DgpsPacketStats stats{};
  auto p1 = makeValidPkt(100u, 1u);
  auto p2 = makeValidPkt(90u, 1u);
  TEST_ASSERT_TRUE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p1), sizeof(p1), 1000, -90.0f, 5.0f, state, stats));
  TEST_ASSERT_FALSE(handleReceivedCorrection(reinterpret_cast<const uint8_t*>(&p2), sizeof(p2), 1100, -90.0f, 5.0f, state, stats));
  TEST_ASSERT_EQUAL_UINT32(1, state.packetsOutOfOrder);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_accepts_valid_packet_and_updates_state);
  RUN_TEST(test_rejects_crc_failure);
  RUN_TEST(test_sequence_tracking_and_duplicate_rejection);
  RUN_TEST(test_compute_corrected_rover_position);
  RUN_TEST(test_sequence_rollover_is_accepted);
  RUN_TEST(test_vertical_weight_is_clamped_and_origin_drift_works);
  RUN_TEST(test_quality_state_transitions);
  RUN_TEST(test_bad_quality_packet_updates_base_state_without_valid_correction);
  RUN_TEST(test_large_forward_jump_counts_missed_not_out_of_order);
  RUN_TEST(test_backward_sequence_is_out_of_order);
  return UNITY_END();
}
