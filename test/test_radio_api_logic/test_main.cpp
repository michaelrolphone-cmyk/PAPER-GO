#include <unity.h>
#include "RadioApiLogic.h"

void test_radio_scan_list_json() {
  std::vector<String> names = {"scan-1.log", "scan-2.log"};
  String j = buildRadioScanListJson(names);
  TEST_ASSERT_EQUAL_STRING("{\"files\":[\"scan-1.log\",\"scan-2.log\"]}", j.c_str());
}
void test_radio_control_json_roundtrip() {
  RadioControlConfig cfg;
  cfg.wifiEnabled = false;
  cfg.bleEnabled = true;
  cfg.loraEnabled = false;
  cfg.bleScanMs = 1800;
  cfg.loraScanMs = 650;
  String j = buildRadioControlJson(cfg);
  TEST_ASSERT_EQUAL_STRING("{\"wifiEnabled\":false,\"bleEnabled\":true,\"loraEnabled\":false,\"bleScanMs\":1800,\"loraScanMs\":650}", j.c_str());

  RadioControlConfig parsed = parseRadioControlConfig(j);
  TEST_ASSERT_TRUE(parsed.valid);
  TEST_ASSERT_FALSE(parsed.wifiEnabled);
  TEST_ASSERT_TRUE(parsed.bleEnabled);
  TEST_ASSERT_FALSE(parsed.loraEnabled);
  TEST_ASSERT_EQUAL_UINT32(1800, parsed.bleScanMs);
  TEST_ASSERT_EQUAL_UINT32(650, parsed.loraScanMs);
}

void test_radio_control_bounds() {
  RadioControlConfig parsed = parseRadioControlConfig("{\"bleScanMs\":10,\"loraScanMs\":25000}");
  TEST_ASSERT_TRUE(parsed.valid);
  TEST_ASSERT_EQUAL_UINT32(250, parsed.bleScanMs);
  TEST_ASSERT_EQUAL_UINT32(10000, parsed.loraScanMs);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_radio_scan_list_json);
  RUN_TEST(test_radio_control_json_roundtrip);
  RUN_TEST(test_radio_control_bounds);
  return UNITY_END();
}
