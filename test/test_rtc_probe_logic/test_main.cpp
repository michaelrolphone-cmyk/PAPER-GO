#include <unity.h>
#include "RtcProbeLogic.h"

void test_probe_success_on_zero_status() {
  TEST_ASSERT_TRUE(isRtcI2cProbeSuccess(0));
}

void test_probe_failure_on_nonzero_status() {
  TEST_ASSERT_FALSE(isRtcI2cProbeSuccess(1));
  TEST_ASSERT_FALSE(isRtcI2cProbeSuccess(4));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_probe_success_on_zero_status);
  RUN_TEST(test_probe_failure_on_nonzero_status);
  return UNITY_END();
}
