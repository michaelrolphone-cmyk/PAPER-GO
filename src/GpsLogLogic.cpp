#include "GpsLogLogic.h"
#include <math.h>

static double toRad(double deg) { return deg * PI / 180.0; }

static double haversineMeters(double lat1, double lon1, double lat2, double lon2) {
  constexpr double kEarthRadiusMeters = 6371000.0;
  const double dLat = toRad(lat2 - lat1);
  const double dLon = toRad(lon2 - lon1);
  const double a = sin(dLat / 2.0) * sin(dLat / 2.0) +
                   cos(toRad(lat1)) * cos(toRad(lat2)) *
                   sin(dLon / 2.0) * sin(dLon / 2.0);
  const double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
  return kEarthRadiusMeters * c;
}


static bool isLeapYear(int year) {
  return ((year % 4) == 0 && (year % 100) != 0) || ((year % 400) == 0);
}

uint64_t buildGpsEpochSeconds(int year, int month, int day, int hour, int minute, int second) {
  if (year < 1970 || month < 1 || month > 12 || day < 1 || day > 31 || hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 60) {
    return 0;
  }
  static const int daysBeforeMonth[] = {0,31,59,90,120,151,181,212,243,273,304,334};
  int maxDay = 31;
  if (month == 2) maxDay = isLeapYear(year) ? 29 : 28;
  else if (month == 4 || month == 6 || month == 9 || month == 11) maxDay = 30;
  if (day > maxDay) return 0;

  uint64_t days = 0;
  for (int y = 1970; y < year; ++y) days += isLeapYear(y) ? 366 : 365;
  days += daysBeforeMonth[month - 1];
  if (month > 2 && isLeapYear(year)) days += 1;
  days += static_cast<uint64_t>(day - 1);

  return days * 86400ULL + static_cast<uint64_t>(hour) * 3600ULL + static_cast<uint64_t>(minute) * 60ULL + static_cast<uint64_t>(second);
}

bool shouldLogGpsFix(const GpsFix& fix, const GpsFix& previousLoggedFix, uint32_t minMoveMeters) {
  if (!fix.valid) return false;
  if (!previousLoggedFix.valid) return true;
  return haversineMeters(fix.lat, fix.lon, previousLoggedFix.lat, previousLoggedFix.lon) >= minMoveMeters;
}

String buildGpsTrackCsvLine(const GpsFix& fix, double headingDeg, bool headingReliable) {
  String line = String(fix.epoch);
  line += "," + String(fix.lat, 6);
  line += "," + String(fix.lon, 6);
  line += "," + String(fix.altM, 1);
  line += "," + String(fix.speedKmph, 1);
  line += "," + String(headingReliable ? headingDeg : -1.0, 1);
  line += "," + String(fix.hdop, 2);
  line += "," + String(fix.sats);
  return line;
}
