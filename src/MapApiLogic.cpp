#include "MapApiLogic.h"

String buildMapsStatusJson(const MapConfig& cfg, const MapTileCoord& center, CacheCoverageState coverage, uint32_t hits, uint32_t misses, bool usingDgps, const String& quality) {
  String j="{";
  j += "\"provider\":\"" + cfg.provider + "\"";
  j += ",\"zoom\":" + String(cfg.zoom);
  j += ",\"offlineOnly\":" + String(cfg.offlineOnly?"true":"false");
  j += ",\"coverage\":\"" + String(cacheCoverageLabel(coverage)) + "\"";
  j += ",\"centerTile\":\"" + mapTileLabel(center) + "\"";
  j += ",\"hits\":" + String(hits) + ",\"misses\":" + String(misses);
  j += ",\"dgpsMode\":\"" + String(usingDgps ? "DGPS" : "GPS") + "\"";
  j += ",\"dgpsQuality\":\"" + quality + "\"";
  j += "}";
  return j;
}
String buildMapsPrefetchResultJson(bool ok, const String& jobId) {
  return String("{\"ok\":") + (ok?"true":"false") + ",\"jobId\":\"" + jobId + "\"}";
}
String buildMapsCacheStatsJson(const String& provider, uint32_t files, uint64_t totalBytes) {
  return String("{\"provider\":\"") + provider + "\",\"files\":" + String(files) + ",\"bytes\":" + String((uint32_t)totalBytes) + "}";
}

String buildMapsCacheClearJson(bool ok, const String& provider, uint32_t filesRemoved) {
  return String("{\"ok\":") + (ok ? "true" : "false") +
         ",\"provider\":\"" + provider + "\",\"filesRemoved\":" + String(filesRemoved) + "}";
}
