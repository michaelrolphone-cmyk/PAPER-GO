#pragma once

#include <Arduino.h>

bool shouldAttemptSavedWifiConnect(const String& rawWifiConfigJson, bool hasCache);
