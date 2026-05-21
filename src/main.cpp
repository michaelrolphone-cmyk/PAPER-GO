#include <Arduino.h>
#include "BoardHAL.h"
#include "BoardConfig.h"
#include "Services.h"
#include "SystemServices.h"
#include "AppManager.h"
#include "Apps.h"
#include "BootLogLogic.h"
#include <esp_system.h>

BoardHAL board;
GPSService gps;
NetworkService net;
CacheService cache;
RadioService radio;
WebServerService web;
SystemServices services;
AppManager apps;

SpringboardApp springboard;
LockScreenApp lockscreen;
GpsMapApp gpsMap;
RadioScannerApp radioScanner;
MeshtasticApp meshtastic;
UrlFetcherApp urlFetcher;
MarkdownReaderApp markdownReader;
FileExplorerApp fileExplorer;
WeatherApp weather;
WebServerApp webApp;
GamesApp games;
SettingsApp settings;

void setup() {
  uint32_t bootStart = millis();
  uint32_t stepStart = bootStart;
  uint8_t bootOkCount = 0;
  uint8_t bootFailCount = 0;
  uint8_t bootSkippedCount = 0;
  auto logStep = [&](const String& step, bool ok, const String& detail = "") {
    uint32_t now = millis();
    Serial.println(bootStepLog(step, ok, detail));
    Serial.println(bootTimingLog(step, now - stepStart));
    stepStart = now;
    if (ok) bootOkCount++; else bootFailCount++;
  };
  auto logSkippedStep = [&](const String& step, const String& detail = "") {
    uint32_t now = millis();
    Serial.println(bootStepSkippedLog(step, detail));
    Serial.println(bootTimingLog(step, now - stepStart));
    stepStart = now;
    bootSkippedCount++;
  };

  bool boardOk = board.begin();
  logStep("board.begin", boardOk, String("sd=") + boolLabel(board.sdMounted()));

  const uint32_t flashBytes = ESP.getFlashChipSize();
  const uint32_t psramBytes = ESP.getPsramSize();
  const bool flashOk = flashBytes >= (16UL * 1024UL * 1024UL);
  const bool psramOk = psramBytes >= (8UL * 1024UL * 1024UL);
  logStep("flash.detect", flashOk, String(flashBytes / (1024UL * 1024UL)) + "MB");
  logStep("psram.detect", psramOk, String(psramBytes / (1024UL * 1024UL)) + "MB");
  logStep("touch.probe", board.touchAvailable(), String("addr=") + (board.touchAvailable() ? String(BoardConfig::GT911_ADDR, HEX) : String("none")));
  logStep("rtc.probe", board.rtcAvailable(), String("addr=0x") + String(BoardConfig::RTC_ADDR, HEX));
  logStep("tps65185.probe", board.tps65185Available(), String("addr=0x") + String(BoardConfig::TPS65185_ADDR, HEX));
  logStep("pca9535.probe", board.pca9535Available(), String("addr=0x") + String(BoardConfig::PCA9535_ADDR, HEX));
  logStep("bq25896.probe", board.bq25896Available(), String("addr=0x") + String(BoardConfig::BQ25896_ADDR, HEX));
  logStep("bq27220.probe", board.bq27220Available(), String("addr=0x") + String(BoardConfig::BQ27220_ADDR, HEX));

  services.board=&board;
  services.gps=&gps;
  services.net=&net;
  services.cache=&cache;
  services.radio=&radio;
  services.web=&web;
  logStep("services.bind", true, "board/gps/net/cache/radio/web");

  if(board.sdMounted()) {
    bool cacheOk = cache.begin();
    logStep("cache.begin", cacheOk, "layout ensured");
  } else {
    logSkippedStep("cache.begin", "sd not mounted");
  }

  net.attachCache(&cache);
  logStep("net.attachCache", true, "/config/wifi.json enabled");

  bool gpsOk = gps.begin();
  logStep("gps.begin", gpsOk, String("uart=") + BoardConfig::PIN_GPS_RX + "," + BoardConfig::PIN_GPS_TX);

  bool netOk = net.begin();
  NetStatus ns = net.status();
  logStep("net.begin", netOk, String("wifi=") + boolLabel(ns.wifi) + ", ip=" + ns.ip.toString());

  bool radioOk = radio.begin();
  logStep("radio.begin", radioOk, String("freq=") + String(BoardConfig::LORA_FREQ_MHZ, 1) + "MHz");

  web.attachContext(&board, &gps, &net, &cache, &radio);
  logStep("web.attachContext", true);
  bool webOk = web.begin();
  logStep("web.begin", webOk);

  apps.add(&springboard);
  apps.add(&lockscreen);
  apps.add(&gpsMap);
  apps.add(&radioScanner);
  apps.add(&meshtastic);
  apps.add(&urlFetcher);
  apps.add(&markdownReader);
  apps.add(&fileExplorer);
  apps.add(&weather);
  apps.add(&webApp);
  apps.add(&games);
  apps.add(&settings);
  apps.begin(services, "springboard");
  logStep("apps.begin", true, "springboard");
  Serial.println(bootSummaryLog(millis() - bootStart, bootOkCount, bootFailCount, bootSkippedCount));
}

void loop() {
  gps.update();
  net.update();
  static uint32_t lastNetLog = 0;
  if (millis() - lastNetLog > 5000) {
    lastNetLog = millis();
    NetStatus ns = net.status();
    Serial.println("[NET] wifi=" + String(ns.wifi ? "on" : "off") + " ssid=" + ns.ssid + " ip=" + ns.ip.toString());
  }
  web.update();
  apps.update(services);
  delay(25);
}
