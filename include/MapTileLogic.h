#pragma once

#include <Arduino.h>
#include "Services.h"

struct MapTileCoord {
  bool valid = false;
  uint8_t zoom = 0;
  int32_t x = 0;
  int32_t y = 0;
};

MapTileCoord mapTileFromFix(const GpsFix& fix, uint8_t zoom);
String mapTileLabel(const MapTileCoord& tile);
