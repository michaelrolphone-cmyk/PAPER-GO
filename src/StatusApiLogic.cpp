#include "StatusApiLogic.h"

String buildStatusApiJson(const NetStatus& net, const GpsFix& gps, const BatteryStatus& batt, bool sdMounted, bool webRunning) {
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
  body += "}";
  return body;
}
