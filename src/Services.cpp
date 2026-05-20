#include "Services.h"
#include "BoardConfig.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <SD.h>
#include "WebUtils.h"
#include "StatusApiLogic.h"
#include "GpsBestFitLogic.h"
#include "GpsLogLogic.h"
#include "GpsConfigLogic.h"
#include "GpsTrackApiLogic.h"
#include "BleAdvertLogic.h"
#include "CacheApiLogic.h"
#include "RadioApiLogic.h"
#include "MeshtasticApiLogic.h"
#include "AppManagementApiLogic.h"
#include "WifiConfigLogic.h"
#include "PowerManagementLogic.h"
#include "PowerApiLogic.h"
#include "NetworkConnectLogic.h"
#include "DgpsApiLogic.h"
#include "MapApiLogic.h"
#include "MapPrefetchLogic.h"
#include "MapCacheOpsLogic.h"
#include "FileApiLogic.h"
#include "MapConfigLogic.h"

namespace {
static Module dgpsMod(BoardConfig::PIN_LORA_NSS, BoardConfig::PIN_LORA_DIO1, BoardConfig::PIN_LORA_RST, BoardConfig::PIN_LORA_BUSY);
static SX1262 dgpsRadio(&dgpsMod);
static bool dgpsRadioInit = false;
}

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
  if (!dgpsRadioInit) {
    dgpsRadioInit = (dgpsRadio.begin(BoardConfig::LORA_FREQ_MHZ) == RADIOLIB_ERR_NONE);
  }
  if (dgpsRadioInit) {
    uint8_t buf[sizeof(DgpsCorrectionPayloadV1)]{};
    size_t len = sizeof(buf);
    int rs = dgpsRadio.receive(buf, len);
    if (rs == RADIOLIB_ERR_NONE) {
      handleReceivedCorrection(buf, len, millis(), dgpsRadio.getRSSI(), dgpsRadio.getSNR(), _dgpsState, _dgpsPacketStats);
    }
  }

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
    _fix.epoch = (_gps.date.isValid() && _gps.time.isValid())
      ? buildGpsEpochSeconds(_gps.date.year(), _gps.date.month(), _gps.date.day(), _gps.time.hour(), _gps.time.minute(), _gps.time.second())
      : 0;
    _dgpsQuality = evaluateDgpsQuality(_fix, millis(), _dgpsState);
    _dgpsAgeMs = _dgpsState.hasRecentPacket ? (millis() - _dgpsState.lastReceiveMillis) : 0;
    _usingDgps = computeCorrectedRoverPosition(_fix, millis(), _dgpsState, 0.5);
    _correctedFix = _fix;
    if (_usingDgps) {
      const double latRad = _dgpsState.baseOriginLatDeg * PI / 180.0;
      const double northM = _dgpsState.roverCorrectedNCm / 100.0;
      const double eastM = _dgpsState.roverCorrectedECm / 100.0;
      _correctedFix.lat = _dgpsState.baseOriginLatDeg + (northM / 6378137.0) * 180.0 / PI;
      _correctedFix.lon = _dgpsState.baseOriginLonDeg + (eastM / (6378137.0 * cos(latRad))) * 180.0 / PI;
      _correctedFix.altM = _dgpsState.baseOriginAltM + _dgpsState.roverCorrectedUCm / 100.0;
    }
    computeHeading();
    updateBestFit();
    logFixIfNeeded();
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



void GPSService::refreshTrackLoggingConfig() {
  uint32_t now = millis();
  if ((int32_t)(now - _nextTrackConfigRefreshMs) < 0) return;
  _nextTrackConfigRefreshMs = now + 10000;

  if (!SD.cardType()) {
    _trackLoggingEnabled = false;
    return;
  }
  File f = SD.open("/config/device.json", FILE_READ);
  if (!f) {
    _trackLoggingEnabled = false;
    return;
  }
  String body;
  while (f.available() && body.length() < 1024) body += (char)f.read();
  f.close();
  _trackLoggingEnabled = parseGpsTrackLoggingEnabled(body, false);
}

