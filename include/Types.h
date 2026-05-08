#pragma once
#include <Arduino.h>

enum class TouchType { None, Tap, LongPress, SwipeLeft, SwipeRight, SwipeUp, SwipeDown, Drag };
struct TouchEvent { TouchType type = TouchType::None; int16_t x = 0; int16_t y = 0; int16_t dx = 0; int16_t dy = 0; bool active = false; };

struct GpsFix {
  bool valid = false;
  double lat = 0;
  double lon = 0;
  double altM = 0;
  double speedKmph = 0;
  double courseDeg = 0;
  double hdop = 99;
  uint8_t sats = 0;
  uint32_t ageMs = UINT32_MAX;
  uint64_t epoch = 0;
};

struct BatteryStatus { int percent = -1; bool charging = false; float voltage = 0; float currentMa = 0; };
struct NetStatus { bool wifi = false; String ssid; IPAddress ip; bool webServer = false; };
struct RadioSignal { String kind; String name; String address; int rssi = 0; int channel = 0; String protocol; String extra; uint32_t lastSeenMs = 0; };
