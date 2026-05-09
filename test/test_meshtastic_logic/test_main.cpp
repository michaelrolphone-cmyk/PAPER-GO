#include <unity.h>
#include "MeshtasticLogic.h"

void test_meshtastic_storage_status_format() {
  String s = formatMeshtasticStorageStatus(12, 3);
  TEST_ASSERT_EQUAL_STRING("messages=12, nodes=3", s.c_str());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_meshtastic_storage_status_format);
  return UNITY_END();
}
