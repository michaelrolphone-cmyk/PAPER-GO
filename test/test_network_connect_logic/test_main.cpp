#include <unity.h>
#include "NetworkConnectLogic.h"

void test_no_cache_means_no_saved_connect_attempt() {
  TEST_ASSERT_FALSE(shouldAttemptSavedWifiConnect("{\"ssid\":\"A\",\"password\":\"B\"}", false));
}

void test_valid_config_attempts_connect() {
  TEST_ASSERT_TRUE(shouldAttemptSavedWifiConnect("{\"ssid\":\"FieldNet\",\"password\":\"pw\"}", true));
}

void test_invalid_config_does_not_attempt_connect() {
  TEST_ASSERT_FALSE(shouldAttemptSavedWifiConnect("{\"password\":\"pw\"}", true));
  TEST_ASSERT_FALSE(shouldAttemptSavedWifiConnect("{}", true));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_no_cache_means_no_saved_connect_attempt);
  RUN_TEST(test_valid_config_attempts_connect);
  RUN_TEST(test_invalid_config_does_not_attempt_connect);
  return UNITY_END();
}
