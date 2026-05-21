#include "TouchInputLogic.h"

namespace {
constexpr size_t kHeaderSize = 1;
constexpr size_t kPointBlockSize = 8;
}

bool decodeGt911TouchPayload(const uint8_t* payload, size_t payloadLen, TouchPointSample& out) {
  out = TouchPointSample{};
  if (!payload || payloadLen < kHeaderSize) return false;

  uint8_t status = payload[0];
  if ((status & 0x80) == 0) return false;
  uint8_t touchCount = status & 0x0F;
  if (touchCount == 0) return true;

  size_t needed = kHeaderSize + (touchCount >= 2 ? 2 : 1) * kPointBlockSize;
  if (payloadLen < needed) return false;

  auto readPoint = [&](size_t offset, int16_t& x, int16_t& y) {
    x = static_cast<int16_t>(payload[offset + 1] | (payload[offset + 2] << 8));
    y = static_cast<int16_t>(payload[offset + 3] | (payload[offset + 4] << 8));
  };

  out.touching = true;
  readPoint(1, out.x1, out.y1);
  if (touchCount >= 2) {
    out.twoPoint = true;
    readPoint(1 + kPointBlockSize, out.x2, out.y2);
  }
  return true;
}

void mapTouchToLandscape(uint16_t panelWidth, uint16_t panelHeight,
                         uint16_t touchMaxX, uint16_t touchMaxY,
                         int16_t srcX, int16_t srcY, int16_t& dstX, int16_t& dstY,
                         bool swapXY, bool mirrorX, bool mirrorY) {
  if (panelWidth == 0 || panelHeight == 0 || touchMaxX == 0 || touchMaxY == 0) {
    dstX = 0;
    dstY = 0;
    return;
  }
  int32_t clampedX = srcX < 0 ? 0 : srcX;
  int32_t clampedY = srcY < 0 ? 0 : srcY;
  if (clampedX > touchMaxX) clampedX = touchMaxX;
  if (clampedY > touchMaxY) clampedY = touchMaxY;

  if (swapXY) {
    int32_t t = clampedX;
    clampedX = clampedY;
    clampedY = t;
    int32_t mt = touchMaxX;
    touchMaxX = touchMaxY;
    touchMaxY = mt;
  }

  if (mirrorX) clampedX = touchMaxX - clampedX;
  if (mirrorY) clampedY = touchMaxY - clampedY;

  int32_t mappedX = (clampedX * (static_cast<int32_t>(panelWidth) - 1)) / touchMaxX;
  int32_t mappedY = (clampedY * (static_cast<int32_t>(panelHeight) - 1)) / touchMaxY;
  if (mappedX < 0) mappedX = 0;
  if (mappedY < 0) mappedY = 0;
  if (mappedX >= panelWidth) mappedX = panelWidth - 1;
  if (mappedY >= panelHeight) mappedY = panelHeight - 1;
  dstX = static_cast<int16_t>(mappedX);
  dstY = static_cast<int16_t>(mappedY);
}
