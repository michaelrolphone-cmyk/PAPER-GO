#include "GpsTrackApiLogic.h"

String gpsTrackEntryPathForRemoval(const String& entryName) {
  if (!entryName.length()) return "";
  if (entryName.startsWith("/gps/tracks/")) return entryName;
  if (entryName.startsWith("/")) return String("/gps/tracks") + entryName;
  return String("/gps/tracks/") + entryName;
}

String gpsTrackEntryNameForResponse(const String& entryName) {
  if (entryName.startsWith("/gps/tracks/")) {
    return entryName.substring(String("/gps/tracks/").length());
  }
  if (entryName.startsWith("/")) return entryName.substring(1);
  return entryName;
}
