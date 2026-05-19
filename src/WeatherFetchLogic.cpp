#include "WeatherFetchLogic.h"
#include <ArduinoJson.h>

WeatherFetchConfig parseWeatherFetchConfig(const String& json) {
  WeatherFetchConfig out;
  JsonDocument doc;
  if (deserializeJson(doc, json) != DeserializationError::Ok) return out;
  if (!doc["urlTemplate"].is<const char*>()) return out;
  out.urlTemplate = String(doc["urlTemplate"].as<const char*>());
  if (doc["timeoutMs"].is<uint32_t>()) out.timeoutMs = doc["timeoutMs"].as<uint32_t>();
  out.valid = out.urlTemplate.indexOf("{lat}") >= 0 && out.urlTemplate.indexOf("{lon}") >= 0;
  return out;
}

String buildWeatherUrl(const WeatherFetchConfig& cfg, const GpsFix& fix) {
  if (!cfg.valid || !fix.valid) return "";
  String u = cfg.urlTemplate;
  u.replace("{lat}", String(fix.lat, 6));
  u.replace("{lon}", String(fix.lon, 6));
  return u;
}

String buildWeatherCacheJson(uint64_t fetchedEpoch, const String& summary) {
  JsonDocument doc;
  doc["fetchedEpoch"] = fetchedEpoch;
  doc["summary"] = summary;
  String out;
  serializeJson(doc, out);
  return out;
}

String extractWeatherSummary(const String& responseJson) {
  JsonDocument doc;
  if (deserializeJson(doc, responseJson) != DeserializationError::Ok) return "Invalid response";
  if (doc["summary"].is<const char*>()) return String(doc["summary"].as<const char*>());
  if (doc["current_weather"]["temperature"].is<float>()) {
    return String("Temp ") + String(doc["current_weather"]["temperature"].as<float>(), 1) + "C";
  }
  return "No summary field";
}
