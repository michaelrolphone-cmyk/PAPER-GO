#include "LockScreenLogic.h"

static int lonToTileX(double lon, int z) {
  double n = (1 << z);
  return (int)((lon + 180.0) / 360.0 * n);
}
static int latToTileY(double lat, int z) {
  const double pi = 3.14159265358979323846;
  double latRad = lat * pi / 180.0;
  double n = (1 << z);
  return (int)((1.0 - log(tan(latRad) + 1.0 / cos(latRad)) / pi) / 2.0 * n);
}

String lockScreenMapTilePath(const GpsFix& fix) {
  if (!fix.valid) return "";
  const int z = 12;
  int x = lonToTileX(fix.lon, z);
  int y = latToTileY(fix.lat, z);
  return String("/cache/maps/default/") + String(z) + "/" + String(x) + "/" + String(y) + ".tile";
}

LockScreenPreviewInfo buildLockScreenPreviewInfo(const GpsFix& fix, bool hasTile) {
  LockScreenPreviewInfo info;
  info.coverage = deriveCacheCoverageState(1, hasTile ? 1 : 0);
  if (!fix.valid) info.summary = "Location source: last known";
  else info.summary = String("Location: ") + String(fix.lat, 4) + ", " + String(fix.lon, 4);
  return info;
}
