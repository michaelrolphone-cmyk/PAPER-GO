#include "GpsConfigLogic.h"

bool parseGpsTrackLoggingEnabled(const String& deviceConfigJson, bool defaultValue) {
  int key = deviceConfigJson.indexOf("\"gpsTrackLogging\"");
  if (key < 0) return defaultValue;
  int colon = deviceConfigJson.indexOf(':', key);
  if (colon < 0) return defaultValue;
  String tail = deviceConfigJson.substring(colon + 1);
  tail.trim();
  if (tail.startsWith("true")) return true;
  if (tail.startsWith("false")) return false;
  return defaultValue;
}


String buildGpsTrackLoggingConfigJson(bool enabled) {
  return String("{\"gpsTrackLogging\":") + (enabled ? "true" : "false") + "}";
}

bool updateGpsTrackLoggingEnabled(const String& deviceConfigJson, bool enabled, String& updatedJson) {
  int key = deviceConfigJson.indexOf("\"gpsTrackLogging\"");
  if (key < 0) {
    updatedJson = buildGpsTrackLoggingConfigJson(enabled);
    return true;
  }
  int colon = deviceConfigJson.indexOf(':', key);
  if (colon < 0) return false;
  int end = deviceConfigJson.indexOf(',', colon + 1);
  if (end < 0) end = deviceConfigJson.indexOf('}', colon + 1);
  if (end < 0) return false;
  String out = deviceConfigJson;
  out.remove(colon + 1, end - (colon + 1));
  out = out.substring(0, colon + 1) + (enabled ? "true" : "false") + out.substring(colon + 1);
  updatedJson = out;
  return true;
}
