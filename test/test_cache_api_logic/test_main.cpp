#include <unity.h>
#include "CacheApiLogic.h"

void test_cache_stats_json() {
  String j = buildCacheStatsJson(7, 2);
  TEST_ASSERT_EQUAL_STRING("{\"mapCacheHits\":7,\"mapCacheMisses\":2}", j.c_str());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_cache_stats_json);
  return UNITY_END();
}
