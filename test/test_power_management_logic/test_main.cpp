#include <unity.h>
#include "PowerManagementLogic.h"

void test_enters_lock_screen_after_idle_timeout() {
  PowerPolicy p;
  p.lockTimeoutMs = 1000;
  p.deepSleepTimeoutMs = 5000;
  PowerState s;
  s.lastInteractionMs = 0;
  s.lockScreenActive = false;

  TEST_ASSERT_EQUAL(PowerAction::None, evaluatePowerAction(p, s, 999));
  TEST_ASSERT_EQUAL(PowerAction::EnterLockScreen, evaluatePowerAction(p, s, 1000));
}

void test_enters_deep_sleep_only_from_lock_screen() {
  PowerPolicy p;
  p.lockTimeoutMs = 1000;
  p.deepSleepTimeoutMs = 2000;
  PowerState s;
  s.lastInteractionMs = 0;
  s.lockScreenActive = true;

  TEST_ASSERT_EQUAL(PowerAction::None, evaluatePowerAction(p, s, 1999));
  TEST_ASSERT_EQUAL(PowerAction::EnterDeepSleep, evaluatePowerAction(p, s, 2000));

  s.lockScreenActive = false;
  TEST_ASSERT_EQUAL(PowerAction::EnterLockScreen, evaluatePowerAction(p, s, 2000));
}

void test_wifi_disable_policy() {
  TEST_ASSERT_TRUE(shouldDisableWifiForLowPower(true, false));
  TEST_ASSERT_FALSE(shouldDisableWifiForLowPower(true, true));
  TEST_ASSERT_FALSE(shouldDisableWifiForLowPower(false, false));
}

void test_parse_power_config_valid_values() {
  PowerConfig cfg = parsePowerConfig("{\"lockTimeoutMs\":45000,\"deepSleepTimeoutMs\":180000,\"allowDeepSleep\":false,\"deepSleepDurationSec\":90}");
  TEST_ASSERT_TRUE(cfg.valid);
  TEST_ASSERT_EQUAL_UINT32(45000, cfg.policy.lockTimeoutMs);
  TEST_ASSERT_EQUAL_UINT32(180000, cfg.policy.deepSleepTimeoutMs);
  TEST_ASSERT_FALSE(cfg.policy.allowDeepSleep);
  TEST_ASSERT_EQUAL_UINT32(90, cfg.policy.deepSleepDurationSec);
}

void test_parse_power_config_applies_clamps_and_defaults() {
  PowerConfig cfg = parsePowerConfig("{\"lockTimeoutMs\":10,\"deepSleepTimeoutMs\":500,\"deepSleepDurationSec\":2}");
  TEST_ASSERT_TRUE(cfg.valid);
  TEST_ASSERT_EQUAL_UINT32(1000, cfg.policy.lockTimeoutMs);
  TEST_ASSERT_EQUAL_UINT32(1000, cfg.policy.deepSleepTimeoutMs);
  TEST_ASSERT_TRUE(cfg.policy.allowDeepSleep);
  TEST_ASSERT_EQUAL_UINT32(10, cfg.policy.deepSleepDurationSec);
}

void test_parse_power_config_invalid_payload() {
  PowerConfig cfg = parsePowerConfig("{}");
  TEST_ASSERT_FALSE(cfg.valid);
  TEST_ASSERT_EQUAL_UINT32(30000, cfg.policy.lockTimeoutMs);
  TEST_ASSERT_EQUAL_UINT32(120000, cfg.policy.deepSleepTimeoutMs);
  TEST_ASSERT_TRUE(cfg.policy.allowDeepSleep);
  TEST_ASSERT_EQUAL_UINT32(60, cfg.policy.deepSleepDurationSec);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_enters_lock_screen_after_idle_timeout);
  RUN_TEST(test_enters_deep_sleep_only_from_lock_screen);
  RUN_TEST(test_wifi_disable_policy);
  RUN_TEST(test_parse_power_config_valid_values);
  RUN_TEST(test_parse_power_config_applies_clamps_and_defaults);
  RUN_TEST(test_parse_power_config_invalid_payload);
  return UNITY_END();
}
