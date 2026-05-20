#include <unity.h>
#include "TouchProbeLogic.h"

void test_prefers_0x5d_when_both_present() {
  TEST_ASSERT_EQUAL_HEX8(0x5D, selectGt911Address(true, true));
}

void test_falls_back_to_0x14() {
  TEST_ASSERT_EQUAL_HEX8(0x14, selectGt911Address(false, true));
}

void test_returns_zero_when_not_found() {
  TEST_ASSERT_EQUAL_HEX8(0x00, selectGt911Address(false, false));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_prefers_0x5d_when_both_present);
  RUN_TEST(test_falls_back_to_0x14);
  RUN_TEST(test_returns_zero_when_not_found);
  return UNITY_END();
}
