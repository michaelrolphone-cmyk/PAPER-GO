#pragma once

#include <Arduino.h>

enum class PowerAction {
  None,
  EnterLockScreen,
  EnterDeepSleep
};

struct PowerPolicy {
  uint32_t lockTimeoutMs = 30000;
  uint32_t deepSleepTimeoutMs = 120000;
  bool allowDeepSleep = true;
  uint32_t deepSleepDurationSec = 60;
};

struct PowerState {
  uint32_t lastInteractionMs = 0;
  bool lockScreenActive = false;
};

struct PowerConfig {
  PowerPolicy policy;
  bool valid = false;
};

PowerAction evaluatePowerAction(const PowerPolicy& policy, const PowerState& state, uint32_t nowMs);
bool shouldDisableWifiForLowPower(bool lockScreenActive, bool charging);
bool shouldPowerButtonEnterLowPower(const String& activeAppId);
bool shouldPowerButtonReturnToOnline(const String& activeAppId);
PowerConfig parsePowerConfig(const String& json);
