#pragma once
#include <Arduino.h>
#include <SD.h>
#include <WiFi.h>
#include <Wire.h>
#include "Types.h"
#include "TouchLogic.h"
#include "LowlightLogic.h"

class BoardHAL {
public:
  bool begin();
  bool beginSD();
  bool sdMounted() const { return _sdMounted; }
  bool rtcAvailable() const { return _rtcAvailable; }
  void beginFrame();
  void endFrame(bool fullRefresh=false);
  void clear(uint8_t gray=15);
  void fillRect(int x,int y,int w,int h,uint8_t gray);
  void drawRect(int x,int y,int w,int h,uint8_t gray);
  void drawText(int x,int y,const String& text,uint8_t gray=0,uint8_t size=1);
  void drawLine(int x1,int y1,int x2,int y2,uint8_t gray=0);
  TouchEvent pollTouch();
  bool pollHomeButtonPressed();
  bool pollPowerButtonPressed();
  void setTouchSample(bool touching, int16_t x, int16_t y);
  void setTouchSampleTwoPoint(bool touching, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
  void setHomeButtonSample(bool pressed);
  void setPowerButtonSample(bool pressed);
  BatteryStatus battery();
  void sleepSeconds(uint32_t seconds);
  void setLowlightMode(bool enabled);
  void toggleBacklight();
  bool backlightOn() const { return shouldBacklightBeOn(_lowlight); }
private:
  bool _sdMounted=false;
  bool _rtcAvailable=false;
  TouchClassifier _touchClassifier;
  bool _touching=false;
  int16_t _touchX=0;
  int16_t _touchY=0;
  bool _touchTwoPoint = false;
  int16_t _touchX2=0;
  int16_t _touchY2=0;
  bool _homeButtonOverride=false;
  bool _homeButtonPressed=false;
  bool _homeButtonLast=false;
  bool _powerButtonOverride=false;
  bool _powerButtonPressed=false;
  bool _powerButtonLast=false;
  LowlightState _lowlight;
  uint8_t _touchAddr = 0;
  void applyBacklightState();
};
