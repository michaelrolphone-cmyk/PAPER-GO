#pragma once
#include <Arduino.h>

struct TouchPointSample {
  bool touching = false;
  bool twoPoint = false;
  int16_t x1 = 0;
  int16_t y1 = 0;
  int16_t x2 = 0;
  int16_t y2 = 0;
};

bool decodeGt911TouchPayload(const uint8_t* payload, size_t payloadLen, TouchPointSample& out);
void mapTouchToLandscape(uint16_t panelWidth, uint16_t panelHeight,
                         uint16_t touchMaxX, uint16_t touchMaxY,
                         int16_t srcX, int16_t srcY, int16_t& dstX, int16_t& dstY,
                         bool swapXY = false, bool mirrorX = false, bool mirrorY = false);
