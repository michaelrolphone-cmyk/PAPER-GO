#pragma once
#include <Arduino.h>
#include <vector>

std::vector<String> defaultAppOrder();
String appDisplayNameById(const String& appId);
std::vector<String> parseOrderedAppIds(const String& jsonText);
bool appSupportsOfflineMode(const String& appId);
