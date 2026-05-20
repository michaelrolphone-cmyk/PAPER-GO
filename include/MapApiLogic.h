#pragma once
#include <Arduino.h>
#include "MapTileLogic.h"
#include "MapCacheLogic.h"
#include "MapConfigLogic.h"

String buildMapsStatusJson(const MapConfig& cfg, const MapTileCoord& center, CacheCoverageState coverage, uint32_t hits, uint32_t misses, bool usingDgps, const String& quality);
String buildMapsPrefetchResultJson(bool ok, const String& jobId);
String buildMapsCacheStatsJson(const String& provider, uint32_t files, uint64_t totalBytes);
String buildMapsCacheClearJson(bool ok, const String& provider, uint32_t filesRemoved);
