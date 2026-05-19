#include "AppManagementApiLogic.h"
#include <ArduinoJson.h>

bool isValidAppId(const String& appId) {
  if (!appId.length() || appId.length() > 32) return false;
  for (size_t i = 0; i < appId.length(); ++i) {
    const char c = appId[i];
    const bool alphaNum = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
    if (!alphaNum && c != '-' && c != '_') return false;
  }
  return true;
}

bool parseAppManagementRequest(const String& body, AppManagementRequest& out) {
  JsonDocument doc;
  if (deserializeJson(doc, body) != DeserializationError::Ok) return false;
  if (!doc["id"].is<const char*>()) return false;
  out.id = doc["id"].as<const char*>();
  out.sourceUrl = doc["sourceUrl"].is<const char*>() ? String(doc["sourceUrl"].as<const char*>()) : String("");
  out.version = doc["version"].is<const char*>() ? String(doc["version"].as<const char*>()) : String("");
  return isValidAppId(out.id);
}

String buildAppManagementResultJson(const char* action, const String& appId, bool ok, const String& detail) {
  String body = "{";
  body += "\"action\":\"" + String(action) + "\"";
  body += ",\"id\":\"" + appId + "\"";
  body += ",\"ok\":" + String(ok ? "true" : "false");
  body += ",\"detail\":\"" + detail + "\"";
  body += "}";
  return body;
}
