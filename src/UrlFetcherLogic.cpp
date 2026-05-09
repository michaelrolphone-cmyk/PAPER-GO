#include "UrlFetcherLogic.h"
#include <ArduinoJson.h>

UrlFetcherConfig parseUrlFetcherConfig(const String& json) {
  UrlFetcherConfig cfg;
  JsonDocument doc;
  if (deserializeJson(doc, json) != DeserializationError::Ok) return cfg;
  if (!doc["url"].is<const char*>()) return cfg;
  cfg.url = String(doc["url"].as<const char*>());
  if (!cfg.url.startsWith("http://") && !cfg.url.startsWith("https://")) return cfg;
  cfg.timeoutMs = doc["timeoutMs"].is<uint16_t>() ? doc["timeoutMs"].as<uint16_t>() : 5000;
  cfg.valid = true;
  return cfg;
}

String cachePathForUrl(const String& url) {
  uint32_t hash = 5381;
  for (size_t i = 0; i < url.length(); ++i) hash = ((hash << 5) + hash) + (uint8_t)url[i];
  return "/cache/http/url-" + String(hash, HEX) + ".txt";
}
