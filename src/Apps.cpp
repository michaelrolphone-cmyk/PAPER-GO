#include "Apps.h"
#include <SD.h>
#include <HTTPClient.h>
#include "AppCatalog.h"
#include "MapCacheLogic.h"
#include "RadioLogLogic.h"
#include "WeatherLogic.h"
#include "WeatherFetchLogic.h"
#include "UrlFetcherLogic.h"
#include "MarkdownLogic.h"
#include "TimeFormatLogic.h"
#include "FileExplorerLogic.h"
#include "MeshtasticLogic.h"
#include "SpringboardLogic.h"
#include "LockScreenLogic.h"
#include "MapTileLogic.h"
#include "ScaffoldNoticeLogic.h"

void SimpleListApp::titleBar(SystemServices& s, const String& t) {
  s.board->fillRect(0, BoardConfig::STATUS_BAR_H, BoardConfig::SCREEN_W, 40, 14);
  s.board->drawText(12, BoardConfig::STATUS_BAR_H+12, t, 0, 2);
  s.board->drawText(BoardConfig::SCREEN_W-90, BoardConfig::STATUS_BAR_H+12, "Home", 0, 1);
}

void SpringboardApp::onStart(SystemServices& s) {
  _orderedIds = defaultAppOrder();
  if (s.cache) {
    String cfg = s.cache->readText("/config/apps.json", 2048);
    auto loaded = parseOrderedAppIds(cfg);
    if (!loaded.empty()) _orderedIds = loaded;
  }
  _page = 0;
  _showOptions = false;
  _selectedIndex = -1;
}

void SpringboardApp::render(SystemServices& s) {
  s.board->clear(15);
  const size_t pageSize = 10;
  s.board->drawRect(6, 6, BoardConfig::SCREEN_W - 12, BoardConfig::SCREEN_H - 12, 0);
  s.board->drawRect(0, 0, BoardConfig::SCREEN_W, BoardConfig::SCREEN_H, 0);

  s.board->drawText(24, 24, "14:37 | LoRa | GPS | SD | 76%", 0, 2);
  s.board->drawLine(16, 62, BoardConfig::SCREEN_W - 16, 62, 0);
  s.board->drawText(300, 78, "PAPER GO", 0, 2);
  size_t pages = springboardPageCount(_orderedIds.size(), pageSize);
  if (_page >= pages && pages > 0) _page = pages - 1;
  size_t start = springboardPageStart(_page, pageSize);
  size_t end = min(start + pageSize, _orderedIds.size());
  const int cols=3, cellW=260, cellH=190, startX=80, startY=110;
  for(size_t i=start;i<end;i++) {
    size_t local = i - start;
    int c=local%cols, r=local/cols;
    int x = (local == 9) ? 350 : (startX + c*cellW);
    int y = (local == 9) ? 680 : (startY + r*cellH);
    s.board->drawRect(x,y,230,170,0);
    String appId = _orderedIds[i];
    bool supportsOffline = appSupportsOfflineMode(appId);
    bool wifiConnected = s.net && s.net->status().wifi;
    bool unavailable = springboardOnlineRequiredUnavailable(supportsOffline, wifiConnected);
    uint8_t titleColor = unavailable ? 5 : 0;
    s.board->drawText(x+18,y+38,appDisplayNameById(appId),titleColor,2);
    s.board->drawText(x+18,y+130,appId,7,1);
    if (!supportsOffline) {
      s.board->fillRect(x+140, y+8, 80, 20, 13);
      s.board->drawRect(x+140, y+8, 80, 20, 0);
      s.board->drawText(x+148, y+12, unavailable ? "OFFLINE" : "ONLINE", 0, 1);
    }
  }
  s.board->drawText(20, 900, String("Page ") + String(_page + 1) + "/" + String(max((size_t)1, pages)) + " swipe left/right", 0, 1);
  if (_showOptions && _selectedIndex >= 0 && _selectedIndex < (int)_orderedIds.size()) {
    s.board->fillRect(250, 190, 460, 180, 13);
    s.board->drawRect(250, 190, 460, 180, 0);
    s.board->drawText(270, 220, "App options", 0, 2);
    s.board->drawText(270, 260, "Selected: " + _orderedIds[_selectedIndex], 0, 1);
    s.board->drawText(270, 295, "Tap here to Move to Front", 0, 1);
  }
}
void SpringboardApp::handleTouch(SystemServices& s, const TouchEvent& ev) {
  const size_t pageSize = 10;
  size_t pages = springboardPageCount(_orderedIds.size(), pageSize);
  if (ev.type == TouchType::SwipeLeft && _page + 1 < pages) { _page++; return; }
  if (ev.type == TouchType::SwipeRight && _page > 0) { _page--; return; }

  if (ev.type == TouchType::LongPress) {
    int idx = springboardTappedIndexForPage(ev.x, ev.y, _page, pageSize);
    if (idx >= 0 && idx < (int)_orderedIds.size()) {
      _selectedIndex = idx;
      _showOptions = true;
    }
    return;
  }

  if(ev.type!=TouchType::Tap) return;

  if (_showOptions) {
    if (ev.x >= 270 && ev.x <= 660 && ev.y >= 280 && ev.y <= 320 && _selectedIndex >= 0) {
      springboardMoveAppToFront(_orderedIds, static_cast<size_t>(_selectedIndex));
    }
    _showOptions = false;
    _selectedIndex = -1;
    return;
  }

  int idx = springboardTappedIndexForPage(ev.x, ev.y, _page, pageSize);
  if (idx < 0 || idx >= (int)_orderedIds.size()) return;

  String appId = _orderedIds[idx];
  bool supportsOffline = appSupportsOfflineMode(appId);
  bool wifiConnected = s.net && s.net->status().wifi;
  if (!springboardCanOpenApp(supportsOffline, wifiConnected)) return;

  s.requestOpenApp = appId;
}

