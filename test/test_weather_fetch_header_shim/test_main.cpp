#include <unity.h>
#include "WeatherFetchLogic.h"

void test_weather_fetch_header_shim_exposes_api() {
  WeatherFetchConfig cfg{};
  cfg.valid = true;
  cfg.urlTemplate = "https://example.test?lat={lat}&lon={lon}";
  cfg.timeoutMs = 5000;
  TEST_ASSERT_TRUE(cfg.valid);
  TEST_ASSERT_TRUE(cfg.urlTemplate.length() > 0);
  TEST_ASSERT_EQUAL_UINT32(5000, cfg.timeoutMs);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_weather_fetch_header_shim_exposes_api);
  return UNITY_END();
}
