#include "WifiConfigLogic.h"

static String jsonValue(const String& json, const char* key) {
  String k = String("\"") + key + "\"";
  int i = json.indexOf(k);
  if (i < 0) return "";
  int c = json.indexOf(':', i + k.length());
  if (c < 0) return "";
  int q1 = json.indexOf('"', c + 1);
  if (q1 < 0) return "";
  int q2 = json.indexOf('"', q1 + 1);
  if (q2 < 0) return "";
  return json.substring(q1 + 1, q2);
}

WifiConfig parseWifiConfig(const String& json) {
  WifiConfig cfg;
  cfg.ssid = jsonValue(json, "ssid");
  cfg.password = jsonValue(json, "password");
  cfg.valid = cfg.ssid.length() > 0;
  return cfg;
}

String buildWifiConfigJson(const String& ssid, const String& password) {
  return String("{\"ssid\":\"") + ssid + "\",\"password\":\"" + password + "\"}";
}

String maskedPassword(const String& password) {
  if (password.length() == 0) return "";
  String out;
  for (size_t i = 0; i < password.length(); ++i) out += '*';
  return out;
}
