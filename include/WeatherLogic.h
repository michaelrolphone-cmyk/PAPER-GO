#pragma once
#include <Arduino.h>

struct WeatherCacheInfo {
  bool valid = false;
  uint64_t fetchedEpoch = 0;
  String summary;
};

WeatherCacheInfo parseWeatherCacheJson(const String& json);
bool isWeatherCacheStale(uint64_t nowEpoch, uint64_t fetchedEpoch, uint32_t staleAfterSec);
