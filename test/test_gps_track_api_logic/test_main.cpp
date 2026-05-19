#include <unity.h>
#include "GpsTrackApiLogic.h"

void test_removal_path_for_relative_name() {
  TEST_ASSERT_EQUAL_STRING("/gps/tracks/current_track.csv", gpsTrackEntryPathForRemoval("current_track.csv").c_str());
}

void test_removal_path_for_absolute_existing_path() {
  TEST_ASSERT_EQUAL_STRING("/gps/tracks/current_track.csv", gpsTrackEntryPathForRemoval("/gps/tracks/current_track.csv").c_str());
}

void test_response_name_strips_prefix() {
  TEST_ASSERT_EQUAL_STRING("current_track.csv", gpsTrackEntryNameForResponse("/gps/tracks/current_track.csv").c_str());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_removal_path_for_relative_name);
  RUN_TEST(test_removal_path_for_absolute_existing_path);
  RUN_TEST(test_response_name_strips_prefix);
  return UNITY_END();
}
