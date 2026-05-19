#pragma once
#include <stdint.h>

inline uint8_t pack4bppPixel(uint8_t existingByte, int pixelIndex, uint8_t gray) {
  const uint8_t nibble = gray & 0x0F;
  if ((pixelIndex & 1) == 0) return (existingByte & 0xF0) | nibble; // even/left -> low nibble
  return (existingByte & 0x0F) | static_cast<uint8_t>(nibble << 4);  // odd/right -> high nibble
}
