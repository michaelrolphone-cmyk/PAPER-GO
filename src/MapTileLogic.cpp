#include "MapTileLogic.h"
#include <math.h>

static double clampLat(double lat) {
  if (lat > 85.05112878) return 85.05112878;
  if (lat < -85.05112878) return -85.05112878;
  return lat;
}

MapTileCoord mapTileFromFix(const GpsFix& fix, uint8_t zoom) {
  MapTileCoord out;
  out.zoom = zoom;
  if (!fix.valid || zoom > 22) return out;

  const double lat = clampLat(fix.lat);
  const double lon = fix.lon;
  const double n = (double)(1UL << zoom);
  const double x = (lon + 180.0) / 360.0 * n;
  const double latRad = lat * M_PI / 180.0;
  const double y = (1.0 - log(tan(latRad) + (1.0 / cos(latRad))) / M_PI) / 2.0 * n;

  out.x = (int32_t)floor(x);
  out.y = (int32_t)floor(y);
  if (out.x < 0) out.x = 0;
  if (out.y < 0) out.y = 0;
  const int32_t maxTile = (int32_t)n - 1;
  if (out.x > maxTile) out.x = maxTile;
  if (out.y > maxTile) out.y = maxTile;
  out.valid = true;
  return out;
}

String mapTileLabel(const MapTileCoord& tile) {
  if (!tile.valid) return "n/a";
  return String(tile.zoom) + "/" + String(tile.x) + "/" + String(tile.y);
}