bool SpringboardApp::handleHomeButton(SystemServices& s) {
  (void)s;
  return springboardHandleHomePress(_page);
}

void LockScreenApp::render(SystemServices& s) {
  s.board->clear(15);
  GpsFix f=s.gps->fix();
  BatteryStatus b = s.board->battery();
  s.board->drawText(40,70,"LOCK",0,2);

  s.board->drawText(40,120, formatUtcTime(f.epoch), 0, 3);
  s.board->drawText(40,160, formatUtcDate(f.epoch), 0, 2);
  s.board->drawText(40,195, String("GPS: ") + (f.valid ? "fix" : "search"), 0, 1);
  s.board->drawText(220,195, String("BAT: ") + (b.percent >= 0 ? String(b.percent) + "%" : "?"), 0, 1);

  s.board->drawRect(40,220,420,240,0);
  s.board->drawText(60,250,"Cached map preview",0,2);
  String tilePath = lockScreenMapTilePath(f);
  bool hasTile = s.cache && tilePath.length() && s.cache->hasFile(tilePath);
  LockScreenPreviewInfo preview = buildLockScreenPreviewInfo(f, hasTile);
  s.board->drawText(60,300,preview.summary,0,1);
  s.board->drawText(60,335,String("Cache: ") + cacheCoverageLabel(preview.coverage), hasTile ? 0 : 5, 1);

  s.board->drawText(40,500,"Tap/gesture to unlock",0,1);
}

void LockScreenApp::onStart(SystemServices& s) {
  s.board->setLowlightMode(true);
}

void LockScreenApp::onStop(SystemServices& s) {
  s.board->setLowlightMode(false);
}

void LockScreenApp::handleTouch(SystemServices& s, const TouchEvent& ev) {
  if (ev.type == TouchType::Tap) s.board->toggleBacklight();
}

