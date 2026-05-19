#include <unity.h>
#include "WeatherFetchLogic.h"

void test_parse_weather_fetch_config() {
  String raw = "{\"urlTemplate\":\"https://api.example.com/weather?lat={lat}&lon={lon}\",\"timeoutMs\":7000}";
  WeatherFetchConfig cfg = parseWeatherFetchConfig(raw);
  TEST_ASSERT_TRUE(cfg.valid);
  TEST_ASSERT_EQUAL_UINT32(7000, cfg.timeoutMs);
}

void test_build_weather_url() {
  WeatherFetchConfig cfg;
  cfg.valid = true;
  cfg.urlTemplate = "https://host/w?lat={lat}&lon={lon}";
  GpsFix fix{};
  fix.valid = true;
  fix.lat = 10.5;
  fix.lon = -20.25;
  String url = buildWeatherUrl(cfg, fix);
  TEST_ASSERT_NOT_EQUAL(-1, url.indexOf("lat=10.500000"));
  TEST_ASSERT_NOT_EQUAL(-1, url.indexOf("lon=-20.250000"));
}

void test_extract_summary_fallback() {
  String response = "{\"current_weather\":{\"temperature\":22.3}}";
  TEST_ASSERT_EQUAL_STRING("Temp 22.3C", extractWeatherSummary(response).c_str());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_parse_weather_fetch_config);
  RUN_TEST(test_build_weather_url);
  RUN_TEST(test_extract_summary_fallback);
  return UNITY_END();
}
