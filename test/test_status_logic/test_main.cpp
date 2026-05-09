#include <unity.h>
#include "StatusLogic.h"

void test_time_source_priority() {
  TEST_ASSERT_EQUAL(TimeSource::RTC, resolveTimeSource({true, true, true, true}));
  TEST_ASSERT_EQUAL(TimeSource::GPS, resolveTimeSource({false, true, true, true}));
  TEST_ASSERT_EQUAL(TimeSource::Network, resolveTimeSource({false, false, true, true}));
  TEST_ASSERT_EQUAL(TimeSource::Saved, resolveTimeSource({false, false, false, true}));
  TEST_ASSERT_EQUAL(TimeSource::Unknown, resolveTimeSource({false, false, false, false}));
}

void test_gps_status_states() {
  GpsFix fix{};
  TEST_ASSERT_EQUAL(GpsStatusState::Off, deriveGpsStatus(fix, 15000, false));
  TEST_ASSERT_EQUAL(GpsStatusState::Searching, deriveGpsStatus(fix, 15000, true));

  fix.valid = true;
  fix.sats = 4;
  fix.ageMs = 500;
  TEST_ASSERT_EQUAL(GpsStatusState::Fix3D, deriveGpsStatus(fix, 15000, true));

  fix.sats = 3;
  TEST_ASSERT_EQUAL(GpsStatusState::Fix2D, deriveGpsStatus(fix, 15000, true));

  fix.sats = 1;
  TEST_ASSERT_EQUAL(GpsStatusState::NoData, deriveGpsStatus(fix, 15000, true));

  fix.ageMs = 20000;
  TEST_ASSERT_EQUAL(GpsStatusState::Degraded, deriveGpsStatus(fix, 15000, true));
}


void test_labels() {
  TEST_ASSERT_EQUAL_STRING("RTC", timeSourceLabel(TimeSource::RTC));
  TEST_ASSERT_EQUAL_STRING("GPS", timeSourceLabel(TimeSource::GPS));
  TEST_ASSERT_EQUAL_STRING("NET", timeSourceLabel(TimeSource::Network));
  TEST_ASSERT_EQUAL_STRING("SAVED", timeSourceLabel(TimeSource::Saved));
  TEST_ASSERT_EQUAL_STRING("search", gpsStatusLabel(GpsStatusState::Searching));
  TEST_ASSERT_EQUAL_STRING("3D", gpsStatusLabel(GpsStatusState::Fix3D));
  TEST_ASSERT_EQUAL_STRING("stale", gpsStatusLabel(GpsStatusState::Degraded));
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_time_source_priority);
  RUN_TEST(test_gps_status_states);
  RUN_TEST(test_labels);
  return UNITY_END();
}
