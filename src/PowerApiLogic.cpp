#include "PowerApiLogic.h"

String buildPowerPolicyJson(const PowerPolicy& policy, bool configPresent) {
  String body = "{";
  body += "\"configPresent\":" + String(configPresent ? "true" : "false");
  body += ",\"lockTimeoutMs\":" + String(policy.lockTimeoutMs);
  body += ",\"deepSleepTimeoutMs\":" + String(policy.deepSleepTimeoutMs);
  body += ",\"allowDeepSleep\":" + String(policy.allowDeepSleep ? "true" : "false");
  body += ",\"deepSleepDurationSec\":" + String(policy.deepSleepDurationSec);
  body += "}";
  return body;
}
