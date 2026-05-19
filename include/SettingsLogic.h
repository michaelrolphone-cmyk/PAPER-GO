#pragma once

#include <Arduino.h>
#include "PowerManagementLogic.h"

struct SettingsViewState {
  String ssid = "";
  String password = "";
  PowerPolicy power;
  bool hasWifiConfig = false;
  bool hasPowerConfig = false;
  int selectedRow = 0;
};

String buildPowerConfigJson(const PowerPolicy& policy);
int settingsRowFromTapY(int tapY, int startY, int rowHeight, int rowCount);
uint32_t cycleLockTimeoutMs(uint32_t current, bool increment);
uint32_t cycleDeepSleepTimeoutMs(uint32_t current, bool increment);
uint32_t cycleDeepSleepDurationSec(uint32_t current, bool increment);

bool settingsTapShouldEditSelectedRow(int selectedRow, int tappedRow);