void GpsMapApp::update(SystemServices& s, uint32_t now) { s.gps->update(); }
void GpsMapApp::render(SystemServices& s) {
  titleBar(s, "GPS Map");
  GpsFix f=s.gps->fix();
  s.board->drawRect(20,95,620,390,0);
  s.board->drawText(220,280,"MAP PREVIEW UNAVAILABLE",7,2);
  s.board->drawText(220,312,mapAppNoticeText(),5,1);
  s.board->drawLine(330,290,370,290,0); s.board->drawLine(350,270,350,310,0);
  int x=670, y=105;
  s.board->drawText(x,y,"Fix",0,2); y+=35;
  s.board->drawText(x,y,f.valid?"valid":"no fix",f.valid?0:5,1); y+=25;
  GpsFix bf = s.gps->bestFit();
  s.board->drawText(x,y,"Lat: "+String(f.lat,6),0,1); y+=22;
  s.board->drawText(x,y,"Lon: "+String(f.lon,6),0,1); y+=22;
  s.board->drawText(x,y,"BestFit: "+(bf.valid?String(bf.lat,5)+","+String(bf.lon,5):String("n/a")),0,1); y+=22;
  s.board->drawText(x,y,"Sats: "+String(f.sats),0,1); y+=22;
  s.board->drawText(x,y,"HDOP: "+String(f.hdop,1),0,1); y+=22;
  if(s.gps->headingReliable()) s.board->drawText(x,y,"Heading: "+String(s.gps->computedHeadingDeg(),0),0,1);
  else s.board->drawText(x,y,"Heading: moving only",5,1);
  y+=35;
  MapTileCoord center = mapTileFromFix(f.valid ? f : bf, 12);
  s.board->drawText(x,y,"Tile: " + mapTileLabel(center),0,1); y+=22;
  uint16_t totalTiles = 0;
  uint16_t cachedTiles = 0;
  if (s.cache && center.valid) {
    for (int dy=-1; dy<=1; ++dy) {
      for (int dx=-1; dx<=1; ++dx) {
        int32_t tx = center.x + dx;
        int32_t ty = center.y + dy;
        if (tx < 0 || ty < 0) continue;
        ++totalTiles;
        bool hit = s.cache->hasFile(s.cache->mapTilePath("default", center.zoom, (uint32_t)tx, (uint32_t)ty));
        if (hit) ++cachedTiles;
        s.cache->recordMapCacheLookup(hit);
      }
    }
  }
  CacheCoverageState coverage = deriveCacheCoverageState(totalTiles, cachedTiles);
  s.board->drawText(x,y,"Tiles: "+String(cacheCoverageLabel(coverage))+" ("+String(cachedTiles)+"/"+String(totalTiles)+")",0,1); y+=22;
  if (s.cache) s.board->drawText(x,y,"Cache H/M: "+String(s.cache->mapCacheHitCount())+"/"+String(s.cache->mapCacheMissCount()),0,1);
  else s.board->drawText(x,y,"Cache service unavailable",5,1);
}

void RadioScannerApp::onStart(SystemServices& s) {
  _signals.clear();
  auto wifi=s.net->scanWifi(); _signals.insert(_signals.end(), wifi.begin(), wifi.end());
  auto ble=s.radio->scanBLE(1500); _signals.insert(_signals.end(), ble.begin(), ble.end());
  auto lora=s.radio->scanLoRaWindow(500); _signals.insert(_signals.end(), lora.begin(), lora.end());
  if (s.cache) {
    GpsFix fix = s.gps ? s.gps->fix() : GpsFix{};
    uint32_t ts = millis();
    String body = buildRadioScanLog(_signals, ts, fix.valid ? &fix : nullptr);
    s.cache->writeText("/radio/scans/scan-" + String(ts) + ".log", body);
  }
}
void RadioScannerApp::render(SystemServices& s) {
  titleBar(s,"Radio Scanner"); int y=100;
  s.board->drawText(20,y,"Kind  RSSI  Channel/Protocol  Name/Address",0,1); y+=26;
  for(auto& sig:_signals){
    s.board->drawText(20,y,sig.kind+"  "+String(sig.rssi)+"  "+String(sig.channel)+" "+sig.protocol+"  "+sig.name+" "+sig.address,0,1); y+=22; if(y>520)break;
  }
  if(_signals.empty()) s.board->drawText(20,y,"No signals captured in scan window.",5,1);
}

