#include "WeatherLogic.h"
#include <ArduinoJson.h>

WeatherCacheInfo parseWeatherCacheJson(const String& json) {
  WeatherCacheInfo out;
  JsonDocument doc;
  if (deserializeJson(doc, json) != DeserializationError::Ok) return out;
  if (!doc["fetchedEpoch"].is<uint64_t>()) return out;
  out.fetchedEpoch = doc["fetchedEpoch"].as<uint64_t>();
  out.summary = doc["summary"].is<const char*>() ? String(doc["summary"].as<const char*>()) : String("(no summary)");
  out.valid = true;
  return out;
}

bool isWeatherCacheStale(uint64_t nowEpoch, uint64_t fetchedEpoch, uint32_t staleAfterSec) {
  if (fetchedEpoch == 0 || nowEpoch < fetchedEpoch) return true;
  return (nowEpoch - fetchedEpoch) > staleAfterSec;
}
