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


void test_boot_step_skipped_log() {
  String line = bootStepSkippedLog("cache.begin", "sd not mounted");
  TEST_ASSERT_EQUAL_STRING("[BOOT] cache.begin => skipped (sd not mounted)", line.c_str());
}

void test_boot_timing_log() {
  String line = bootTimingLog("net.begin", 27);
  TEST_ASSERT_EQUAL_STRING("[BOOT] net.begin took 27ms", line.c_str());
}

void test_boot_summary_log() {
  String line = bootSummaryLog(180, 7, 2, 1);
  TEST_ASSERT_EQUAL_STRING("[BOOT] summary => total=180ms, ok=7, fail=2, skipped=1", line.c_str());
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_bool_label);
  RUN_TEST(test_boot_step_log_with_detail);
  RUN_TEST(test_boot_step_log_without_detail);
  RUN_TEST(test_boot_step_skipped_log);
  RUN_TEST(test_boot_timing_log);
  RUN_TEST(test_boot_summary_log);
  UNITY_END();
}

void loop() {}
