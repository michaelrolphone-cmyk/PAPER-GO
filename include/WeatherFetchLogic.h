#pragma once
#include <Arduino.h>
#include "Services.h"

struct WeatherFetchConfig {
  bool valid = false;
  String urlTemplate;
  uint32_t timeoutMs = 5000;
};

WeatherFetchConfig parseWeatherFetchConfig(const String& json);
String buildWeatherUrl(const WeatherFetchConfig& cfg, const GpsFix& fix);
String buildWeatherCacheJson(uint64_t fetchedEpoch, const String& summary);
String extractWeatherSummary(const String& responseJson);
