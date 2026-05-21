#include "HardwareApiLogic.h"

String buildHardwareStatusJson(const HardwareStatus& hs) {
  String body = "{";
  body += "\"flash_ok\":" + String(hs.flashOk ? "true" : "false");
  body += ",\"psram_ok\":" + String(hs.psramOk ? "true" : "false");
  body += ",\"sd_mounted\":" + String(hs.sdMounted ? "true" : "false");
  body += ",\"wifi_ready\":" + String(hs.wifiReady ? "true" : "false");
  body += ",\"ble_ready\":" + String(hs.bleReady ? "true" : "false");
  body += ",\"lora_ready\":" + String(hs.loraReady ? "true" : "false");
  body += ",\"gps_ready\":" + String(hs.gpsReady ? "true" : "false");
  body += ",\"epd_ready\":" + String(hs.epdReady ? "true" : "false");
  body += ",\"touch_ready\":" + String(hs.touchReady ? "true" : "false");
  body += ",\"rtc_ready\":" + String(hs.rtcReady ? "true" : "false");
  body += ",\"tps65185_ready\":" + String(hs.tps65185Ready ? "true" : "false");
  body += ",\"bq25896_ready\":" + String(hs.bq25896Ready ? "true" : "false");
  body += ",\"bq27220_ready\":" + String(hs.bq27220Ready ? "true" : "false");
  body += ",\"pca9535_ready\":" + String(hs.pca9535Ready ? "true" : "false");
  body += "}";
  return body;
}
