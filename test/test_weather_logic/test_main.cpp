#include <unity.h>
#include "WeatherLogic.h"

void test_parse_weather_cache_json() {
  String json = "{\"fetchedEpoch\":1700000000,\"summary\":\"Clear skies\"}";
  WeatherCacheInfo info = parseWeatherCacheJson(json);
  TEST_ASSERT_TRUE(info.valid);
  TEST_ASSERT_EQUAL_UINT64(1700000000ULL, info.fetchedEpoch);
  TEST_ASSERT_EQUAL_STRING("Clear skies", info.summary.c_str());
}

void test_stale_detection() {
  TEST_ASSERT_FALSE(isWeatherCacheStale(1700000900ULL, 1700000000ULL, 1800));
  TEST_ASSERT_TRUE(isWeatherCacheStale(1700005000ULL, 1700000000ULL, 1800));
  TEST_ASSERT_TRUE(isWeatherCacheStale(0, 1700000000ULL, 1800));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_parse_weather_cache_json);
  RUN_TEST(test_stale_detection);
  return UNITY_END();
}
