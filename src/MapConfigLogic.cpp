#include "MapConfigLogic.h"
#include <ArduinoJson.h>

MapConfig sanitizeMapConfig(const MapConfig& in) {
  MapConfig c = in;
  if (!c.provider.length()) c.provider = "default";
  if (c.minZoom < 0) c.minZoom = 0;
  if (c.maxZoom > 22) c.maxZoom = 22;
  if (c.minZoom > c.maxZoom) c.minZoom = c.maxZoom;
  if (c.zoom < c.minZoom) c.zoom = c.minZoom;
  if (c.zoom > c.maxZoom) c.zoom = c.maxZoom;
  if (c.prefetchRadius < 0) c.prefetchRadius = 0;
  if (c.prefetchRadius > 5) c.prefetchRadius = 5;
  if (c.tileSizePx <= 0) c.tileSizePx = 256;
  return c;
}

MapConfig parseMapConfig(const String& json) {
  MapConfig cfg;
  JsonDocument d;
  if (deserializeJson(d, json) != DeserializationError::Ok) return cfg;
  if (d["provider"].is<const char*>()) cfg.provider = d["provider"].as<const char*>();
  cfg.zoom = d["zoom"] | cfg.zoom;
  cfg.minZoom = d["minZoom"] | cfg.minZoom;
  cfg.maxZoom = d["maxZoom"] | cfg.maxZoom;
  cfg.prefetchRadius = d["prefetchRadius"] | cfg.prefetchRadius;
  cfg.offlineOnly = d["offlineOnly"] | cfg.offlineOnly;
  cfg.tileSizePx = d["tileSizePx"] | cfg.tileSizePx;
  cfg.valid = true;
  return sanitizeMapConfig(cfg);
}
