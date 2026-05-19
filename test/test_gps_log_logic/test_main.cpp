#include <unity.h>
#include "GpsLogLogic.h"

void test_should_log_first_valid_fix() {
  GpsFix now; now.valid = true; now.lat = 37.1; now.lon = -122.2;
  GpsFix previous;
  TEST_ASSERT_TRUE(shouldLogGpsFix(now, previous, 5));
}

void test_should_not_log_invalid_fix() {
  GpsFix now;
  GpsFix previous;
  TEST_ASSERT_FALSE(shouldLogGpsFix(now, previous, 5));
}

void test_should_respect_minimum_movement() {
  GpsFix previous; previous.valid = true; previous.lat = 37.000000; previous.lon = -122.000000;
  GpsFix now = previous;
  now.lat = 37.000010;
  TEST_ASSERT_FALSE(shouldLogGpsFix(now, previous, 5));
  now.lat = 37.000100;
  TEST_ASSERT_TRUE(shouldLogGpsFix(now, previous, 5));
}

void test_build_csv_line_uses_heading_sentinel_when_unreliable() {
  GpsFix fix; fix.valid = true; fix.epoch = 1234; fix.lat = 1.25; fix.lon = 2.5; fix.altM = 30; fix.speedKmph = 4.5; fix.hdop = 0.9; fix.sats = 7;
  String row = buildGpsTrackCsvLine(fix, 87.3, false);
  TEST_ASSERT_NOT_EQUAL(-1, row.indexOf("1234,1.250000,2.500000,30.0,4.5,-1.0,0.90,7"));
}


void test_build_gps_epoch_seconds() {
  TEST_ASSERT_EQUAL_UINT64(0ULL, buildGpsEpochSeconds(1969, 12, 31, 23, 59, 59));
  TEST_ASSERT_EQUAL_UINT64(0ULL, buildGpsEpochSeconds(2026, 2, 30, 12, 0, 0));
  TEST_ASSERT_EQUAL_UINT64(0ULL, buildGpsEpochSeconds(2026, 13, 1, 0, 0, 0));
  TEST_ASSERT_EQUAL_UINT64(0ULL, buildGpsEpochSeconds(2026, 1, 1, 25, 0, 0));
  TEST_ASSERT_EQUAL_UINT64(0ULL, buildGpsEpochSeconds(2026, 1, 1, 0, 61, 0));
  TEST_ASSERT_EQUAL_UINT64(0ULL, buildGpsEpochSeconds(2026, 1, 1, 0, 0, 70));

  TEST_ASSERT_EQUAL_UINT64(0ULL, buildGpsEpochSeconds(1970, 1, 1, 0, 0, 0));
  TEST_ASSERT_EQUAL_UINT64(86400ULL, buildGpsEpochSeconds(1970, 1, 2, 0, 0, 0));
  TEST_ASSERT_EQUAL_UINT64(946684800ULL, buildGpsEpochSeconds(2000, 1, 1, 0, 0, 0));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_should_log_first_valid_fix);
  RUN_TEST(test_should_not_log_invalid_fix);
  RUN_TEST(test_should_respect_minimum_movement);
  RUN_TEST(test_build_csv_line_uses_heading_sentinel_when_unreliable);
  RUN_TEST(test_build_gps_epoch_seconds);
  return UNITY_END();
}
