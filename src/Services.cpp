#include "Services.h"
#include "BoardConfig.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <SD.h>
#include "WebUtils.h"
#include "StatusApiLogic.h"
#include "GpsBestFitLogic.h"
#include "BleAdvertLogic.h"
#include "CacheApiLogic.h"
#include "RadioApiLogic.h"
#include "MeshtasticApiLogic.h"
#include "AppManagementApiLogic.h"

static double bearingDeg(double lat1, double lon1, double lat2, double lon2) {
  const double d2r = PI / 180.0, r2d = 180.0 / PI;
  lat1 *= d2r; lat2 *= d2r; double dLon = (lon2-lon1) * d2r;
  double y = sin(dLon) * cos(lat2);
  double x = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(dLon);
  double b = atan2(y, x) * r2d;
  return fmod(b + 360.0, 360.0);
}

bool GPSService::begin() {
  _serial.begin(BoardConfig::GPS_BAUD, SERIAL_8N1, BoardConfig::PIN_GPS_RX, BoardConfig::PIN_GPS_TX);
  return true;
}
void GPSService::update() {
  while (_serial.available()) _gps.encode(_serial.read());
  if (_gps.location.isUpdated()) {
    _prev = _fix;
    _fix.valid = _gps.location.isValid();
    _fix.lat = _gps.location.lat();
    _fix.lon = _gps.location.lng();
    _fix.ageMs = _gps.location.age();
    _fix.sats = _gps.satellites.isValid() ? _gps.satellites.value() : 0;
    _fix.hdop = _gps.hdop.isValid() ? _gps.hdop.hdop() : 99;
    _fix.altM = _gps.altitude.isValid() ? _gps.altitude.meters() : 0;
    _fix.speedKmph = _gps.speed.isValid() ? _gps.speed.kmph() : 0;
    _fix.courseDeg = _gps.course.isValid() ? _gps.course.deg() : 0;
    computeHeading();
    updateBestFit();
  }
}
void GPSService::updateBestFit() {
  _history.push_back(_fix);
  if (_history.size() > 8) _history.pop_front();
  std::vector<GpsFix> v(_history.begin(), _history.end());
  _bestFit = computeBestFitFix(v, 0.02);
}

void GPSService::computeHeading() {
  if (!_fix.valid || !_prev.valid) { _headingReliable=false; return; }
  if (_fix.speedKmph < 2.0) { _headingReliable=false; return; }
  double dLat = abs(_fix.lat - _prev.lat), dLon = abs(_fix.lon - _prev.lon);
  if (dLat + dLon < 0.000005) { _headingReliable=false; return; }
  _heading = bearingDeg(_prev.lat, _prev.lon, _fix.lat, _fix.lon);
  _headingReliable = true;
}

bool NetworkService::begin() { WiFi.mode(WIFI_STA); return true; }
void NetworkService::update() {
  _status.wifi = WiFi.status() == WL_CONNECTED;
  _status.ssid = WiFi.SSID();
  _status.ip = WiFi.localIP();
}
bool NetworkService::connectSaved() { WiFi.begin(); return true; }
bool NetworkService::connect(const String& ssid, const String& pass) { WiFi.begin(ssid.c_str(), pass.c_str()); return true; }
NetStatus NetworkService::status() const { return _status; }
std::vector<RadioSignal> NetworkService::scanWifi() {
  std::vector<RadioSignal> out;
  int n = WiFi.scanNetworks(false, true);
  for (int i=0;i<n;i++) {
    RadioSignal s; s.kind="WiFi"; s.name=WiFi.SSID(i); s.address=WiFi.BSSIDstr(i); s.rssi=WiFi.RSSI(i); s.channel=WiFi.channel(i); s.protocol=(WiFi.encryptionType(i)==WIFI_AUTH_OPEN?"open":"secured"); s.lastSeenMs=millis(); out.push_back(s);
  }
  WiFi.scanDelete();
  return out;
}

bool CacheService::begin() { ensureLayout(); return true; }
void CacheService::ensureLayout() {
  const char* dirs[] = {"/apps","/cache","/cache/maps","/cache/weather","/cache/http","/documents","/documents/markdown","/documents/text","/webroot","/games","/gps","/gps/tracks","/gps/fixes","/meshtastic","/meshtastic/messages","/meshtastic/nodes","/meshtastic/config","/radio","/radio/scans","/config","/logs"};
  for (auto d: dirs) if (!SD.exists(d)) SD.mkdir(d);
}
String CacheService::mapTilePath(const String& provider, int z, int x, int y) const { return "/cache/maps/"+provider+"/"+String(z)+"/"+String(x)+"/"+String(y)+".tile"; }
bool CacheService::hasFile(const String& path) const { return SD.exists(path); }
bool CacheService::writeText(const String& path, const String& text) { File f=SD.open(path, FILE_WRITE); if(!f)return false; f.print(text); f.close(); return true; }
String CacheService::readText(const String& path, size_t maxLen) { File f=SD.open(path, FILE_READ); if(!f)return ""; String s; while(f.available() && s.length()<maxLen) s+=(char)f.read(); f.close(); return s; }
void CacheService::appendLog(const String& name, const String& line) { File f=SD.open("/logs/"+name, FILE_APPEND); if(f){ f.println(line); f.close(); } }
void CacheService::recordMapCacheLookup(bool hit) {
  if (hit) _mapCacheHits++;
  else _mapCacheMisses++;
  _lastMapCacheLookupMs = millis();
  appendLog("map_cache.log", String(hit ? "hit" : "miss") + ",hits=" + String(_mapCacheHits) + ",misses=" + String(_mapCacheMisses));
}