void MeshtasticApp::render(SystemServices& s) {
  titleBar(s,"Meshtastic");
  if (!s.board->sdMounted()) {
    s.board->drawText(20,110,"SD not mounted.",5,2);
    s.board->drawText(20,145,"Meshtastic storage unavailable.",5,1);
    s.board->drawText(20,175,"Insert SD to access /meshtastic data.",0,1);
    return;
  }
  size_t msgCount = 0, nodeCount = 0;
  File msgDir = SD.open("/meshtastic/messages");
  if (msgDir) { File f = msgDir.openNextFile(); while (f) { msgCount++; f = msgDir.openNextFile(); } msgDir.close(); }
  File nodeDir = SD.open("/meshtastic/nodes");
  if (nodeDir) { File f = nodeDir.openNextFile(); while (f) { nodeCount++; f = nodeDir.openNextFile(); } nodeDir.close(); }

  s.board->drawText(20,110,"Meshtastic storage boundary active.",0,2);
  s.board->drawText(20,150,"Storage: /meshtastic/messages /nodes /config",0,1);
  s.board->drawText(20,180,formatMeshtasticStorageStatus(msgCount, nodeCount),0,1);
  s.board->drawText(20,210,meshtasticAppNoticeText(),5,1);
}
void UrlFetcherApp::onStart(SystemServices& s) {
  _status = "No config";
  _preview = "";
  if (!s.cache) { _status = "Cache unavailable"; return; }
  String cfgRaw = s.cache->readText("/config/url_fetcher.json", 2048);
  UrlFetcherConfig cfg = parseUrlFetcherConfig(cfgRaw);
  if (!cfg.valid) { _status = "Invalid /config/url_fetcher.json"; return; }

  String cachePath = cachePathForUrl(cfg.url);
  if (!s.net || !s.net->status().wifi) {
    _status = "Offline: using cache";
    _preview = s.cache->readText(cachePath, 600);
    return;
  }

  HTTPClient http;
  http.setTimeout(cfg.timeoutMs);
  if (!http.begin(cfg.url)) { _status = "HTTP begin failed"; return; }
  int code = http.GET();
  if (code > 0) {
    String body = http.getString();
    s.cache->writeText(cachePath, body);
    _preview = body.substring(0, 600);
    _status = "Fetched HTTP " + String(code);
  } else {
    _status = "HTTP GET failed";
    _preview = s.cache->readText(cachePath, 600);
  }
  http.end();
}

void UrlFetcherApp::render(SystemServices& s) {
  titleBar(s,"URL Fetcher");
  s.board->drawText(20,110,"Config: /config/url_fetcher.json",0,1);
  s.board->drawText(20,140,"Status: " + _status,0,1);
  s.board->drawRect(20,180,880,260,0);
  s.board->drawText(40,210,_preview.length() ? _preview : "No cached response.",0,1);
}
void MarkdownReaderApp::onStart(SystemServices& s) {
  const String path = "/documents/markdown/readme.md";
  String progress = s.cache ? s.cache->readText("/config/markdown_progress.json", 256) : "";
  _startLine = markdownReadProgressStartLine(progress, path, 0);
  _lastMaxStart = 0;
}

