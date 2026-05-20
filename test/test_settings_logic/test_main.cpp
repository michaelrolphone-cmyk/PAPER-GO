#include <Arduino.h>
#include <unity.h>
#include "SettingsLogic.h"

void test_row_from_tap_y_bounds() {
  TEST_ASSERT_EQUAL(-1, settingsRowFromTapY(99, 100, 20, 3));
  TEST_ASSERT_EQUAL(0, settingsRowFromTapY(100, 100, 20, 3));
  TEST_ASSERT_EQUAL(2, settingsRowFromTapY(159, 100, 20, 3));
  TEST_ASSERT_EQUAL(-1, settingsRowFromTapY(160, 100, 20, 3));
}

void test_cycle_lock_timeout_wraps() {
  TEST_ASSERT_EQUAL_UINT32(30000, cycleLockTimeoutMs(10000, true));
  TEST_ASSERT_EQUAL_UINT32(10000, cycleLockTimeoutMs(300000, true));
  TEST_ASSERT_EQUAL_UINT32(300000, cycleLockTimeoutMs(10000, false));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_row_from_tap_y_bounds);
  RUN_TEST(test_cycle_lock_timeout_wraps);
  UNITY_END();
}

void loop() {}
