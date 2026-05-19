#include <unity.h>
#include "GpsConfigLogic.h"

void test_default_false_when_missing_key() {
  TEST_ASSERT_FALSE(parseGpsTrackLoggingEnabled("{}", false));
}

void test_parses_true_and_false() {
  TEST_ASSERT_TRUE(parseGpsTrackLoggingEnabled("{\"gpsTrackLogging\":true}", false));
  TEST_ASSERT_FALSE(parseGpsTrackLoggingEnabled("{\"gpsTrackLogging\":false}", true));
}

void test_handles_whitespace() {
  TEST_ASSERT_TRUE(parseGpsTrackLoggingEnabled("{\"gpsTrackLogging\" :   true, \"x\":1}", false));
}


void test_build_config_json() {
  TEST_ASSERT_EQUAL_STRING("{\"gpsTrackLogging\":true}", buildGpsTrackLoggingConfigJson(true).c_str());
  TEST_ASSERT_EQUAL_STRING("{\"gpsTrackLogging\":false}", buildGpsTrackLoggingConfigJson(false).c_str());
}

void test_update_existing_key() {
  String updated;
  TEST_ASSERT_TRUE(updateGpsTrackLoggingEnabled("{\"a\":1,\"gpsTrackLogging\":false,\"b\":2}", true, updated));
  TEST_ASSERT_NOT_EQUAL(-1, updated.indexOf("\"gpsTrackLogging\":true"));
}

void test_update_missing_key_builds_minimal_json() {
  String updated;
  TEST_ASSERT_TRUE(updateGpsTrackLoggingEnabled("{\"a\":1}", true, updated));
  TEST_ASSERT_EQUAL_STRING("{\"gpsTrackLogging\":true}", updated.c_str());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_default_false_when_missing_key);
  RUN_TEST(test_parses_true_and_false);
  RUN_TEST(test_handles_whitespace);
  RUN_TEST(test_build_config_json);
  RUN_TEST(test_update_existing_key);
  RUN_TEST(test_update_missing_key_builds_minimal_json);
  return UNITY_END();
}
