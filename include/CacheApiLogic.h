#pragma once
#include <Arduino.h>

String buildCacheStatsJson(uint32_t mapHits, uint32_t mapMisses);
