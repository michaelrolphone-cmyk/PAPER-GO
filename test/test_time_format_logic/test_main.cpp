#include <unity.h>
#include "TimeFormatLogic.h"

void test_format_known_epoch() {
  TEST_ASSERT_EQUAL_STRING("2023-11-14", formatUtcDate(1700000000ULL).c_str());
  TEST_ASSERT_EQUAL_STRING("22:13:20 UTC", formatUtcTime(1700000000ULL).c_str());
}

void test_unknown_epoch() {
  TEST_ASSERT_EQUAL_STRING("Date: unknown", formatUtcDate(0).c_str());
  TEST_ASSERT_EQUAL_STRING("Time: unknown", formatUtcTime(0).c_str());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_format_known_epoch);
  RUN_TEST(test_unknown_epoch);
  return UNITY_END();
}
