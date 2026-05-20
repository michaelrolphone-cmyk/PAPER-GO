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
#include "RadioApiLogic.h"
#include "GamesUiLogic.h"
#include "SettingsLogic.h"
#include "WifiConfigLogic.h"
#include "PowerManagementLogic.h"
#include "DgpsApiLogic.h"
#include "CommonControlsLogic.h"

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
  const int cols=5, cellW=180, cellH=190, startX=30, startY=90;
  for(size_t i=start;i<end;i++) {
    size_t local = i - start;
    int c=local%cols, r=local/cols;
    int x = startX + c*cellW;
    int y = startY + r*cellH;
    s.board->drawRect(x,y,160,160,0);
    String appId = _orderedIds[i];
    bool supportsOffline = appSupportsOfflineMode(appId);
    bool wifiConnected = s.net && s.net->status().wifi;
    bool unavailable = springboardOnlineRequiredUnavailable(supportsOffline, wifiConnected);
    uint8_t titleColor = unavailable ? 5 : 0;
    s.board->drawText(x+18,y+38,appDisplayNameById(appId),titleColor,2);
    s.board->drawText(x+18,y+130,appId,7,1);
    if (!supportsOffline) {
      s.board->fillRect(x+74, y+8, 80, 20, 13);
      s.board->drawRect(x+74, y+8, 80, 20, 0);
      s.board->drawText(x+80, y+12, unavailable ? "OFFLINE" : "ONLINE", 0, 1);
    }
  }
  s.board->drawText(20, 500, String("Page ") + String(_page + 1) + "/" + String(max((size_t)1, pages)) + " swipe left/right (wrap)", 0, 1);
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
  if (ev.type == TouchType::SwipeLeft) {
    springboardAdvancePage(_page, pages);
    return;
  }
  if (ev.type == TouchType::SwipeRight) {
    springboardRetreatPage(_page, pages);
    return;
  }

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
      bool moved = springboardMoveAppToFront(_orderedIds, static_cast<size_t>(_selectedIndex));
      if (moved && s.cache) {
        s.cache->writeText("/config/apps.json", serializeOrderedAppIds(_orderedIds));
      }
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
  GpsFix f=s.gps->activeFix();
  GpsFix raw=s.gps->fix();
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
  GpsFix f=s.gps->activeFix();
  GpsFix raw=s.gps->fix();
  s.board->drawRect(20,95,620,390,0);
  const int gridX = 40, gridY = 120, cellW = 180, cellH = 110;
  MapTileCoord center = mapTileFromFix(f.valid ? f : raw, 12);
  uint16_t totalTiles = 0;
  uint16_t cachedTiles = 0;
  for (int gy = 0; gy < 3; ++gy) {
    for (int gx = 0; gx < 3; ++gx) {
      int dx = gx - 1;
      int dy = gy - 1;
      int x0 = gridX + gx * (cellW + 8);
      int y0 = gridY + gy * (cellH + 8);
      int32_t tx = center.x + dx;
      int32_t ty = center.y + dy;
      bool tileValid = center.valid && tx >= 0 && ty >= 0;
      bool hit = false;
      if (tileValid && s.cache) {
        ++totalTiles;
        hit = s.cache->hasFile(s.cache->mapTilePath("default", center.zoom, (uint32_t)tx, (uint32_t)ty));
        if (hit) ++cachedTiles;
        s.cache->recordMapCacheLookup(hit);
      }
      s.board->fillRect(x0, y0, cellW, cellH, hit ? 12 : 14);
      s.board->drawRect(x0, y0, cellW, cellH, 0);
      s.board->drawText(x0 + 8, y0 + 8, tileValid ? mapTileLabel(MapTileCoord{true, center.zoom, tx, ty}) : String("n/a"), 0, 1);
      s.board->drawText(x0 + 8, y0 + 34, hit ? "cached" : "missing", hit ? 0 : 5, 1);
      if (dx == 0 && dy == 0) {
        s.board->drawLine(x0 + cellW / 2 - 10, y0 + cellH / 2, x0 + cellW / 2 + 10, y0 + cellH / 2, 0);
        s.board->drawLine(x0 + cellW / 2, y0 + cellH / 2 - 10, x0 + cellW / 2, y0 + cellH / 2 + 10, 0);
      }
    }
  }
  int x=670, y=105;
  s.board->drawText(x,y,"Mode",0,2); y+=35;
  s.board->drawText(x,y,s.gps->usingDgps()?"DGPS":"GPS",s.gps->usingDgps()?0:5,1); y+=25;
  s.board->drawText(x,y,"Quality: "+dgpsQualityStateLabel(s.gps->dgpsQuality()),0,1); y+=25;
  s.board->drawText(x,y,"Age: "+String(s.gps->dgpsAgeMs()/1000.0,1)+"s",0,1); y+=25;
  s.board->drawText(x,y,"Fix",0,2); y+=35;
  s.board->drawText(x,y,f.valid?"valid":"no fix",f.valid?0:5,1); y+=25;
  GpsFix bf = s.gps->bestFit();
  s.board->drawText(x,y,"Raw: "+String(raw.lat,6)+","+String(raw.lon,6),0,1); y+=22;
  s.board->drawText(x,y,"Active: "+String(f.lat,6)+","+String(f.lon,6),0,1); y+=22;
  s.board->drawText(x,y,"BestFit: "+(bf.valid?String(bf.lat,5)+","+String(bf.lon,5):String("n/a")),0,1); y+=22;
  s.board->drawText(x,y,"Sats: "+String(f.sats),0,1); y+=22;
  s.board->drawText(x,y,"HDOP: "+String(f.hdop,1),0,1); y+=22;
  if(s.gps->headingReliable()) s.board->drawText(x,y,"Heading: "+String(s.gps->computedHeadingDeg(),0),0,1);
  else s.board->drawText(x,y,"Heading: moving only",5,1);
  y+=35;
  center = mapTileFromFix(f.valid ? f : bf, 12);
  s.board->drawText(x,y,"Tile: " + mapTileLabel(center),0,1); y+=22;
  CacheCoverageState coverage = deriveCacheCoverageState(totalTiles, cachedTiles);
  s.board->drawText(x,y,"Tiles: "+String(cacheCoverageLabel(coverage))+" ("+String(cachedTiles)+"/"+String(totalTiles)+")",0,1); y+=22;
  if (s.cache) s.board->drawText(x,y,"Cache H/M: "+String(s.cache->mapCacheHitCount())+"/"+String(s.cache->mapCacheMissCount()),0,1);
  else s.board->drawText(x,y,"Cache service unavailable",5,1);
}

