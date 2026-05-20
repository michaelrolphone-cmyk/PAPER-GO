#include "MapPrefetchLogic.h"
#include <ArduinoJson.h>

MapPrefetchRequest parseMapPrefetchRequest(const String& body) {
  MapPrefetchRequest r;
  JsonDocument d;
  if (deserializeJson(d, body) != DeserializationError::Ok) return r;
  if (!d["lat"].is<double>() || !d["lon"].is<double>()) return r;
  r.lat = d["lat"].as<double>();
  r.lon = d["lon"].as<double>();
  r.zoom = d["zoom"] | 12;
  r.radius = d["radius"] | 1;
  if (r.zoom < 0) r.zoom = 0;
  if (r.zoom > 22) r.zoom = 22;
  if (r.radius < 0) r.radius = 0;
  if (r.radius > 8) r.radius = 8;
  r.valid = true;
  return r;
}

String buildMapPrefetchJobLine(const String& jobId, const MapPrefetchRequest& req) {
  return jobId + "," + String(req.lat, 6) + "," + String(req.lon, 6) + "," + String(req.zoom) + "," + String(req.radius);
}
