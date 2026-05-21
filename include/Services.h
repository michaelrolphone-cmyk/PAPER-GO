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
#include "DgpsLogic.h"

class GPSService {
public:
  bool begin();
  void update();
  GpsFix fix() const { return _fix; }
  GpsFix activeFix() const { return _usingDgps ? _correctedFix : _fix; }
  GpsFix correctedFix() const { return _correctedFix; }
  GpsFix bestFit() const { return _bestFit; }
  double computedHeadingDeg() const { return _heading; }
  bool headingReliable() const { return _headingReliable; }
  bool usingDgps() const { return _usingDgps; }
  DgpsQualityState dgpsQuality() const { return _dgpsQuality; }
  uint32_t dgpsAgeMs() const { return _dgpsAgeMs; }
  const RoverCorrectionState& dgpsState() const { return _dgpsState; }
  const DgpsPacketStats& dgpsPacketStats() const { return _dgpsPacketStats; }
private:
  void logFixIfNeeded();
  void refreshTrackLoggingConfig();
  HardwareSerial _serial{1};
  TinyGPSPlus _gps;
  GpsFix _fix;
  GpsFix _prev;
  double _heading = 0;
  bool _headingReliable = false;
  std::deque<GpsFix> _history;
  GpsFix _bestFit;
  GpsFix _lastLoggedFix;
  GpsFix _correctedFix;
  bool _usingDgps = false;
  DgpsQualityState _dgpsQuality = DgpsQualityState::NO_BASE;
  uint32_t _dgpsAgeMs = 0;
  RoverCorrectionState _dgpsState;
  DgpsPacketStats _dgpsPacketStats;
  bool _trackLoggingEnabled = false;
  uint32_t _nextTrackConfigRefreshMs = 0;
  void computeHeading();
  void updateBestFit();
};

class CacheService;

class NetworkService {
public:
  bool begin();
  void attachCache(CacheService* cache);
  void update();
  bool connectSaved();
  bool connect(const String& ssid, const String& pass);
  bool disconnect();
  bool forgetSaved();
  bool saveCredentials(const String& ssid, const String& pass);
  std::vector<RadioSignal> scanWifi();
  NetStatus status() const;
private:
  NetStatus _status;
  CacheService* _cache = nullptr;
  RadioService* _radio = nullptr;
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
  uint32_t lastMapCacheLookupMs() const { return _lastMapCacheLookupMs; }
private:
  uint32_t _mapCacheHits = 0;
  uint32_t _mapCacheMisses = 0;
  uint32_t _lastMapCacheLookupMs = 0;
};

class RadioService {
public:
  bool begin();
  bool bleReady() const { return _bleReady; }
  bool loraReady() const { return _loraReady; }
  std::vector<RadioSignal> scanBLE(uint32_t ms=3000);
  std::vector<RadioSignal> scanLoRaWindow(uint32_t ms=2000);
private:
  SX1262* _lora = nullptr;
  bool _bleReady = false;
  bool _loraReady = false;
};

class WebServerService {
public:
  bool begin();
  void update();
  void start();
  void stop();
  bool running() const { return _running; }
  void attachContext(BoardHAL* board, GPSService* gps, NetworkService* net, CacheService* cache, RadioService* radio);
private:
  WebServer _server{80};
  bool _running=false;
  BoardHAL* _board = nullptr;
  GPSService* _gps = nullptr;
  NetworkService* _net = nullptr;
  CacheService* _cache = nullptr;
  RadioService* _radio = nullptr;
  void servePath(const String& path);
};
