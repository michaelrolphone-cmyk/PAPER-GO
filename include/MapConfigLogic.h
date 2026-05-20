#pragma once
#include <Arduino.h>

struct MapConfig {
  String provider = "default";
  int zoom = 12;
  int minZoom = 1;
  int maxZoom = 18;
  int prefetchRadius = 1;
  bool offlineOnly = true;
  int tileSizePx = 256;
  bool valid = false;
};

MapConfig parseMapConfig(const String& json);
MapConfig sanitizeMapConfig(const MapConfig& in);
