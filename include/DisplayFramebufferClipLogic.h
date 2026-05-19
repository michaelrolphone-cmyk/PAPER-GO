#pragma once

#include "DisplayRenderLogic.h"

inline bool clipRectToFramebuffer(RenderRect& rect, int width, int height) {
  if (width <= 0 || height <= 0 || rect.w <= 0 || rect.h <= 0) return false;
  int left = rect.x;
  int top = rect.y;
  int right = rect.x + rect.w;
  int bottom = rect.y + rect.h;
  if (right <= 0 || bottom <= 0) return false;
  if (left >= width || top >= height) return false;
  if (left < 0) left = 0;
  if (top < 0) top = 0;
  if (right > width) right = width;
  if (bottom > height) bottom = height;
  rect.x = left;
  rect.y = top;
  rect.w = right - left;
  rect.h = bottom - top;
  return rect.w > 0 && rect.h > 0;
}

inline bool clipLineToFramebuffer(int& x1, int& y1, int& x2, int& y2, int width, int height) {
  if (width <= 0 || height <= 0) return false;
  enum : uint8_t { LEFT = 1, RIGHT = 2, TOP = 4, BOTTOM = 8 };
  auto outCode = [width, height](int x, int y) -> uint8_t {
    uint8_t code = 0;
    if (x < 0) code |= LEFT;
    else if (x >= width) code |= RIGHT;
    if (y < 0) code |= TOP;
    else if (y >= height) code |= BOTTOM;
    return code;
  };
  uint8_t c1 = outCode(x1, y1);
  uint8_t c2 = outCode(x2, y2);
  while (true) {
    if ((c1 | c2) == 0) return true;
    if ((c1 & c2) != 0) return false;
    uint8_t cOut = c1 ? c1 : c2;
    int x = 0;
    int y = 0;
    if (cOut & TOP) {
      if (y2 == y1) return false;
      x = x1 + (x2 - x1) * (0 - y1) / (y2 - y1);
      y = 0;
    } else if (cOut & BOTTOM) {
      if (y2 == y1) return false;
      x = x1 + (x2 - x1) * ((height - 1) - y1) / (y2 - y1);
      y = height - 1;
    } else if (cOut & RIGHT) {
      if (x2 == x1) return false;
      y = y1 + (y2 - y1) * ((width - 1) - x1) / (x2 - x1);
      x = width - 1;
    } else {
      if (x2 == x1) return false;
      y = y1 + (y2 - y1) * (0 - x1) / (x2 - x1);
      x = 0;
    }
    if (cOut == c1) {
      x1 = x; y1 = y; c1 = outCode(x1, y1);
    } else {
      x2 = x; y2 = y; c2 = outCode(x2, y2);
    }
  }
}
