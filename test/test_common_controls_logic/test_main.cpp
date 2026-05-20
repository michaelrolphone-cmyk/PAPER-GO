#include <Arduino.h>
#include <unity.h>
#include "CommonControlsLogic.h"

void test_bool_label() {
  TEST_ASSERT_EQUAL_STRING("true", commonControlBoolLabel(true).c_str());
  TEST_ASSERT_EQUAL_STRING("false", commonControlBoolLabel(false).c_str());
}

void test_labeled_value_format() {
  TEST_ASSERT_EQUAL_STRING("Lock timeout (ms): 30000", commonControlLabeledValue("Lock timeout (ms)", "30000").c_str());
}

void test_selected_color_uses_accent() {
  CommonControlRowStyle s;
  s.textGray = 1;
  s.accentGray = 7;
  TEST_ASSERT_EQUAL_UINT8(7, commonControlTextColor(true, s));
  TEST_ASSERT_EQUAL_UINT8(1, commonControlTextColor(false, s));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_bool_label);
  RUN_TEST(test_labeled_value_format);
  RUN_TEST(test_selected_color_uses_accent);
  UNITY_END();
}

void loop() {}
