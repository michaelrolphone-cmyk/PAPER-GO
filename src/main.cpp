#include <Arduino.h>
#include "BoardHAL.h"
#include "BoardConfig.h"
#include "Services.h"
#include "SystemServices.h"
#include "AppManager.h"
#include "Apps.h"
#include "BootLogLogic.h"

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
  bool boardOk = board.begin();
  Serial.println(bootStepLog("board.begin", boardOk, String("sd=") + boolLabel(board.sdMounted())));

  services.board=&board;
  services.gps=&gps;
  services.net=&net;
  services.cache=&cache;
  services.radio=&radio;
  services.web=&web;
  Serial.println(bootStepLog("services.bind", true, "board/gps/net/cache/radio/web"));

  bool cacheOk = false;
  if(board.sdMounted()) cacheOk = cache.begin();
  Serial.println(bootStepLog("cache.begin", cacheOk, board.sdMounted() ? "layout ensured" : "sd not mounted"));

  net.attachCache(&cache);
  Serial.println(bootStepLog("net.attachCache", true, "/config/wifi.json enabled"));

  bool gpsOk = gps.begin();
  Serial.println(bootStepLog("gps.begin", gpsOk, String("uart=") + BoardConfig::PIN_GPS_RX + "," + BoardConfig::PIN_GPS_TX));

  bool netOk = net.begin();
  NetStatus ns = net.status();
  Serial.println(bootStepLog("net.begin", netOk, String("wifi=") + boolLabel(ns.wifi) + ", ip=" + ns.ip.toString()));

  bool radioOk = radio.begin();
  Serial.println(bootStepLog("radio.begin", radioOk, String("freq=") + String(BoardConfig::LORA_FREQ_MHZ, 1) + "MHz"));

  web.attachContext(&board, &gps, &net, &cache);
  Serial.println(bootStepLog("web.attachContext", true));
  bool webOk = web.begin();
  Serial.println(bootStepLog("web.begin", webOk));

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
