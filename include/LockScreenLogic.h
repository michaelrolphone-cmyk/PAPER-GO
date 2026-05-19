#pragma once
#include <Arduino.h>
#include "Types.h"
#include "MapCacheLogic.h"

struct LockScreenPreviewInfo {
  String summary;
  CacheCoverageState coverage = CacheCoverageState::Uncached;
};

String lockScreenMapTilePath(const GpsFix& fix);
LockScreenPreviewInfo buildLockScreenPreviewInfo(const GpsFix& fix, bool hasTile);
