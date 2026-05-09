#pragma once
#include "Types.h"

class TouchClassifier {
public:
  void reset();
  TouchEvent update(bool touching, int16_t x, int16_t y, uint32_t nowMs);
private:
  bool _active = false;
  int16_t _startX = 0;
  int16_t _startY = 0;
  int16_t _lastX = 0;
  int16_t _lastY = 0;
  uint32_t _startMs = 0;
  bool _longPressEmitted = false;
};