void GPSService::logFixIfNeeded() {
  refreshTrackLoggingConfig();
  if (!_trackLoggingEnabled) return;
  if (!SD.cardType()) return;
  if (!shouldLogGpsFix(_fix, _lastLoggedFix, 5)) return;

  File f = SD.open("/gps/tracks/current_track.csv", FILE_APPEND);
  if (!f) return;
  f.println(buildGpsTrackCsvLine(_fix, _heading, _headingReliable));
  f.close();
  _lastLoggedFix = _fix;
}
bool NetworkService::begin() {
  WiFi.mode(WIFI_STA);
  return connectSaved();
}
void NetworkService::attachCache(CacheService* cache) { _cache = cache; }
void NetworkService::update() {
  _status.wifi = WiFi.status() == WL_CONNECTED;
  _status.ssid = WiFi.SSID();
  _status.ip = WiFi.localIP();
}
bool NetworkService::connectSaved() {
  String raw;
  if (_cache) raw = _cache->readText("/config/wifi.json", 1024);
  if (!shouldAttemptSavedWifiConnect(raw, _cache != nullptr)) return false;
  WifiConfig cfg = parseWifiConfig(raw);
  WiFi.begin(cfg.ssid.c_str(), cfg.password.c_str());
  return true;
}
bool NetworkService::connect(const String& ssid, const String& pass) {
  WiFi.begin(ssid.c_str(), pass.c_str());
  return saveCredentials(ssid, pass);
}
bool NetworkService::disconnect() { WiFi.disconnect(); return true; }
bool NetworkService::forgetSaved() {
  if (!_cache) return false;
  return _cache->writeText("/config/wifi.json", "{}");
}
bool NetworkService::saveCredentials(const String& ssid, const String& pass) {
  if (!_cache) return false;
  return _cache->writeText("/config/wifi.json", buildWifiConfigJson(ssid, pass));
}
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
  if (!SD.cardType()) return;
  const char* dirs[] = {"/apps","/cache","/cache/maps","/cache/weather","/cache/http","/documents","/documents/markdown","/documents/text","/webroot","/games","/gps","/gps/tracks","/gps/fixes","/meshtastic","/meshtastic/messages","/meshtastic/nodes","/meshtastic/config","/radio","/radio/scans","/config","/logs"};
  for (auto d: dirs) if (!SD.exists(d)) SD.mkdir(d);
}
String CacheService::mapTilePath(const String& provider, int z, int x, int y) const { return "/cache/maps/"+provider+"/"+String(z)+"/"+String(x)+"/"+String(y)+".tile"; }
bool CacheService::hasFile(const String& path) const { return SD.exists(path); }
bool CacheService::writeText(const String& path, const String& text) {
  if (SD.exists(path)) SD.remove(path);
  File f = SD.open(path, FILE_WRITE);
  if (!f) return false;
  size_t wrote = f.print(text);
  f.close();
  return wrote == text.length();
}
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
  _server.on("/api/power/policy", [this](){
    if (!_cache) { _server.send(503, "application/json", "{\"error\":\"cache context unavailable\"}"); return; }
    String raw = _cache->readText("/config/power.json", 1024);
    PowerConfig cfg = parsePowerConfig(raw);
    _server.send(200, "application/json", buildPowerPolicyJson(cfg.policy, cfg.valid));
  });
  _server.on("/api/cache/stats", [this](){
    if (!_cache) { _server.send(503, "application/json", "{\"error\":\"cache context unavailable\"}"); return; }
    _server.send(200, "application/json", buildCacheStatsJson(_cache->mapCacheHitCount(), _cache->mapCacheMissCount()));
  });
  _server.on("/api/meshtastic/stats", [this](){
    if (!_board || !_board->sdMounted()) { _server.send(503, "application/json", "{\"error\":\"sd not mounted\"}"); return; }
    size_t msgCount = 0, nodeCount = 0;
    File msgDir = SD.open("/meshtastic/messages");
    if (msgDir) { File f = msgDir.openNextFile(); while (f) { msgCount++; f = msgDir.openNextFile(); } msgDir.close(); }
    File nodeDir = SD.open("/meshtastic/nodes");
    if (nodeDir) { File f = nodeDir.openNextFile(); while (f) { nodeCount++; f = nodeDir.openNextFile(); } nodeDir.close(); }
    _server.send(200, "application/json", buildMeshtasticStatsJson(msgCount, nodeCount));
  });
  _server.on("/api/radio/scans", [this](){
    if (!_board || !_board->sdMounted()) { _server.send(503, "application/json", "{\"error\":\"sd not mounted\"}"); return; }
    File dir = SD.open("/radio/scans");
    if (!dir) { _server.send(404, "application/json", "{\"error\":\"missing /radio/scans\"}"); return; }
    std::vector<String> names;
    File f = dir.openNextFile();
    while (f && names.size() < 64) { names.push_back(String(f.name())); f = dir.openNextFile(); }
    dir.close();
    _server.send(200, "application/json", buildRadioScanListJson(names));
  });
  _server.on("/api/radio/control", [this](){
    if (!_cache) { _server.send(503, "application/json", "{\"error\":\"cache context unavailable\"}"); return; }
    String raw = _cache->readText("/config/radio.json", 1024);
    RadioControlConfig cfg = parseRadioControlConfig(raw);
    _server.send(200, "application/json", buildRadioControlJson(cfg));
  });
  _server.on("/api/radio/control", HTTP_POST, [this](){
    if (!_cache) { _server.send(503, "application/json", "{\"error\":\"cache context unavailable\"}"); return; }
    RadioControlConfig cfg = parseRadioControlConfig(_server.arg("plain"));
    if (!cfg.valid) {
      _server.send(400, "application/json", "{\"error\":\"invalid radio control payload\"}");
      return;
    }
    bool ok = _cache->writeText("/config/radio.json", buildRadioControlJson(cfg));
    _server.send(ok ? 200 : 500, "application/json", String("{\"ok\":") + (ok ? "true" : "false") + "}");
  });

  _server.on("/api/gps/status", [this](){
    if (!_gps) { _server.send(503, "application/json", "{\"error\":\"gps context unavailable\"}"); return; }
    _server.send(200, "application/json", buildDgpsStatusJson(_gps->fix(), _gps->activeFix(), _gps->usingDgps(), _gps->dgpsQuality(), _gps->dgpsAgeMs(), _gps->dgpsState(), _gps->dgpsPacketStats()));
  });
  _server.on("/api/gps/dgps", [this](){
    if (!_gps) { _server.send(503, "application/json", "{\"error\":\"gps context unavailable\"}"); return; }
    _server.send(200, "application/json", buildDgpsPacketJson(_gps->dgpsState(), _gps->dgpsAgeMs()));
  });
  _server.on("/api/maps/status", [this](){
    if (!_gps || !_cache) { _server.send(503, "application/json", "{\"error\":\"map context unavailable\"}"); return; }
    MapConfig cfg = parseMapConfig(_cache->readText("/config/maps.json", 1024));
    if (!cfg.valid) cfg = sanitizeMapConfig(MapConfig{});
    MapTileCoord center = mapTileFromFix(_gps->activeFix(), (uint8_t)cfg.zoom);
    uint16_t total=0,cached=0;
    if (center.valid) {
      for (int dy=-1; dy<=1; ++dy) for (int dx=-1; dx<=1; ++dx) {
        int32_t tx=center.x+dx, ty=center.y+dy; if(tx<0||ty<0) continue; total++;
        bool hit=_cache->hasFile(_cache->mapTilePath(cfg.provider, cfg.zoom, (uint32_t)tx, (uint32_t)ty)); if(hit) cached++;
      }
    }
    CacheCoverageState cov = deriveCacheCoverageState(total, cached);
    _server.send(200, "application/json", buildMapsStatusJson(cfg, center, cov, _cache->mapCacheHitCount(), _cache->mapCacheMissCount(), _gps->usingDgps(), dgpsQualityStateLabel(_gps->dgpsQuality())));
  });
  _server.on("/api/maps/prefetch", HTTP_POST, [this](){
    if (!_cache) { _server.send(503, "application/json", "{\"error\":\"cache context unavailable\"}"); return; }
    MapPrefetchRequest req = parseMapPrefetchRequest(_server.arg("plain"));
    if (!req.valid) { _server.send(400, "application/json", "{\"error\":\"invalid prefetch payload\"}"); return; }
    String jobId = String(millis());
    _cache->appendLog("map_prefetch.log", buildMapPrefetchJobLine(jobId, req));
    _server.send(200, "application/json", buildMapsPrefetchResultJson(true, jobId));
  });
  _server.on("/api/maps/cache", [this](){
    if (!_board || !_board->sdMounted()) { _server.send(503, "application/json", "{\"error\":\"sd not mounted\"}"); return; }
    String provider = "default";
    if (_cache) { MapConfig cfg = parseMapConfig(_cache->readText("/config/maps.json", 1024)); if (cfg.valid) provider = cfg.provider; }
    String root = String("/cache/maps/") + provider;
    File dir = SD.open(root);
    if (!dir) { _server.send(200, "application/json", buildMapsCacheStatsJson(provider, 0, 0)); return; }
    uint32_t files=0; uint64_t bytes=0; File f=dir.openNextFile();
    while (f) { if (!f.isDirectory()) { files++; bytes += f.size(); } f = dir.openNextFile(); }
    dir.close();
    _server.send(200, "application/json", buildMapsCacheStatsJson(provider, files, bytes));
  });
  _server.on("/api/maps/cache/clear", HTTP_POST, [this](){
    if (!_board || !_board->sdMounted()) { _server.send(503, "application/json", "{\"error\":\"sd not mounted\"}"); return; }
    String provider = "default";
    if (_cache) { MapConfig cfg = parseMapConfig(_cache->readText("/config/maps.json", 1024)); if (cfg.valid) provider = cfg.provider; }
    String root = String("/cache/maps/") + provider;
    File dir = SD.open(root);
    if (!dir) { _server.send(200, "application/json", buildMapsCacheClearJson(true, provider, 0)); return; }
    uint32_t removed = removeFilesRecursively(SD, root);
    dir.close();
    _server.send(200, "application/json", buildMapsCacheClearJson(true, provider, removed));
  });
  _server.on("/api/files/list", [this](){
    if (!_board || !_board->sdMounted()) { _server.send(503, "application/json", fileApiErrorJson("io_error","sd not mounted")); return; }
    String path = normalizeFilePath(_server.arg("path"));
    if (!isAllowedFilePath(path)) { _server.send(403, "application/json", fileApiErrorJson("forbidden_path","Path outside allowed roots")); return; }
    int page = _server.hasArg("page") ? _server.arg("page").toInt() : 0;
    int pageSize = _server.hasArg("pageSize") ? _server.arg("pageSize").toInt() : 100;
    if (page < 0) page = 0; if (pageSize <= 0) pageSize = 100; if (pageSize > 500) pageSize = 500;
    File dir = SD.open(path);
    if (!dir || !dir.isDirectory()) { _server.send(404, "application/json", fileApiErrorJson("not_found","Directory not found")); return; }
    std::vector<FileListEntry> all;
    File f = dir.openNextFile();
    while (f) { FileListEntry e; e.name=String(f.name()); e.path=String(f.name()); e.dir=f.isDirectory(); e.size=(uint32_t)f.size(); all.push_back(e); f = dir.openNextFile(); }
    dir.close();
    int total = (int)all.size();
    int start = page * pageSize;
    std::vector<FileListEntry> entries;
    for (int i=start;i<start+pageSize && i<total;i++) entries.push_back(all[(size_t)i]);
    _server.send(200, "application/json", fileApiOkJson(fileListJson(path, page, pageSize, total, entries)));
  });
  _server.on("/api/files/delete", HTTP_POST, [this](){
    if (!_board || !_board->sdMounted()) { _server.send(503, "application/json", fileApiErrorJson("io_error","sd not mounted")); return; }
    String body = _server.arg("plain");
    int p0 = body.indexOf("\"path\":\"");
    if (p0 < 0) { _server.send(400, "application/json", fileApiErrorJson("invalid_request","path missing")); return; }
    p0 += 8; int p1 = body.indexOf('"', p0); String path = normalizeFilePath(body.substring(p0, p1));
    if (!isAllowedFilePath(path)) { _server.send(403, "application/json", fileApiErrorJson("forbidden_path","Path outside allowed roots")); return; }
    bool ok = SD.remove(path);
    if (!ok) { _server.send(500, "application/json", fileApiErrorJson("io_error","delete failed")); return; }
    _server.send(200, "application/json", fileApiOkJson("{\"path\":\""+path+"\"}"));
  });
  _server.on("/api/gps/tracks", [this](){
    if (!_board || !_board->sdMounted()) { _server.send(503, "application/json", "{\"error\":\"sd not mounted\"}"); return; }
    File dir = SD.open("/gps/tracks");
    if (!dir) { _server.send(404, "application/json", "{\"error\":\"missing /gps/tracks\"}"); return; }
    String body = "{\"files\":[";
    bool first = true;
    File f = dir.openNextFile();
    while (f) {
      if (!first) body += ",";
      first = false;
      body += "{\"name\":\"" + gpsTrackEntryNameForResponse(String(f.name())) + "\",\"size\":" + String((uint32_t)f.size()) + "}";
      f = dir.openNextFile();
    }
    dir.close();
    body += "]}";
    _server.send(200, "application/json", body);
  });
  _server.on("/api/gps/tracks/clear", HTTP_POST, [this](){
    if (!_board || !_board->sdMounted()) { _server.send(503, "application/json", "{\"error\":\"sd not mounted\"}"); return; }
    File dir = SD.open("/gps/tracks");
    if (!dir) { _server.send(404, "application/json", "{\"error\":\"missing /gps/tracks\"}"); return; }
    size_t removed = 0;
    File f = dir.openNextFile();
    while (f) {
      String path = gpsTrackEntryPathForRemoval(String(f.name()));
      if (path.length() && SD.remove(path)) removed++;
      f = dir.openNextFile();
    }
    dir.close();
    _server.send(200, "application/json", String("{\"ok\":true,\"removed\":") + String((uint32_t)removed) + "}");
  });
  _server.on("/api/weather/cache", [this](){
    if (!_board || !_board->sdMounted()) { _server.send(503, "application/json", "{\"error\":\"sd not mounted\"}"); return; }
    if (!SD.exists("/cache/weather/current.json")) { _server.send(404, "application/json", "{\"error\":\"missing /cache/weather/current.json\"}"); return; }
    File f = SD.open("/cache/weather/current.json", FILE_READ);
    if (!f) { _server.send(500, "application/json", "{\"error\":\"failed to read weather cache\"}"); return; }
    String body; while (f.available()) body += (char)f.read(); f.close();
    _server.send(200, "application/json", body);
  });
  _server.on("/api/apps/order", [this](){
    if (!_board || !_board->sdMounted()) { _server.send(503, "application/json", "{\"error\":\"sd not mounted\"}"); return; }
    if (!SD.exists("/config/apps.json")) { _server.send(404, "application/json", "{\"error\":\"missing /config/apps.json\"}"); return; }
    File f = SD.open("/config/apps.json", FILE_READ);
    if (!f) { _server.send(500, "application/json", "{\"error\":\"failed to read /config/apps.json\"}"); return; }
    String body;
    while (f.available()) body += (char)f.read();
    f.close();
    _server.send(200, "application/json", body);
  });
  _server.on("/api/apps/install", HTTP_POST, [this](){
    if (!_board || !_board->sdMounted()) {
      _server.send(503, "application/json", buildAppManagementResultJson("install", "", false, "sd not mounted"));
      return;
    }
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
    if (!_board || !_board->sdMounted()) {
      _server.send(503, "application/json", buildAppManagementResultJson("remove", "", false, "sd not mounted"));
      return;
    }
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
    if (!_board || !_board->sdMounted()) {
      _server.send(503, "application/json", buildAppManagementResultJson("update", "", false, "sd not mounted"));
      return;
    }
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
  if (!_board || !_board->sdMounted()) { _server.send(503, "text/plain", "SD not mounted"); return; }
  if(!isSafeWebUri(uri)) { _server.send(400, "text/plain", "Invalid URI"); return; }
  String path = mapUriToWebrootPath(uri);
  if(!SD.exists(path)) { _server.send(404, "text/plain", "Not found"); return; }
  File f=SD.open(path, FILE_READ);
  _server.streamFile(f, mimeTypeForPath(path));
  f.close();
}
