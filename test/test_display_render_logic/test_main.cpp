#include <Arduino.h>
#include <unity.h>

#include "DisplayRenderLogic.h"

void test_clamp_gray_level_bounds() {
  TEST_ASSERT_EQUAL_UINT8(0, clampGrayLevel(-5));
  TEST_ASSERT_EQUAL_UINT8(0, clampGrayLevel(0));
  TEST_ASSERT_EQUAL_UINT8(7, clampGrayLevel(7));
  TEST_ASSERT_EQUAL_UINT8(15, clampGrayLevel(15));
  TEST_ASSERT_EQUAL_UINT8(15, clampGrayLevel(99));
}

void test_clip_rect_to_display_intersection() {
  RenderRect rect{-20, -10, 80, 40};
  TEST_ASSERT_TRUE(clipRectToDisplay(rect));
  TEST_ASSERT_EQUAL(0, rect.x);
  TEST_ASSERT_EQUAL(0, rect.y);
  TEST_ASSERT_EQUAL(60, rect.w);
  TEST_ASSERT_EQUAL(30, rect.h);
}

void test_clip_rect_to_display_rejects_outside() {
  RenderRect rect{1200, 10, 20, 20};
  TEST_ASSERT_FALSE(clipRectToDisplay(rect));

  RenderRect empty{20, 20, 0, 10};
  TEST_ASSERT_FALSE(clipRectToDisplay(empty));
}


void test_clip_line_to_display_intersection() {
  int x1=-20, y1=20, x2=20, y2=20;
  TEST_ASSERT_TRUE(clipLineToDisplay(x1, y1, x2, y2));
  TEST_ASSERT_EQUAL(0, x1);
  TEST_ASSERT_EQUAL(20, y1);
  TEST_ASSERT_EQUAL(20, x2);
  TEST_ASSERT_EQUAL(20, y2);
}

void test_clip_line_to_display_rejects_outside() {
  int x1=-30, y1=-20, x2=-10, y2=-5;
  TEST_ASSERT_FALSE(clipLineToDisplay(x1, y1, x2, y2));
}

void test_glyph5x7_row_maps_letters_digits_and_fallback() {
  TEST_ASSERT_EQUAL_HEX8(0x1F, glyph5x7Row('E', 0));
  TEST_ASSERT_EQUAL_HEX8(0x1F, glyph5x7Row('e', 0));
  TEST_ASSERT_EQUAL_HEX8(0x0E, glyph5x7Row('0', 0));
  TEST_ASSERT_EQUAL_HEX8(0x00, glyph5x7Row(' ', 3));
  TEST_ASSERT_EQUAL_HEX8(0x0E, glyph5x7Row('@', 0));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_clamp_gray_level_bounds);
  RUN_TEST(test_clip_rect_to_display_intersection);
  RUN_TEST(test_clip_rect_to_display_rejects_outside);
  RUN_TEST(test_clip_line_to_display_intersection);
  RUN_TEST(test_clip_line_to_display_rejects_outside);
  RUN_TEST(test_glyph5x7_row_maps_letters_digits_and_fallback);
  UNITY_END();
}

void loop() {}
