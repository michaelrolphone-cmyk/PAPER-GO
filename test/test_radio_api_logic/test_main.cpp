#include <unity.h>
#include "RadioApiLogic.h"

void test_radio_scan_list_json() {
  std::vector<String> names = {"scan-1.log", "scan-2.log"};
  String j = buildRadioScanListJson(names);
  TEST_ASSERT_EQUAL_STRING("{\"files\":[\"scan-1.log\",\"scan-2.log\"]}", j.c_str());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_radio_scan_list_json);
  return UNITY_END();
}
