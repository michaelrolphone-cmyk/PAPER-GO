#pragma once

#include <stdint.h>
#include "BoardConfig.h"

struct RenderRect {
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
};

uint8_t clampGrayLevel(int gray);
bool clipRectToDisplay(RenderRect& rect);

bool clipLineToDisplay(int& x1, int& y1, int& x2, int& y2);

uint8_t glyph5x7Row(char c, uint8_t row);
