#pragma once
#include <Arduino.h>
#include <SD.h>
#include <WiFi.h>
#include <Wire.h>
#include "Types.h"
#include "TouchLogic.h"

class BoardHAL {
public:
  bool begin();
  bool beginSD();
  bool sdMounted() const { return _sdMounted; }
  void beginFrame();
  void endFrame(bool fullRefresh=false);
  void clear(uint8_t gray=15);
  void fillRect(int x,int y,int w,int h,uint8_t gray);
  void drawRect(int x,int y,int w,int h,uint8_t gray);
  void drawText(int x,int y,const String& text,uint8_t gray=0,uint8_t size=1);
  void drawLine(int x1,int y1,int x2,int y2,uint8_t gray=0);
  TouchEvent pollTouch();
  void setTouchSample(bool touching, int16_t x, int16_t y);
  BatteryStatus battery();
  void sleepSeconds(uint32_t seconds);
private:
  bool _sdMounted=false;
  TouchClassifier _touchClassifier;
  bool _touching=false;
  int16_t _touchX=0;
  int16_t _touchY=0;
};