void MarkdownReaderApp::render(SystemServices& s) {
  titleBar(s,"Markdown Reader");
  String path = "/documents/markdown/readme.md";
  if (!s.cache) {
    s.board->drawText(20,110,"Cache service unavailable",5,1);
    return;
  }
  String raw = s.cache->readText(path, 6000);
  if (raw.length() == 0) {
    s.board->drawText(20,110,"Missing file: " + path,5,1);
    return;
  }
  String title = markdownTitle(raw);
  String rendered = markdownRenderPreview(raw, 5000, 128, 56);
  const size_t windowLines = 16;
  _startLine = markdownClampStartLine(rendered, _startLine, windowLines);
  int clampedEnd = markdownClampStartLine(rendered, INT32_MAX, windowLines);
  _lastMaxStart = clampedEnd;
  String preview = markdownWindow(rendered, _startLine, windowLines);
  int totalPages = _lastMaxStart + 1;
  int currentPage = _startLine + 1;
  s.board->drawText(20,110,"File: " + path,0,1);
  s.board->drawText(20,138,"Title: " + title + "  (" + String(currentPage) + "/" + String(totalPages) + ")",0,1);
  s.board->drawRect(20,168,900,340,0);
  s.board->drawText(30,190,preview,0,1);
  s.board->drawText(20,515,"Swipe up for next, tap top/bottom edge for prev/next",0,1);
}

void MarkdownReaderApp::handleTouch(SystemServices& s, const TouchEvent& ev) {
  int previous = _startLine;
  if (ev.type == TouchType::SwipeUp && _startLine < _lastMaxStart) _startLine += 4;
  if (ev.type == TouchType::Tap && ev.y < (BoardConfig::STATUS_BAR_H + 80) && _startLine > 0) _startLine -= 4;
  if (ev.type == TouchType::Tap && ev.y > 500 && _startLine < _lastMaxStart) _startLine += 4;
  if (_startLine < 0) _startLine = 0;
  if (_startLine > _lastMaxStart) _startLine = _lastMaxStart;
  if (s.cache && previous != _startLine) {
    s.cache->writeText("/config/markdown_progress.json", markdownBuildProgressState("/documents/markdown/readme.md", _startLine));
  }
}

void FileExplorerApp::render(SystemServices& s) {
  titleBar(s,"Files: "+_path); int y=105;
  if (!s.board->sdMounted()) { s.board->drawText(20,y,"SD not mounted",5,1); return; }
  File root=SD.open(_path); if(!root){s.board->drawText(20,y,"Cannot open path",5,1); return;}

  std::vector<FileEntryView> entries;
  File file=root.openNextFile();
  while(file){
    FileEntryView e;
    e.name = file.name();
    e.isDir = file.isDirectory();
    e.size = file.size();
    entries.push_back(e);
    file = root.openNextFile();
  }
  root.close();

  sortFileEntries(entries);
  if (_path != "/") {
    s.board->drawText(20, y, "[D] ..", 0, 1);
    y += 22;
  }
  for (auto& e : entries) {
    if (y >= 498) break;
    s.board->drawText(20, y, formatFileEntry(e), 0, 1);
    y += 22;
  }
  s.board->drawText(20, 510, "Tap folder to enter, tap file to preview path", 0, 1);
}

void FileExplorerApp::handleTouch(SystemServices& s, const TouchEvent& ev) {
  if (ev.type != TouchType::Tap || !s.board->sdMounted()) return;
  if (ev.y < 105 || ev.y > 498) return;

  File root = SD.open(_path);
  if (!root) return;
  std::vector<FileEntryView> entries;
  File file=root.openNextFile();
  while(file){
    FileEntryView e;
    e.name = file.name();
    e.isDir = file.isDirectory();
    e.size = file.size();
    entries.push_back(e);
    file = root.openNextFile();
  }
  root.close();
  sortFileEntries(entries);

  int row = (ev.y - 105) / 22;
  int idx = row;
  if (_path != "/") {
    if (row == 0) {
      _path = parentPath(_path);
      return;
    }
    idx = row - 1;
  }
  if (idx < 0 || idx >= (int)entries.size()) return;

  const FileEntryView& sel = entries[idx];
  if (sel.isDir) {
    _path = joinPath(_path, sel.name);
  } else {
    String filePath = joinPath(_path, sel.name);
    s.board->drawText(20, 510, "Selected: " + filePath, 0, 1);
  }
}

