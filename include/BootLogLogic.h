#pragma once

#include <Arduino.h>

String bootStepLog(const String& step, bool ok, const String& detail = "");
String boolLabel(bool value);
String bootStepSkippedLog(const String& step, const String& detail = "");
String bootTimingLog(const String& step, uint32_t elapsedMs);
String bootSummaryLog(uint32_t totalMs, uint8_t okCount, uint8_t failCount, uint8_t skippedCount = 0);
