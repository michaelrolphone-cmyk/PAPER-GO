#include "BootLogLogic.h"

String boolLabel(bool value) {
  return value ? "ok" : "fail";
}

String bootStepLog(const String& step, bool ok, const String& detail) {
  String line = "[BOOT] " + step + " => " + boolLabel(ok);
  if (detail.length()) line += " (" + detail + ")";
  return line;
}
