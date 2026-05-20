#include <unity.h>
#include "DgpsApiLogic.h"

void test_dgps_quality_state_labels_exhaustive() {
  TEST_ASSERT_EQUAL_STRING("NO_ROVER_FIX", dgpsQualityStateLabel(DgpsQualityState::NO_ROVER_FIX).c_str());
  TEST_ASSERT_EQUAL_STRING("NO_BASE", dgpsQualityStateLabel(DgpsQualityState::NO_BASE).c_str());
  TEST_ASSERT_EQUAL_STRING("BASE_STALE", dgpsQualityStateLabel(DgpsQualityState::BASE_STALE).c_str());
  TEST_ASSERT_EQUAL_STRING("BASE_BAD", dgpsQualityStateLabel(DgpsQualityState::BASE_BAD).c_str());
  TEST_ASSERT_EQUAL_STRING("DGPS_DEGRADED", dgpsQualityStateLabel(DgpsQualityState::DGPS_DEGRADED).c_str());
  TEST_ASSERT_EQUAL_STRING("DGPS_GOOD", dgpsQualityStateLabel(DgpsQualityState::DGPS_GOOD).c_str());
  TEST_ASSERT_EQUAL_STRING("DGPS_BEST", dgpsQualityStateLabel(DgpsQualityState::DGPS_BEST).c_str());
}

void test_build_dgps_status_json_fields() {
  GpsFix raw{}; raw.valid=true; raw.lat=1.1; raw.lon=2.2; raw.hdop=0.9; raw.sats=8;
  GpsFix active=raw;
  RoverCorrectionState s{}; s.packetsReceived=9; s.packetsMissed=1; s.lastRssi=-90.5f; s.lastSnr=7.2f;
  DgpsPacketStats stats{}; stats.badQuality = 2;
  String j = buildDgpsStatusJson(raw, active, true, DgpsQualityState::DGPS_GOOD, 1200, s, stats);
  TEST_ASSERT_TRUE(j.indexOf("\"usingDgps\":true") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"quality\":\"DGPS_GOOD\"") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"received\":9") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"badQuality\":2") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"badLength\":0") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"badHeader\":0") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"badPayloadSize\":0") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"badFlags\":0") >= 0);
}

void test_build_dgps_packet_json_fields() {
  RoverCorrectionState s{};
  s.hasRecentPacket = true; s.hasValidCorrection = true; s.baseOriginLatDeg = 10.0; s.baseOriginLonDeg = 20.0;
  s.lastPacket.seq = 5; s.lastPacket.originSeq = 8; s.lastPacket.filtErrECm = 12; s.predictedErrECm = 13;
  s.lastPacket.rateECmPerSec = 4; s.lastPacket.rateNCmPerSec = -2; s.lastPacket.rateUCmPerSec = 1;
  String j = buildDgpsPacketJson(s, 500);
  TEST_ASSERT_TRUE(j.indexOf("\"seq\":5") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"originSeq\":8") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"predErrCm\"") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"ratesCmPerSec\"") >= 0);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_dgps_quality_state_labels_exhaustive);
  RUN_TEST(test_build_dgps_status_json_fields);
  RUN_TEST(test_build_dgps_packet_json_fields);
  return UNITY_END();
}
