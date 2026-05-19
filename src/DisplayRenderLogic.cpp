#include "DisplayRenderLogic.h"

uint8_t clampGrayLevel(int gray) {
  if (gray < 0) return 0;
  if (gray >= BoardConfig::GRAYS) return BoardConfig::GRAYS - 1;
  return static_cast<uint8_t>(gray);
}

bool clipRectToDisplay(RenderRect& rect) {
  int left = rect.x;
  int top = rect.y;
  int right = rect.x + rect.w;
  int bottom = rect.y + rect.h;

  if (rect.w <= 0 || rect.h <= 0) return false;
  if (right <= 0 || bottom <= 0) return false;
  if (left >= BoardConfig::SCREEN_W || top >= BoardConfig::SCREEN_H) return false;

  if (left < 0) left = 0;
  if (top < 0) top = 0;
  if (right > BoardConfig::SCREEN_W) right = BoardConfig::SCREEN_W;
  if (bottom > BoardConfig::SCREEN_H) bottom = BoardConfig::SCREEN_H;

  rect.x = left;
  rect.y = top;
  rect.w = right - left;
  rect.h = bottom - top;
  return rect.w > 0 && rect.h > 0;
}


static bool clipTest(float p, float q, float& t0, float& t1) {
  if (p == 0.0f) return q >= 0.0f;
  float r = q / p;
  if (p < 0.0f) {
    if (r > t1) return false;
    if (r > t0) t0 = r;
  } else {
    if (r < t0) return false;
    if (r < t1) t1 = r;
  }
  return true;
}

bool clipLineToDisplay(int& x1, int& y1, int& x2, int& y2) {
  float t0 = 0.0f;
  float t1 = 1.0f;
  float dx = static_cast<float>(x2 - x1);
  float dy = static_cast<float>(y2 - y1);

  const float xmin = 0.0f;
  const float ymin = 0.0f;
  const float xmax = static_cast<float>(BoardConfig::SCREEN_W - 1);
  const float ymax = static_cast<float>(BoardConfig::SCREEN_H - 1);

  if (!clipTest(-dx, static_cast<float>(x1) - xmin, t0, t1)) return false;
  if (!clipTest(dx, xmax - static_cast<float>(x1), t0, t1)) return false;
  if (!clipTest(-dy, static_cast<float>(y1) - ymin, t0, t1)) return false;
  if (!clipTest(dy, ymax - static_cast<float>(y1), t0, t1)) return false;

  float nx1 = static_cast<float>(x1) + t0 * dx;
  float ny1 = static_cast<float>(y1) + t0 * dy;
  float nx2 = static_cast<float>(x1) + t1 * dx;
  float ny2 = static_cast<float>(y1) + t1 * dy;

  x1 = static_cast<int>(nx1 + 0.5f);
  y1 = static_cast<int>(ny1 + 0.5f);
  x2 = static_cast<int>(nx2 + 0.5f);
  y2 = static_cast<int>(ny2 + 0.5f);
  return true;
}

uint8_t glyph5x7Row(char c, uint8_t row) {
  if (row > 6) return 0;
  if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
  switch (c) {
    case 'A': { static const uint8_t r[7]={0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}; return r[row]; }
    case 'B': { static const uint8_t r[7]={0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}; return r[row]; }
    case 'C': { static const uint8_t r[7]={0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}; return r[row]; }
    case 'E': { static const uint8_t r[7]={0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}; return r[row]; }
    case 'F': { static const uint8_t r[7]={0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}; return r[row]; }
    case 'G': { static const uint8_t r[7]={0x0E,0x11,0x10,0x17,0x11,0x11,0x0F}; return r[row]; }
    case 'H': { static const uint8_t r[7]={0x11,0x11,0x11,0x1F,0x11,0x11,0x11}; return r[row]; }
    case 'I': { static const uint8_t r[7]={0x1F,0x04,0x04,0x04,0x04,0x04,0x1F}; return r[row]; }
    case 'K': { static const uint8_t r[7]={0x11,0x12,0x14,0x18,0x14,0x12,0x11}; return r[row]; }
    case 'L': { static const uint8_t r[7]={0x10,0x10,0x10,0x10,0x10,0x10,0x1F}; return r[row]; }
    case 'M': { static const uint8_t r[7]={0x11,0x1B,0x15,0x15,0x11,0x11,0x11}; return r[row]; }
    case 'N': { static const uint8_t r[7]={0x11,0x19,0x15,0x13,0x11,0x11,0x11}; return r[row]; }
    case 'O': { static const uint8_t r[7]={0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}; return r[row]; }
    case 'P': { static const uint8_t r[7]={0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}; return r[row]; }
    case 'R': { static const uint8_t r[7]={0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}; return r[row]; }
    case 'S': { static const uint8_t r[7]={0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}; return r[row]; }
    case 'T': { static const uint8_t r[7]={0x1F,0x04,0x04,0x04,0x04,0x04,0x04}; return r[row]; }
    case 'U': { static const uint8_t r[7]={0x11,0x11,0x11,0x11,0x11,0x11,0x0E}; return r[row]; }
    case 'V': { static const uint8_t r[7]={0x11,0x11,0x11,0x11,0x11,0x0A,0x04}; return r[row]; }
    case 'W': { static const uint8_t r[7]={0x11,0x11,0x11,0x15,0x15,0x15,0x0A}; return r[row]; }
    case 'Y': { static const uint8_t r[7]={0x11,0x11,0x0A,0x04,0x04,0x04,0x04}; return r[row]; }
    case ':': { static const uint8_t r[7]={0x00,0x04,0x00,0x00,0x04,0x00,0x00}; return r[row]; }
    case '.': { static const uint8_t r[7]={0x00,0x00,0x00,0x00,0x00,0x04,0x00}; return r[row]; }
    case '-': { static const uint8_t r[7]={0x00,0x00,0x00,0x1F,0x00,0x00,0x00}; return r[row]; }
    case '/': { static const uint8_t r[7]={0x01,0x02,0x04,0x08,0x10,0x00,0x00}; return r[row]; }
    case '%': { static const uint8_t r[7]={0x18,0x19,0x02,0x04,0x08,0x13,0x03}; return r[row]; }
    case '0': { static const uint8_t r[7]={0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}; return r[row]; }
    case '1': { static const uint8_t r[7]={0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}; return r[row]; }
    case '2': { static const uint8_t r[7]={0x0E,0x11,0x01,0x02,0x04,0x08,0x1F}; return r[row]; }
    case '3': { static const uint8_t r[7]={0x1F,0x02,0x04,0x02,0x01,0x11,0x0E}; return r[row]; }
    case '4': { static const uint8_t r[7]={0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}; return r[row]; }
    case '5': { static const uint8_t r[7]={0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E}; return r[row]; }
    case '6': { static const uint8_t r[7]={0x0E,0x10,0x1E,0x11,0x11,0x11,0x0E}; return r[row]; }
    case '7': { static const uint8_t r[7]={0x1F,0x01,0x02,0x04,0x08,0x08,0x08}; return r[row]; }
    case '8': { static const uint8_t r[7]={0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}; return r[row]; }
    case '9': { static const uint8_t r[7]={0x0E,0x11,0x11,0x0F,0x01,0x02,0x1C}; return r[row]; }
    case ' ': return 0;
    default: { static const uint8_t r[7]={0x0E,0x11,0x01,0x02,0x04,0x00,0x04}; return r[row]; }
  }
}
