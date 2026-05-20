#include "BootLogLogic.h"

String boolLabel(bool value) {
  return value ? "ok" : "fail";
}

String bootStepLog(const String& step, bool ok, const String& detail) {
  String line = "[BOOT] " + step + " => " + boolLabel(ok);
  if (detail.length()) line += " (" + detail + ")";
  return line;
}


String bootStepSkippedLog(const String& step, const String& detail) {
  String line = "[BOOT] " + step + " => skipped";
  if (detail.length()) line += " (" + detail + ")";
  return line;
}

String bootTimingLog(const String& step, uint32_t elapsedMs) {
  return "[BOOT] " + step + " took " + String(elapsedMs) + "ms";
}

String bootSummaryLog(uint32_t totalMs, uint8_t okCount, uint8_t failCount, uint8_t skippedCount) {
  return "[BOOT] summary => total=" + String(totalMs) + "ms, ok=" + String(okCount) + ", fail=" + String(failCount) + ", skipped=" + String(skippedCount);
}
