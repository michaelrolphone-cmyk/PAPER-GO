#pragma once

#include <Arduino.h>

bool parseGpsTrackLoggingEnabled(const String& deviceConfigJson, bool defaultValue = false);
String buildGpsTrackLoggingConfigJson(bool enabled);

bool updateGpsTrackLoggingEnabled(const String& deviceConfigJson, bool enabled, String& updatedJson);
