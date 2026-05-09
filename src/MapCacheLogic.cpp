#include "MapCacheLogic.h"

CacheCoverageState deriveCacheCoverageState(uint16_t totalTiles, uint16_t cachedTiles) {
  if (totalTiles == 0 || cachedTiles == 0) return CacheCoverageState::Uncached;
  if (cachedTiles >= totalTiles) return CacheCoverageState::FullyCached;
  return CacheCoverageState::PartiallyCached;
}

const char* cacheCoverageLabel(CacheCoverageState state) {
  switch (state) {
    case CacheCoverageState::FullyCached: return "fully cached";
    case CacheCoverageState::PartiallyCached: return "partially cached";
    default: return "uncached";
  }
}
