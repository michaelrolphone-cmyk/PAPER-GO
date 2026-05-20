#include <unity.h>
#include "PowerApiLogic.h"

void test_build_power_policy_json_contains_fields() {
  PowerPolicy policy;
  policy.lockTimeoutMs = 45000;
  policy.deepSleepTimeoutMs = 180000;
  policy.allowDeepSleep = false;
  policy.deepSleepDurationSec = 90;
  policy.allowWifiInLockScreen = true;

  String json = buildPowerPolicyJson(policy, true);
  TEST_ASSERT_TRUE(json.indexOf("\"configPresent\":true") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"lockTimeoutMs\":45000") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"deepSleepTimeoutMs\":180000") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"allowDeepSleep\":false") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"deepSleepDurationSec\":90") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"allowWifiInLockScreen\":true") >= 0);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_build_power_policy_json_contains_fields);
  return UNITY_END();
}
