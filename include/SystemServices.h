#pragma once
#include <Arduino.h>
#include <vector>
#include "BoardHAL.h"
#include "Types.h"

class GPSService;
class NetworkService;
class CacheService;
class RadioService;
class WebServerService;

class SystemServices {
public:
  BoardHAL* board = nullptr;
  GPSService* gps = nullptr;
  NetworkService* net = nullptr;
  CacheService* cache = nullptr;
  RadioService* radio = nullptr;
  WebServerService* web = nullptr;
  String activeAppId;
  bool requestHome = false;
  String requestOpenApp;
};
