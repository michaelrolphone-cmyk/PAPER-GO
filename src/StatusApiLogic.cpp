#include "StatusApiLogic.h"

String buildStatusApiJson(const NetStatus& net, const GpsFix& gps, const BatteryStatus& batt, bool sdMounted, bool webRunning, bool unreadMessages, bool cacheActivity) {
  String body = "{";
  body += "\"wifi\":" + String(net.wifi ? "true" : "false");
  body += ",\"ip\":\"" + net.ip.toString() + "\"";
  body += ",\"gpsValid\":" + String(gps.valid ? "true" : "false");
  body += ",\"lat\":" + String(gps.lat, 6);
  body += ",\"lon\":" + String(gps.lon, 6);
  body += ",\"batteryPercent\":" + String(batt.percent);
  body += ",\"charging\":" + String(batt.charging ? "true" : "false");
  body += ",\"sdMounted\":" + String(sdMounted ? "true" : "false");
  body += ",\"webRunning\":" + String(webRunning ? "true" : "false");
  body += ",\"unreadMessages\":" + String(unreadMessages ? "true" : "false");
  body += ",\"cacheActivity\":" + String(cacheActivity ? "true" : "false");
  body += "}";
  return body;
}

bool hasUnreadMeshtasticMessages(size_t messageFiles) {
  return messageFiles > 0;
}

bool hasRecentCacheActivity(uint32_t lastLookupMs, uint32_t nowMs, uint32_t activeWindowMs) {
  if (lastLookupMs == 0 || activeWindowMs == 0) return false;
  if (nowMs < lastLookupMs) return false;
  return (nowMs - lastLookupMs) <= activeWindowMs;
}