void WeatherApp::render(SystemServices& s) {
  titleBar(s,"Weather");
  s.board->drawText(20,110,"Weather by GPS location",0,1);
  s.board->drawText(20,140,"Cache file: /cache/weather/current.json",0,1);
  s.board->drawRect(20,180,880,260,0);

  if (!s.cache) {
    s.board->drawText(50,220,"Cache service unavailable.",5,2);
    return;
  }

  GpsFix fix = s.gps ? s.gps->fix() : GpsFix{};
  String status = "cache-only";
  if (fix.valid && s.net && s.net->status().wifi) {
    String cfgRaw = s.cache->readText("/config/weather.json", 2048);
    WeatherFetchConfig cfg = parseWeatherFetchConfig(cfgRaw);
    String url = buildWeatherUrl(cfg, fix);
    if (url.length()) {
      HTTPClient http;
      http.setTimeout(cfg.timeoutMs);
      if (http.begin(url)) {
        int code = http.GET();
        if (code > 0) {
          String body = http.getString();
          String summary = extractWeatherSummary(body);
          uint64_t epoch = fix.epoch ? fix.epoch : (uint64_t)(millis() / 1000);
          s.cache->writeText("/cache/weather/current.json", buildWeatherCacheJson(epoch, summary));
          status = "fetched";
        } else {
          status = "http get failed";
        }
        http.end();
      } else {
        status = "http begin failed";
      }
    } else {
      status = "invalid /config/weather.json";
    }
  }

  String raw = s.cache->readText("/cache/weather/current.json", 4096);
  WeatherCacheInfo info = parseWeatherCacheJson(raw);
  if (!info.valid) {
    s.board->drawText(50,220,"No valid weather cache found.",5,2);
    return;
  }

  uint64_t nowEpoch = s.gps ? s.gps->fix().epoch : 0;
  bool stale = isWeatherCacheStale(nowEpoch, info.fetchedEpoch, 1800);
  s.board->drawText(50,190, String("Fetch: ") + status, 0, 1);
  s.board->drawText(50,220, String("Summary: ") + info.summary, 0, 1);
  s.board->drawText(50,250, String("Fetched: ") + String((uint32_t)info.fetchedEpoch), 0, 1);
  s.board->drawText(50,280, String("State: ") + (stale ? "STALE" : "FRESH"), stale ? 5 : 0, 2);
}
void WebServerApp::render(SystemServices& s) { titleBar(s,"Web Server"); NetStatus ns=s.net->status(); s.board->drawText(20,110,String("Status: ")+(s.web->running()?"running":"stopped"),0,2); s.board->drawText(20,150,"IP: "+ns.ip.toString(),0,1); s.board->drawText(20,180,"Serving: /webroot",0,1); s.board->drawText(20,230,"Tap upper-left content area to toggle",5,1); }
void WebServerApp::handleTouch(SystemServices& s, const TouchEvent& ev) { if(ev.type==TouchType::Tap){ if(s.web->running()) s.web->stop(); else s.web->start(); } }
void GamesApp::render(SystemServices& s) { titleBar(s,"Games"); const char* g[]={"Chess","Go","Tic-Tac-Toe","Minesweeper"}; for(int i=0;i<4;i++){ int x=40+i*220; s.board->drawRect(x,140,180,140,0); s.board->drawText(x+20,200,g[i],0,2);} s.board->drawText(20,330,gamesAppNoticeText(),5,1); }
void SettingsApp::render(SystemServices& s) { titleBar(s,"Settings"); int y=110; s.board->drawText(20,y,settingsAppNoticeText(),5,2); y+=34; s.board->drawText(20,y,"Planned sections: Wi-Fi, GPS, LoRa, Display, Cache, Power, About",0,1); y+=26; s.board->drawText(20,y,"Diagnostics screens are scaffold-only in this build.",0,1); }
