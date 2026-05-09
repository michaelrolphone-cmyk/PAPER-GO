#include <Arduino.h>
#include "BoardHAL.h"
#include "Services.h"
#include "SystemServices.h"
#include "AppManager.h"
#include "Apps.h"

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
  board.begin();
  services.board=&board;
  services.gps=&gps;
  services.net=&net;
  services.cache=&cache;
  services.radio=&radio;
  services.web=&web;

  if(board.sdMounted()) cache.begin();
  gps.begin();
  net.begin();
  radio.begin();
  web.attachContext(&board, &gps, &net, &cache);
  web.begin();

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
  web.update();
  apps.update(services);
  delay(25);
}
