#include "PowerManagementLogic.h"

static bool parseUnsignedJsonValue(const String& json, const char* key, uint32_t* out) {
  String k = String("\"") + key + "\"";
  int i = json.indexOf(k);
  if (i < 0) return false;
  int c = json.indexOf(':', i + k.length());
  if (c < 0) return false;
  int start = c + 1;
  while (start < (int)json.length() && (json[start] == ' ' || json[start] == '\t' || json[start] == '\n' || json[start] == '\r')) start++;
  int end = start;
  while (end < (int)json.length() && json[end] >= '0' && json[end] <= '9') end++;
  if (end == start) return false;
  *out = (uint32_t) json.substring(start, end).toInt();
  return true;
}

static bool parseBoolJsonValue(const String& json, const char* key, bool* out) {
  String k = String("\"") + key + "\"";
  int i = json.indexOf(k);
  if (i < 0) return false;
  int c = json.indexOf(':', i + k.length());
  if (c < 0) return false;
  int start = c + 1;
  while (start < (int)json.length() && (json[start] == ' ' || json[start] == '\t' || json[start] == '\n' || json[start] == '\r')) start++;
  if (json.startsWith("true", start)) { *out = true; return true; }
  if (json.startsWith("false", start)) { *out = false; return true; }
  return false;
}

PowerAction evaluatePowerAction(const PowerPolicy& policy, const PowerState& state, uint32_t nowMs) {
  uint32_t idleMs = nowMs - state.lastInteractionMs;
  if (!state.lockScreenActive && idleMs >= policy.lockTimeoutMs) return PowerAction::EnterLockScreen;
  if (policy.allowDeepSleep && state.lockScreenActive && idleMs >= policy.deepSleepTimeoutMs) return PowerAction::EnterDeepSleep;
  return PowerAction::None;
}

bool shouldDisableWifiForLowPower(bool lockScreenActive, bool charging) {
  return lockScreenActive && !charging;
}

PowerConfig parsePowerConfig(const String& json) {
  PowerConfig cfg;
  uint32_t lockTimeoutMs = 0;
  uint32_t deepSleepTimeoutMs = 0;
  bool allowDeepSleep = true;
  uint32_t deepSleepDurationSec = 60;

  bool hasLock = parseUnsignedJsonValue(json, "lockTimeoutMs", &lockTimeoutMs);
  bool hasDeepSleep = parseUnsignedJsonValue(json, "deepSleepTimeoutMs", &deepSleepTimeoutMs);
  bool hasAllow = parseBoolJsonValue(json, "allowDeepSleep", &allowDeepSleep);
  bool hasSleepSec = parseUnsignedJsonValue(json, "deepSleepDurationSec", &deepSleepDurationSec);

  if (hasLock) cfg.policy.lockTimeoutMs = lockTimeoutMs;
  if (hasDeepSleep) cfg.policy.deepSleepTimeoutMs = deepSleepTimeoutMs;
  if (hasAllow) cfg.policy.allowDeepSleep = allowDeepSleep;
  if (hasSleepSec) cfg.policy.deepSleepDurationSec = deepSleepDurationSec;

  if (cfg.policy.lockTimeoutMs < 1000) cfg.policy.lockTimeoutMs = 1000;
  if (cfg.policy.deepSleepTimeoutMs < cfg.policy.lockTimeoutMs) cfg.policy.deepSleepTimeoutMs = cfg.policy.lockTimeoutMs;

  if (cfg.policy.deepSleepDurationSec < 10) cfg.policy.deepSleepDurationSec = 10;

  cfg.valid = hasLock || hasDeepSleep || hasAllow || hasSleepSec;
  return cfg;
}
