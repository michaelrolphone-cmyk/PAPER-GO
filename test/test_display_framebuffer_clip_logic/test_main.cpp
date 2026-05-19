#include <unity.h>
#include "DisplayFramebufferClipLogic.h"

void test_clip_rect_to_framebuffer_bounds() {
  RenderRect rect{-20, -10, 40, 30};
  TEST_ASSERT_TRUE(clipRectToFramebuffer(rect, 100, 50));
  TEST_ASSERT_EQUAL(0, rect.x);
  TEST_ASSERT_EQUAL(0, rect.y);
  TEST_ASSERT_EQUAL(20, rect.w);
  TEST_ASSERT_EQUAL(20, rect.h);
}

void test_clip_rect_outside_returns_false() {
  RenderRect rect{200, 60, 10, 10};
  TEST_ASSERT_FALSE(clipRectToFramebuffer(rect, 100, 50));
}

void test_clip_line_to_framebuffer() {
  int x1 = -10, y1 = 10, x2 = 110, y2 = 10;
  TEST_ASSERT_TRUE(clipLineToFramebuffer(x1, y1, x2, y2, 100, 50));
  TEST_ASSERT_EQUAL(0, x1);
  TEST_ASSERT_EQUAL(10, y1);
  TEST_ASSERT_EQUAL(99, x2);
  TEST_ASSERT_EQUAL(10, y2);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_clip_rect_to_framebuffer_bounds);
  RUN_TEST(test_clip_rect_outside_returns_false);
  RUN_TEST(test_clip_line_to_framebuffer);
  return UNITY_END();
}
