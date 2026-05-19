#include <unity.h>
#include "AppRenderDecisionLogic.h"

void test_no_input_does_not_render() {
  TEST_ASSERT_FALSE(shouldRenderAfterInputEvent(TouchType::None, false, false));
}

void test_touch_requests_render() {
  TEST_ASSERT_TRUE(shouldRenderAfterInputEvent(TouchType::Tap, false, false));
  TEST_ASSERT_TRUE(shouldRenderAfterInputEvent(TouchType::SwipeLeft, false, false));
}

void test_button_press_requests_render() {
  TEST_ASSERT_TRUE(shouldRenderAfterInputEvent(TouchType::None, true, false));
  TEST_ASSERT_TRUE(shouldRenderAfterInputEvent(TouchType::None, false, true));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_no_input_does_not_render);
  RUN_TEST(test_touch_requests_render);
  RUN_TEST(test_button_press_requests_render);
  UNITY_END();
}

void loop() {}