bool RadioService::begin() {
  static Module mod(BoardConfig::PIN_LORA_NSS, BoardConfig::PIN_LORA_DIO1, BoardConfig::PIN_LORA_RST, BoardConfig::PIN_LORA_BUSY);
  static SX1262 lora(&mod);
  _lora = &lora;
  int state = _lora->begin(BoardConfig::LORA_FREQ_MHZ);
  Serial.printf("LoRa begin state=%d\n", state);
  return state == RADIOLIB_ERR_NONE;
}
std::vector<RadioSignal> RadioService::scanBLE(uint32_t ms) {
  std::vector<RadioSignal> out;
  NimBLEDevice::init("");
  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setActiveScan(false);
  scan->start(ms/1000, false);
  NimBLEScanResults res = scan->getResults();
  for (int i=0;i<res.getCount();i++) {
    auto d = res.getDevice(i);
    if (!d) continue;
    int svcCount = d->getServiceUUIDCount();
    int mfgLen = d->haveManufacturerData() ? d->getManufacturerData().length() : 0;
    RadioSignal s;
    s.kind="BLE";
    s.name=d->getName().c_str();
    s.address=d->getAddress().toString().c_str();
    s.rssi=d->getRSSI();
    s.protocol=buildBleAdvertSummary(s.name, s.rssi, svcCount, mfgLen);
    s.extra = d->toString().c_str();
    s.lastSeenMs=millis();
    out.push_back(s);
  }
  scan->clearResults();
  return out;
}
std::vector<RadioSignal> RadioService::scanLoRaWindow(uint32_t ms) {
  std::vector<RadioSignal> out;
  if (!_lora) return out;
  uint32_t end=millis()+ms;
  while((int32_t)(millis()-end)<0) {
    String str;
    int state = _lora->receive(str);
    if(state == RADIOLIB_ERR_NONE) {
      RadioSignal s; s.kind="LoRa"; s.name="packet"; s.rssi=(int)_lora->getRSSI(); s.protocol="LoRa raw"; s.extra=str; s.lastSeenMs=millis(); out.push_back(s);
    }
    delay(10);
  }
  return out;
}

