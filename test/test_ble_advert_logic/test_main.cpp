#include <unity.h>
#include "BleAdvertLogic.h"

void test_ble_summary_contains_all_fields() {
  String s = buildBleAdvertSummary("Beacon", -70, 3, 12);
  TEST_ASSERT_TRUE(s.indexOf("name=Beacon") >= 0);
  TEST_ASSERT_TRUE(s.indexOf("rssi=-70") >= 0);
  TEST_ASSERT_TRUE(s.indexOf("svc=3") >= 0);
  TEST_ASSERT_TRUE(s.indexOf("mfgBytes=12") >= 0);
}

void test_ble_summary_handles_empty_name() {
  String s = buildBleAdvertSummary("", -40, 0, 0);
  TEST_ASSERT_TRUE(s.indexOf("name=") < 0);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_ble_summary_contains_all_fields);
  RUN_TEST(test_ble_summary_handles_empty_name);
  return UNITY_END();
}
