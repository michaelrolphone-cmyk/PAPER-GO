#include <Arduino.h>
#include <unity.h>

#include "SettingsLogic.h"

void test_build_power_config_json_contains_all_fields() {
  PowerPolicy p;
  p.lockTimeoutMs = 60000;
  p.deepSleepTimeoutMs = 120000;
  p.allowDeepSleep = false;
  p.deepSleepDurationSec = 300;
  String json = buildPowerConfigJson(p);
  TEST_ASSERT_NOT_EQUAL(-1, json.indexOf("\"lockTimeoutMs\":60000"));
  TEST_ASSERT_NOT_EQUAL(-1, json.indexOf("\"deepSleepTimeoutMs\":120000"));
  TEST_ASSERT_NOT_EQUAL(-1, json.indexOf("\"allowDeepSleep\":false"));
  TEST_ASSERT_NOT_EQUAL(-1, json.indexOf("\"deepSleepDurationSec\":300"));
}

void test_settings_row_from_tap_y_bounds() {
  TEST_ASSERT_EQUAL(-1, settingsRowFromTapY(155, 156, 24, 6));
  TEST_ASSERT_EQUAL(0, settingsRowFromTapY(156, 156, 24, 6));
  TEST_ASSERT_EQUAL(5, settingsRowFromTapY(156 + (5 * 24) + 2, 156, 24, 6));
  TEST_ASSERT_EQUAL(-1, settingsRowFromTapY(156 + (6 * 24), 156, 24, 6));
}

void test_cycle_values_progress_and_wrap() {
  TEST_ASSERT_EQUAL_UINT32(30000, cycleLockTimeoutMs(10000, true));
  TEST_ASSERT_EQUAL_UINT32(10000, cycleLockTimeoutMs(30000, false));
  TEST_ASSERT_EQUAL_UINT32(10000, cycleLockTimeoutMs(300000, true));

  TEST_ASSERT_EQUAL_UINT32(60000, cycleDeepSleepTimeoutMs(30000, true));
  TEST_ASSERT_EQUAL_UINT32(30000, cycleDeepSleepTimeoutMs(60000, false));

  TEST_ASSERT_EQUAL_UINT32(30, cycleDeepSleepDurationSec(10, true));
  TEST_ASSERT_EQUAL_UINT32(10, cycleDeepSleepDurationSec(30, false));
}


void test_tap_behavior_requires_second_tap_on_same_row_for_edit() {
  TEST_ASSERT_FALSE(settingsTapShouldEditSelectedRow(0, 1));
  TEST_ASSERT_TRUE(settingsTapShouldEditSelectedRow(3, 3));
  TEST_ASSERT_FALSE(settingsTapShouldEditSelectedRow(-1, 3));
}

void setup() {
  delay(1000);
  UNITY_BEGIN();
  RUN_TEST(test_build_power_config_json_contains_all_fields);
  RUN_TEST(test_settings_row_from_tap_y_bounds);
  RUN_TEST(test_cycle_values_progress_and_wrap);
  RUN_TEST(test_tap_behavior_requires_second_tap_on_same_row_for_edit);
  UNITY_END();
}

void loop() {}
