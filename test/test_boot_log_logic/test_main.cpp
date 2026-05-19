#include <Arduino.h>
#include <unity.h>
#include "BootLogLogic.h"

void test_bool_label() {
  TEST_ASSERT_EQUAL_STRING("ok", boolLabel(true).c_str());
  TEST_ASSERT_EQUAL_STRING("fail", boolLabel(false).c_str());
}

void test_boot_step_log_with_detail() {
  String line = bootStepLog("cache.begin", true, "sd mounted");
  TEST_ASSERT_TRUE(line.indexOf("[BOOT] cache.begin => ok") >= 0);
  TEST_ASSERT_TRUE(line.indexOf("sd mounted") >= 0);
}

void test_boot_step_log_without_detail() {
  String line = bootStepLog("radio.begin", false);
  TEST_ASSERT_EQUAL_STRING("[BOOT] radio.begin => fail", line.c_str());
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_bool_label);
  RUN_TEST(test_boot_step_log_with_detail);
  RUN_TEST(test_boot_step_log_without_detail);
  UNITY_END();
}

void loop() {}
