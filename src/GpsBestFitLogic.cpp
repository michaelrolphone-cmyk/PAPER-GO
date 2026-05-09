#include "GpsBestFitLogic.h"

GpsFix computeBestFitFix(const std::vector<GpsFix>& fixes, double maxDeltaDeg) {
  GpsFix out;
  double latSum = 0, lonSum = 0, hdopSum = 0;
  int used = 0;

  for (const auto& f : fixes) {
    if (!f.valid) continue;
    if (used == 0) {
      latSum += f.lat; lonSum += f.lon; hdopSum += f.hdop; used++; continue;
    }
    double meanLat = latSum / used;
    double meanLon = lonSum / used;
    if (abs(f.lat - meanLat) > maxDeltaDeg || abs(f.lon - meanLon) > maxDeltaDeg) continue;
    latSum += f.lat;
    lonSum += f.lon;
    hdopSum += f.hdop;
    used++;
  }

  if (used == 0) return out;
  out.valid = true;
  out.lat = latSum / used;
  out.lon = lonSum / used;
  out.hdop = hdopSum / used;
  return out;
}
