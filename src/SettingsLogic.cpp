#include "SettingsLogic.h"

static uint32_t cycleInTable(uint32_t current, const uint32_t* values, size_t count, bool increment) {
  size_t idx = 0;
  for (size_t i = 0; i < count; ++i) {
    if (values[i] == current) { idx = i; break; }
    if (values[i] > current) { idx = i; break; }
    idx = count - 1;
  }
  if (increment) return values[(idx + 1) % count];
  return values[(idx + count - 1) % count];
}

String buildPowerConfigJson(const PowerPolicy& policy) {
  String body = "{";
  body += "\"lockTimeoutMs\":" + String(policy.lockTimeoutMs) + ",";
  body += "\"deepSleepTimeoutMs\":" + String(policy.deepSleepTimeoutMs) + ",";
  body += "\"allowDeepSleep\":" + String(policy.allowDeepSleep ? "true" : "false") + ",";
  body += "\"deepSleepDurationSec\":" + String(policy.deepSleepDurationSec);
  body += "}";
  return body;
}

int settingsRowFromTapY(int tapY, int startY, int rowHeight, int rowCount) {
  if (tapY < startY) return -1;
  int row = (tapY - startY) / rowHeight;
  if (row < 0 || row >= rowCount) return -1;
  return row;
}

uint32_t cycleLockTimeoutMs(uint32_t current, bool increment) {
  static const uint32_t values[] = {10000, 30000, 60000, 120000, 300000};
  return cycleInTable(current, values, sizeof(values) / sizeof(values[0]), increment);
}

uint32_t cycleDeepSleepTimeoutMs(uint32_t current, bool increment) {
  static const uint32_t values[] = {30000, 60000, 120000, 300000, 600000};
  return cycleInTable(current, values, sizeof(values) / sizeof(values[0]), increment);
}

uint32_t cycleDeepSleepDurationSec(uint32_t current, bool increment) {
  static const uint32_t values[] = {10, 30, 60, 120, 300, 600};
  return cycleInTable(current, values, sizeof(values) / sizeof(values[0]), increment);
}


bool settingsTapShouldEditSelectedRow(int selectedRow, int tappedRow) {
  return selectedRow >= 0 && selectedRow == tappedRow;
}
