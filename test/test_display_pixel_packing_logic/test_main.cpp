#include <unity.h>
#include "DisplayPixelPackingLogic.h"

void test_even_pixel_uses_low_nibble() {
  const uint8_t out = pack4bppPixel(0xA0, 0, 0x03);
  TEST_ASSERT_EQUAL_HEX8(0xA3, out);
}

void test_odd_pixel_uses_high_nibble() {
  const uint8_t out = pack4bppPixel(0x0B, 1, 0x0C);
  TEST_ASSERT_EQUAL_HEX8(0xCB, out);
}

void test_gray_clamped_to_nibble() {
  const uint8_t evenOut = pack4bppPixel(0xF0, 0, 0x3F);
  const uint8_t oddOut = pack4bppPixel(0x0F, 1, 0x2A);
  TEST_ASSERT_EQUAL_HEX8(0xFF, evenOut);
  TEST_ASSERT_EQUAL_HEX8(0xAF, oddOut);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_even_pixel_uses_low_nibble);
  RUN_TEST(test_odd_pixel_uses_high_nibble);
  RUN_TEST(test_gray_clamped_to_nibble);
  return UNITY_END();
}
