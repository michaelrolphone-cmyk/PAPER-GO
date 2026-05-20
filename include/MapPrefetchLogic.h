#pragma once
#include <Arduino.h>

struct MapPrefetchRequest {
  double lat = 0;
  double lon = 0;
  int zoom = 12;
  int radius = 1;
  bool valid = false;
};

MapPrefetchRequest parseMapPrefetchRequest(const String& body);
String buildMapPrefetchJobLine(const String& jobId, const MapPrefetchRequest& req);
