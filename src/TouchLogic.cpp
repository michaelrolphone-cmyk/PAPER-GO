#include "TouchLogic.h"

static const int16_t TAP_MOVE_THRESHOLD = 12;
static const int16_t SWIPE_THRESHOLD = 50;
static const uint32_t LONG_PRESS_MS = 650;

void TouchClassifier::reset() {
  _active = false;
  _longPressEmitted = false;
}

TouchEvent TouchClassifier::update(bool touching, int16_t x, int16_t y, uint32_t nowMs) {
  TouchEvent ev{};

  if (touching && !_active) {
    _active = true;
    _startX = _lastX = x;
    _startY = _lastY = y;
    _startMs = nowMs;
    _longPressEmitted = false;
    ev.active = true;
    return ev;
  }

  if (touching && _active) {
    int16_t dx = x - _lastX;
    int16_t dy = y - _lastY;
    _lastX = x;
    _lastY = y;

    ev.active = true;
    ev.x = x;
    ev.y = y;
    ev.dx = dx;
    ev.dy = dy;

    int16_t totalDx = x - _startX;
    int16_t totalDy = y - _startY;
    if (!_longPressEmitted && abs(totalDx) <= TAP_MOVE_THRESHOLD && abs(totalDy) <= TAP_MOVE_THRESHOLD && (nowMs - _startMs) >= LONG_PRESS_MS) {
      _longPressEmitted = true;
      ev.type = TouchType::LongPress;
      return ev;
    }

    if (abs(dx) > 0 || abs(dy) > 0) {
      ev.type = TouchType::Drag;
      return ev;
    }

    return ev;
  }

  if (!touching && _active) {
    _active = false;
    int16_t totalDx = _lastX - _startX;
    int16_t totalDy = _lastY - _startY;

    ev.x = _lastX;
    ev.y = _lastY;
    ev.dx = totalDx;
    ev.dy = totalDy;

    if (_longPressEmitted) return ev;

    if (abs(totalDx) <= TAP_MOVE_THRESHOLD && abs(totalDy) <= TAP_MOVE_THRESHOLD) {
      ev.type = TouchType::Tap;
      return ev;
    }

    if (abs(totalDx) >= abs(totalDy) && abs(totalDx) >= SWIPE_THRESHOLD) {
      ev.type = totalDx > 0 ? TouchType::SwipeRight : TouchType::SwipeLeft;
      return ev;
    }
    if (abs(totalDy) >= SWIPE_THRESHOLD) {
      ev.type = totalDy > 0 ? TouchType::SwipeDown : TouchType::SwipeUp;
      return ev;
    }
  }

  return ev;
}
