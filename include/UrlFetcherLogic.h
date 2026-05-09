#pragma once
#include <Arduino.h>

struct UrlFetcherConfig {
  bool valid = false;
  String url;
  uint16_t timeoutMs = 5000;
};

UrlFetcherConfig parseUrlFetcherConfig(const String& json);
String cachePathForUrl(const String& url);
