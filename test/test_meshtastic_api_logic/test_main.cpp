#include <unity.h>
#include "MeshtasticApiLogic.h"

void test_meshtastic_stats_json() {
  String j = buildMeshtasticStatsJson(4, 2);
  TEST_ASSERT_EQUAL_STRING("{\"messageFiles\":4,\"nodeFiles\":2}", j.c_str());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_meshtastic_stats_json);
  return UNITY_END();
}