void RadioScannerApp::onStart(SystemServices& s) {
  _signals.clear();
  RadioControlConfig cfg;
  if (s.cache) {
    cfg = parseRadioControlConfig(s.cache->readText("/config/radio.json", 1024));
  }
  if (cfg.wifiEnabled && s.net) {
    auto wifi=s.net->scanWifi(); _signals.insert(_signals.end(), wifi.begin(), wifi.end());
  }
  if (cfg.bleEnabled && s.radio) {
    auto ble=s.radio->scanBLE(cfg.bleScanMs); _signals.insert(_signals.end(), ble.begin(), ble.end());
  }
  if (cfg.loraEnabled && s.radio) {
    auto lora=s.radio->scanLoRaWindow(cfg.loraScanMs); _signals.insert(_signals.end(), lora.begin(), lora.end());
  }
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

void FileExplorerApp::onStart(SystemServices& s) {
  (void)s;
  _path = "/";
  _page = 0;
  _sortDesc = false;
}

void FileExplorerApp::render(SystemServices& s) {
  titleBar(s,"Files: "+_path); int y=105;
  if (!s.board->sdMounted()) { s.board->drawText(20,y,"SD not mounted",5,1); return; }
  File root=SD.open(_path); if(!root){s.board->drawText(20,y,"Cannot open path",5,1); return;}

  std::vector<FileEntryView> entries;
  File file=root.openNextFile();
  while(file){
    FileEntryView e;
    e.name = fileBaseName(String(file.name()));
    e.isDir = file.isDirectory();
    e.size = file.size();
    entries.push_back(e);
    file = root.openNextFile();
  }
  root.close();

  sortFileEntries(entries);
  if (_sortDesc) std::reverse(entries.begin(), entries.end());
  const int pageSize = 17;
  int totalRows = (int)entries.size() + (_path != "/" ? 1 : 0);
  int maxPage = totalRows > 0 ? (totalRows - 1) / pageSize : 0;
  if (_page > maxPage) _page = maxPage;
  int start = _page * pageSize;
  int end = start + pageSize;

  if (_path != "/") {
    if (start == 0) {
      s.board->drawText(20, y, "[D] ..", 0, 1);
      y += 22;
    }
  }
  int rowIndex = _path != "/" ? 1 : 0;
  for (auto& e : entries) {
    if (rowIndex < start) { rowIndex++; continue; }
    if (rowIndex >= end) break;
    if (y >= 498) break;
    s.board->drawText(20, y, formatFileEntry(e), 0, 1);
    y += 22;
    rowIndex++;
  }
  s.board->drawText(20, 510, "Tap: open  Top: sort  Bottom: next page", 0, 1);
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
    e.name = fileBaseName(String(file.name()));
    e.isDir = file.isDirectory();
    e.size = file.size();
    entries.push_back(e);
    file = root.openNextFile();
  }
  root.close();
  sortFileEntries(entries);
  if (_sortDesc) std::reverse(entries.begin(), entries.end());
  const int pageSize = 17;
  int totalRows = (int)entries.size() + (_path != "/" ? 1 : 0);
  int maxPage = totalRows > 0 ? (totalRows - 1) / pageSize : 0;

  if (ev.y < (BoardConfig::STATUS_BAR_H + 60)) { _sortDesc = !_sortDesc; return; }
  if (ev.y > 500) { if (_page < maxPage) _page++; else _page = 0; return; }
  int row = (ev.y - 105) / 22;
  row += _page * pageSize;
  int idx = row;
  if (_path != "/") {
    if (row == 0) {
      _path = parentPath(_path);
      _page = 0;
      return;
    }
    idx = row - 1;
  }
  if (idx < 0 || idx >= (int)entries.size()) return;

  const FileEntryView& sel = entries[idx];
  if (sel.isDir) {
    _path = joinPath(_path, sel.name);
    _page = 0;
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
void GamesApp::onStart(SystemServices& s) {
  (void)s;
  _screen = GameScreen::Menu;
  _ttt.reset();
  _mines.begin(8, 8, 10);
  _chess.reset();
  _go.begin(9);
  _chessSelected = -1;
}

void GamesApp::render(SystemServices& s) {
  titleBar(s,"Games");
  if (_screen == GameScreen::Menu) {
    const char* g[]={"Chess","Go","Tic-Tac-Toe","Minesweeper"};
    for(int i=0;i<4;i++){ int x=40+i*220; s.board->drawRect(x,140,180,140,0); s.board->drawText(x+20,200,g[i],0,2);} 
    s.board->drawText(20,330,"Tap a game tile to play.",0,1);
    return;
  }
  if (_screen == GameScreen::TicTacToe) {
    s.board->drawText(20,100,"Tic-Tac-Toe",0,2);
    for (int i=0;i<9;i++) {
      int x=40 + (i%3)*120, y=150 + (i/3)*120;
      s.board->drawRect(x,y,100,100,0);
      auto c=_ttt.cellAt(i);
      if (c==TicTacToeGame::Cell::X) s.board->drawText(x+38,y+40,"X",0,2);
      if (c==TicTacToeGame::Cell::O) s.board->drawText(x+38,y+40,"O",0,2);
    }
    s.board->drawText(420,160, String("Turn: ") + (_ttt.currentPlayer()==TicTacToeGame::Cell::X?"X":"O"),0,1);
    s.board->drawText(420,190, String("State: ") + ticTacToeStateLabel(_ttt.state()),0,1);
    s.board->drawText(420,220,"Tap top bar to menu",5,1);
    return;
  }
  if (_screen == GameScreen::Minesweeper) {
    s.board->drawText(20,100,"Minesweeper",0,2);
    for (int y=0;y<8;y++) for (int x=0;x<8;x++) {
      int px=40+x*55, py=140+y*55;
      s.board->drawRect(px,py,50,50,0);
      if (_mines.isFlagged(x,y)) s.board->drawText(px+16,py+18,"F",0,1);
      if (_mines.isRevealed(x,y)) {
        uint8_t n=_mines.adjacent(x,y);
        s.board->drawText(px+16,py+18,n?String(n):".",0,1);
      }
    }
    s.board->drawText(520,150,"Tap=reveal",0,1);
    s.board->drawText(520,176,"Long=flag",0,1);
    s.board->drawText(520,202, String("State: ") + minesweeperStateLabel(_mines.state()),0,1);
    return;
  }
  if (_screen == GameScreen::Chess) {
    s.board->drawText(20,100,"Chess",0,2);
    for (int i=0;i<64;i++) {
      int x=40+(i%8)*55,y=130+(i/8)*55;
      s.board->drawRect(x,y,50,50,0);
      auto sq=_chess.at(i);
      const char* p="";
      if (sq.piece==ChessGame::Piece::Pawn) p="P"; else if (sq.piece==ChessGame::Piece::Knight) p="N"; else if (sq.piece==ChessGame::Piece::Bishop) p="B"; else if (sq.piece==ChessGame::Piece::Rook) p="R"; else if (sq.piece==ChessGame::Piece::Queen) p="Q"; else if (sq.piece==ChessGame::Piece::King) p="K";
      if (sq.piece!=ChessGame::Piece::Empty) s.board->drawText(x+14,y+18,p,sq.color==ChessGame::Color::Black?7:0,1);
    }
    s.board->drawText(520,140,_chessSelected>=0?"Select target":"Select piece",0,1);
    return;
  }
  if (_screen == GameScreen::Go) {
    s.board->drawText(20,100,"Go 9x9",0,2);
    for(int y=0;y<9;y++) for(int x=0;x<9;x++) {
      int px=60+x*50, py=150+y*50;
      s.board->drawRect(px,py,40,40,0);
      auto st=_go.at(x,y);
      if(st==GoGame::Stone::Black) s.board->fillRect(px+10,py+10,20,20,0);
      else if(st==GoGame::Stone::White){ s.board->drawRect(px+10,py+10,20,20,0); s.board->fillRect(px+11,py+11,18,18,15);}    
    }
    s.board->drawText(540,160,"Tap board",0,1);
    s.board->drawText(540,185,"Long=pass",0,1);
  }
}

void GamesApp::handleTouch(SystemServices& s, const TouchEvent& ev) {
  (void)s;
  if (ev.type != TouchType::Tap && ev.type != TouchType::LongPress) return;
  if (ev.y < 90) { _screen = GameScreen::Menu; return; }
  if (_screen == GameScreen::Menu && ev.type == TouchType::Tap) {
    GamesMenuSelection sel = gamesMenuSelectionAt(ev.x, ev.y);
    if (sel == GamesMenuSelection::Chess) _screen = GameScreen::Chess;
    else if (sel == GamesMenuSelection::Go) _screen = GameScreen::Go;
    else if (sel == GamesMenuSelection::TicTacToe) _screen = GameScreen::TicTacToe;
    else if (sel == GamesMenuSelection::Minesweeper) _screen = GameScreen::Minesweeper;
    return;
  }
  if (_screen == GameScreen::TicTacToe && ev.type == TouchType::Tap) {
    int8_t cell = ticTacToeCellAt(ev.x, ev.y);
    if (cell >= 0) _ttt.playMove(static_cast<uint8_t>(cell));
    return;
  }
  if (_screen == GameScreen::Minesweeper) {
    uint8_t cx = 0, cy = 0;
    if (minesweeperCellAt(ev.x, ev.y, cx, cy)) {
      if (ev.type==TouchType::Tap) _mines.reveal(cx,cy); else _mines.toggleFlag(cx,cy);
    }
    return;
  }
  if (_screen == GameScreen::Chess && ev.type == TouchType::Tap) {
    uint8_t idx = 0;
    if (chessCellAt(ev.x, ev.y, idx)) {
      if (_chessSelected < 0) _chessSelected = idx;
      else { _chess.playMove(static_cast<uint8_t>(_chessSelected), idx); _chessSelected = -1; }
    }
    return;
  }
  if (_screen == GameScreen::Go) {
    if (ev.type==TouchType::LongPress) { _go.pass(); return; }
    uint8_t cx = 0, cy = 0;
    if (goCellAt(ev.x, ev.y, cx, cy)) _go.place(cx,cy);
  }
}
void SettingsApp::onStart(SystemServices& s) {
  _state.selectedRow = 0;
  loadFromConfig(s);
}

void SettingsApp::loadFromConfig(SystemServices& s) {
  _state = SettingsViewState{};
  if (!s.cache) return;
  WifiConfig wifi = parseWifiConfig(s.cache->readText("/config/wifi.json", 1024));
  if (wifi.valid) {
    _state.hasWifiConfig = true;
    _state.ssid = wifi.ssid;
    _state.password = wifi.password;
  }
  PowerConfig p = parsePowerConfig(s.cache->readText("/config/power.json", 1024));
  if (p.valid) {
    _state.hasPowerConfig = true;
    _state.power = p.policy;
  }
}

void SettingsApp::savePowerConfig(SystemServices& s) {
  if (!s.cache) return;
  s.cache->writeText("/config/power.json", buildPowerConfigJson(_state.power));
}

void SettingsApp::render(SystemServices& s) {
  titleBar(s,"Settings");
  int y = 104;
  s.board->drawText(20, y, "Tap row to select. Tap selected row again (left/right) to edit.", 0, 1); y += 28;
  s.board->drawText(20, y, String(_state.hasWifiConfig ? "Wi-Fi: loaded" : "Wi-Fi: missing /config/wifi.json"), _state.hasWifiConfig ? 0 : 5, 1); y += 24;
  drawCommonControlRow(*s.board, 20, y, "SSID", _state.ssid.length() ? _state.ssid : String("(none)"), _state.selectedRow == 0); y += 24;
  String masked = _state.password.length() ? String("********") : String("(empty)");
  drawCommonControlRow(*s.board, 20, y, "Password", masked, _state.selectedRow == 1); y += 24;
  drawCommonControlRow(*s.board, 20, y, "Lock timeout (ms)", String(_state.power.lockTimeoutMs), _state.selectedRow == 2); y += 24;
  drawCommonControlRow(*s.board, 20, y, "Deep sleep timeout (ms)", String(_state.power.deepSleepTimeoutMs), _state.selectedRow == 3); y += 24;
  drawCommonControlRow(*s.board, 20, y, "Allow deep sleep", commonControlBoolLabel(_state.power.allowDeepSleep), _state.selectedRow == 4); y += 24;
  drawCommonControlRow(*s.board, 20, y, "Deep sleep duration (s)", String(_state.power.deepSleepDurationSec), _state.selectedRow == 5); y += 24;
  drawCommonControlRow(*s.board, 20, y, "Allow Wi-Fi in lock", commonControlBoolLabel(_state.power.allowWifiInLockScreen), _state.selectedRow == 6); y += 28;
  s.board->drawText(20, y, "Changes to power settings are saved to /config/power.json", 0, 1);
}

void SettingsApp::handleTouch(SystemServices& s, const TouchEvent& ev) {
  if (ev.type != TouchType::Tap) return;
  const int tappedRow = settingsRowFromTapY(ev.y, 156, 24, 7);
  bool editSelected = settingsTapShouldEditSelectedRow(_state.selectedRow, tappedRow);
  if (tappedRow >= 0) _state.selectedRow = tappedRow;
  if (!editSelected) return;

  bool changed = false;
  bool increment = ev.x > (BoardConfig::SCREEN_W / 2);
  if (_state.selectedRow == 2) { _state.power.lockTimeoutMs = cycleLockTimeoutMs(_state.power.lockTimeoutMs, increment); changed = true; }
  else if (_state.selectedRow == 3) { _state.power.deepSleepTimeoutMs = cycleDeepSleepTimeoutMs(_state.power.deepSleepTimeoutMs, increment); changed = true; }
  else if (_state.selectedRow == 4) { _state.power.allowDeepSleep = !_state.power.allowDeepSleep; changed = true; }
  else if (_state.selectedRow == 5) { _state.power.deepSleepDurationSec = cycleDeepSleepDurationSec(_state.power.deepSleepDurationSec, increment); changed = true; }
  else if (_state.selectedRow == 6) { _state.power.allowWifiInLockScreen = !_state.power.allowWifiInLockScreen; changed = true; }

  if (_state.power.deepSleepTimeoutMs < _state.power.lockTimeoutMs) _state.power.deepSleepTimeoutMs = _state.power.lockTimeoutMs;
  if (changed) savePowerConfig(s);
}
