#include "CacheApiLogic.h"

String buildCacheStatsJson(uint32_t mapHits, uint32_t mapMisses) {
  return String("{\"mapCacheHits\":") + String(mapHits) + ",\"mapCacheMisses\":" + String(mapMisses) + "}";
}
