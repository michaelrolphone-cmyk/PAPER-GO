#pragma once
#include <Arduino.h>
#include <vector>
#include <deque>
#include <WebServer.h>
#include <TinyGPSPlus.h>
#include <RadioLib.h>
#include <NimBLEDevice.h>
#include "Types.h"
#include "BoardHAL.h"

class GPSService {
public:
  bool begin();
  void update();
  GpsFix fix() const { return _fix; }
  GpsFix bestFit() const { return _bestFit; }
  double computedHeadingDeg() const { return _heading; }
  bool headingReliable() const { return _headingReliable; }
private:
  HardwareSerial _serial{1};
  TinyGPSPlus _gps;
  GpsFix _fix;
  GpsFix _prev;
  double _heading = 0;
  bool _headingReliable = false;
  std::deque<GpsFix> _history;
  GpsFix _bestFit;
  void computeHeading();
  void updateBestFit();
};

class NetworkService {
public:
  bool begin();
  void update();
  bool connectSaved();
  bool connect(const String& ssid, const String& pass);
  std::vector<RadioSignal> scanWifi();
  NetStatus status() const;
private:
  NetStatus _status;
};

class CacheService {
public:
  bool begin();
  void ensureLayout();
  String mapTilePath(const String& provider, int z, int x, int y) const;
  bool hasFile(const String& path) const;
  bool writeText(const String& path, const String& text);
  String readText(const String& path, size_t maxLen=32768);
  void appendLog(const String& name, const String& line);
  void recordMapCacheLookup(bool hit);
  uint32_t mapCacheHitCount() const { return _mapCacheHits; }
  uint32_t mapCacheMissCount() const { return _mapCacheMisses; }
};

class RadioService {
public:
  bool begin();
  std::vector<RadioSignal> scanBLE(uint32_t ms=3000);
  std::vector<RadioSignal> scanLoRaWindow(uint32_t ms=2000);
private:
  SX1262* _lora = nullptr;
};

class WebServerService {
public:
  bool begin();
  void update();
  void start();
  void stop();
  bool running() const { return _running; }
  void attachContext(BoardHAL* board, GPSService* gps, NetworkService* net, CacheService* cache);
private:
  WebServer _server{80};
  bool _running=false;
  BoardHAL* _board = nullptr;
  GPSService* _gps = nullptr;
  NetworkService* _net = nullptr;
  CacheService* _cache = nullptr;
  void servePath(const String& path);
};
