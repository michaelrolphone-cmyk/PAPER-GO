#pragma once
#include <Arduino.h>
#include <vector>

struct RadioControlConfig {
  bool wifiEnabled = true;
  bool bleEnabled = true;
  bool loraEnabled = true;
  uint32_t bleScanMs = 1500;
  uint32_t loraScanMs = 500;
  bool valid = false;
};

String buildRadioScanListJson(const std::vector<String>& fileNames);
String buildRadioControlJson(const RadioControlConfig& cfg);
RadioControlConfig parseRadioControlConfig(const String& json);

String escapeJsonString(const String& input);
