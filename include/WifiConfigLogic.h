#pragma once

#include <Arduino.h>

struct WifiConfig {
  String ssid;
  String password;
  bool valid = false;
};

WifiConfig parseWifiConfig(const String& json);
String buildWifiConfigJson(const String& ssid, const String& password);
String maskedPassword(const String& password);
