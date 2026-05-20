#include "RadioApiLogic.h"

static bool parseUnsignedJsonValue(const String& json, const char* key, uint32_t* out) {
  String k = String("\"") + key + "\"";
  int i = json.indexOf(k);
  if (i < 0) return false;
  int c = json.indexOf(':', i + k.length());
  if (c < 0) return false;
  int start = c + 1;
  while (start < (int)json.length() && (json[start] == ' ' || json[start] == '\t' || json[start] == '\n' || json[start] == '\r')) start++;
  int end = start;
  while (end < (int)json.length() && json[end] >= '0' && json[end] <= '9') end++;
  if (end == start) return false;
  *out = (uint32_t) json.substring(start, end).toInt();
  return true;
}

static bool parseBoolJsonValue(const String& json, const char* key, bool* out) {
  String k = String("\"") + key + "\"";
  int i = json.indexOf(k);
  if (i < 0) return false;
  int c = json.indexOf(':', i + k.length());
  if (c < 0) return false;
  int start = c + 1;
  while (start < (int)json.length() && (json[start] == ' ' || json[start] == '\t' || json[start] == '\n' || json[start] == '\r')) start++;
  if (json.startsWith("true", start)) { *out = true; return true; }
  if (json.startsWith("false", start)) { *out = false; return true; }
  return false;
}


String escapeJsonString(const String& input) {
  String out;
  out.reserve(input.length() + 8);
  for (size_t i = 0; i < input.length(); ++i) {
    char c = input.charAt(i);
    switch (c) {
      case '\\': out += "\\\\"; break;
      case '"': out += "\\\""; break;
      case '\b': out += "\\b"; break;
      case '\f': out += "\\f"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default: out += c; break;
    }
  }
  return out;
}

String buildRadioScanListJson(const std::vector<String>& fileNames) {
  String out = "{\"files\":[";
  for (size_t i=0;i<fileNames.size();++i) {
    if (i) out += ",";
    out += "\"" + escapeJsonString(fileNames[i]) + "\"";
  }
  out += "]}";
  return out;
}

String buildRadioControlJson(const RadioControlConfig& cfg) {
  return String("{\"wifiEnabled\":") + (cfg.wifiEnabled ? "true" : "false") +
    ",\"bleEnabled\":" + (cfg.bleEnabled ? "true" : "false") +
    ",\"loraEnabled\":" + (cfg.loraEnabled ? "true" : "false") +
    ",\"bleScanMs\":" + String(cfg.bleScanMs) +
    ",\"loraScanMs\":" + String(cfg.loraScanMs) + "}";
}

RadioControlConfig parseRadioControlConfig(const String& json) {
  RadioControlConfig cfg;
  bool hasWifi = parseBoolJsonValue(json, "wifiEnabled", &cfg.wifiEnabled);
  bool hasBle = parseBoolJsonValue(json, "bleEnabled", &cfg.bleEnabled);
  bool hasLora = parseBoolJsonValue(json, "loraEnabled", &cfg.loraEnabled);
  bool hasBleMs = parseUnsignedJsonValue(json, "bleScanMs", &cfg.bleScanMs);
  bool hasLoraMs = parseUnsignedJsonValue(json, "loraScanMs", &cfg.loraScanMs);

  if (cfg.bleScanMs < 250) cfg.bleScanMs = 250;
  if (cfg.bleScanMs > 10000) cfg.bleScanMs = 10000;
  if (cfg.loraScanMs < 100) cfg.loraScanMs = 100;
  if (cfg.loraScanMs > 10000) cfg.loraScanMs = 10000;

  cfg.valid = hasWifi || hasBle || hasLora || hasBleMs || hasLoraMs;
  return cfg;
}
