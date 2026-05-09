#pragma once

#include <stdint.h>

enum class CacheCoverageState {
  FullyCached,
  PartiallyCached,
  Uncached
};

CacheCoverageState deriveCacheCoverageState(uint16_t totalTiles, uint16_t cachedTiles);
const char* cacheCoverageLabel(CacheCoverageState state);
