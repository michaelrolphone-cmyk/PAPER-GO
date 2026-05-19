#include "AppCatalog.h"
#include <ArduinoJson.h>

std::vector<String> defaultAppOrder() {
  return {"gpsmap","radio","mesh","url","markdown","files","weather","web","games","settings"};
}

String appDisplayNameById(const String& appId) {
  if (appId == "gpsmap") return "GPS Map";
  if (appId == "radio") return "Radio";
  if (appId == "mesh") return "Mesh";
  if (appId == "url") return "URL";
  if (appId == "markdown") return "Markdown";
  if (appId == "files") return "Files";
  if (appId == "weather") return "Weather";
  if (appId == "web") return "Web";
  if (appId == "games") return "Games";
  if (appId == "settings") return "Settings";
  return "Unknown";
}

std::vector<String> parseOrderedAppIds(const String& jsonText) {
  std::vector<String> ordered;
  if (!jsonText.length()) return ordered;

  JsonDocument doc;
  if (deserializeJson(doc, jsonText) != DeserializationError::Ok) return ordered;
  if (!doc["order"].is<JsonArray>()) return ordered;

  auto allowed = defaultAppOrder();
  JsonArray arr = doc["order"].as<JsonArray>();
  for (JsonVariant v : arr) {
    if (!v.is<const char*>()) continue;
    String id = v.as<const char*>();
    bool valid = false;
    for (const auto& x : allowed) {
      if (x == id) {
        valid = true;
        break;
      }
    }
    if (!valid) continue;

    bool exists = false;
    for (const auto& x : ordered) {
      if (x == id) {
        exists = true;
        break;
      }
    }
    if (!exists) ordered.push_back(id);
  }

  for (const auto& id : allowed) {
    bool exists = false;
    for (const auto& x : ordered) {
      if (x == id) {
        exists = true;
        break;
      }
    }
    if (!exists) ordered.push_back(id);
  }

  return ordered;
}

String serializeOrderedAppIds(const std::vector<String>& orderedIds) {
  JsonDocument doc;
  JsonArray arr = doc["order"].to<JsonArray>();
  for (const auto& id : orderedIds) arr.add(id);
  String out;
  serializeJson(doc, out);
  return out;
}

bool appSupportsOfflineMode(const String& appId) {
  return !(appId == "url" || appId == "weather");
}