bool WebServerService::begin() { return true; }
void WebServerService::attachContext(BoardHAL* board, GPSService* gps, NetworkService* net, CacheService* cache) { _board = board; _gps = gps; _net = net; _cache = cache; }
void WebServerService::start() {
  if (_running) return;
  _server.on("/api/health", [this](){ _server.send(200, "application/json", "{\"ok\":true,\"service\":\"web\"}"); });
  _server.on("/api/status", [this](){
    if (!_board || !_gps || !_net) { _server.send(503, "application/json", "{\"error\":\"status context unavailable\"}"); return; }
    size_t msgCount = 0;
    File msgDir = SD.open("/meshtastic/messages");
    if (msgDir) { File f = msgDir.openNextFile(); while (f) { msgCount++; f = msgDir.openNextFile(); } msgDir.close(); }
    const bool unreadMessages = hasUnreadMeshtasticMessages(msgCount);
    const bool cacheActivity = _cache && hasRecentCacheActivity(_cache->lastMapCacheLookupMs(), millis(), 5UL * 60UL * 1000UL);
    String body = buildStatusApiJson(_net->status(), _gps->fix(), _board->battery(), _board->sdMounted(), _running, unreadMessages, cacheActivity);
    _server.send(200, "application/json", body);
  });
  _server.on("/api/cache/stats", [this](){
    if (!_cache) { _server.send(503, "application/json", "{\"error\":\"cache context unavailable\"}"); return; }
    _server.send(200, "application/json", buildCacheStatsJson(_cache->mapCacheHitCount(), _cache->mapCacheMissCount()));
  });
  _server.on("/api/meshtastic/stats", [this](){
    size_t msgCount = 0, nodeCount = 0;
    File msgDir = SD.open("/meshtastic/messages");
    if (msgDir) { File f = msgDir.openNextFile(); while (f) { msgCount++; f = msgDir.openNextFile(); } msgDir.close(); }
    File nodeDir = SD.open("/meshtastic/nodes");
    if (nodeDir) { File f = nodeDir.openNextFile(); while (f) { nodeCount++; f = nodeDir.openNextFile(); } nodeDir.close(); }
    _server.send(200, "application/json", buildMeshtasticStatsJson(msgCount, nodeCount));
  });
  _server.on("/api/radio/scans", [this](){
    File dir = SD.open("/radio/scans");
    if (!dir) { _server.send(404, "application/json", "{\"error\":\"missing /radio/scans\"}"); return; }
    std::vector<String> names;
    File f = dir.openNextFile();
    while (f && names.size() < 64) { names.push_back(String(f.name())); f = dir.openNextFile(); }
    dir.close();
    _server.send(200, "application/json", buildRadioScanListJson(names));
  });
  _server.on("/api/weather/cache", [this](){
    if (!SD.exists("/cache/weather/current.json")) { _server.send(404, "application/json", "{\"error\":\"missing /cache/weather/current.json\"}"); return; }
    File f = SD.open("/cache/weather/current.json", FILE_READ);
    if (!f) { _server.send(500, "application/json", "{\"error\":\"failed to read weather cache\"}"); return; }
    String body; while (f.available()) body += (char)f.read(); f.close();
    _server.send(200, "application/json", body);
  });
  _server.on("/api/apps/order", [this](){
    if (!SD.exists("/config/apps.json")) { _server.send(404, "application/json", "{\"error\":\"missing /config/apps.json\"}"); return; }
    File f = SD.open("/config/apps.json", FILE_READ);
    if (!f) { _server.send(500, "application/json", "{\"error\":\"failed to read /config/apps.json\"}"); return; }
    String body;
    while (f.available()) body += (char)f.read();
    f.close();
    _server.send(200, "application/json", body);
  });
  _server.on("/api/apps/install", HTTP_POST, [this](){
    AppManagementRequest req;
    if (!parseAppManagementRequest(_server.arg("plain"), req)) {
      _server.send(400, "application/json", buildAppManagementResultJson("install", "", false, "invalid request"));
      return;
    }
    const String manifestPath = "/apps/" + req.id + ".json";
    String manifest = "{\"id\":\"" + req.id + "\",\"sourceUrl\":\"" + req.sourceUrl + "\",\"version\":\"" + req.version + "\"}";
    File f = SD.open(manifestPath, FILE_WRITE);
    if (!f) {
      _server.send(500, "application/json", buildAppManagementResultJson("install", req.id, false, "failed to write app manifest"));
      return;
    }
    f.print(manifest);
    f.close();
    _server.send(200, "application/json", buildAppManagementResultJson("install", req.id, true, "installed"));
  });
  _server.on("/api/apps/remove", HTTP_POST, [this](){
    AppManagementRequest req;
    if (!parseAppManagementRequest(_server.arg("plain"), req)) {
      _server.send(400, "application/json", buildAppManagementResultJson("remove", "", false, "invalid request"));
      return;
    }
    const String manifestPath = "/apps/" + req.id + ".json";
    if (!SD.exists(manifestPath)) {
      _server.send(404, "application/json", buildAppManagementResultJson("remove", req.id, false, "app not installed"));
      return;
    }
    if (!SD.remove(manifestPath)) {
      _server.send(500, "application/json", buildAppManagementResultJson("remove", req.id, false, "failed to remove app"));
      return;
    }
    _server.send(200, "application/json", buildAppManagementResultJson("remove", req.id, true, "removed"));
  });
  _server.on("/api/apps/update", HTTP_POST, [this](){
    AppManagementRequest req;
    if (!parseAppManagementRequest(_server.arg("plain"), req)) {
      _server.send(400, "application/json", buildAppManagementResultJson("update", "", false, "invalid request"));
      return;
    }
    const String manifestPath = "/apps/" + req.id + ".json";
    if (!SD.exists(manifestPath)) {
      _server.send(404, "application/json", buildAppManagementResultJson("update", req.id, false, "app not installed"));
      return;
    }
    String manifest = "{\"id\":\"" + req.id + "\",\"sourceUrl\":\"" + req.sourceUrl + "\",\"version\":\"" + req.version + "\"}";
    File f = SD.open(manifestPath, FILE_WRITE);
    if (!f) {
      _server.send(500, "application/json", buildAppManagementResultJson("update", req.id, false, "failed to update app"));
      return;
    }
    f.print(manifest);
    f.close();
    _server.send(200, "application/json", buildAppManagementResultJson("update", req.id, true, "updated"));
  });
  _server.onNotFound([this](){ servePath(_server.uri()); });
  _server.begin(); _running=true;
}
void WebServerService::stop() { _server.stop(); _running=false; }
void WebServerService::update() { if(_running) _server.handleClient(); }
void WebServerService::servePath(const String& uri) {
  if(!isSafeWebUri(uri)) { _server.send(400, "text/plain", "Invalid URI"); return; }
  String path = mapUriToWebrootPath(uri);
  if(!SD.exists(path)) { _server.send(404, "text/plain", "Not found"); return; }
  File f=SD.open(path, FILE_READ);
  _server.streamFile(f, mimeTypeForPath(path));
  f.close();
}
