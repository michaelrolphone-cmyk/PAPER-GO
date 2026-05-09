#include <unity.h>
#include "TouchLogic.h"

void test_tap_classification() {
  TouchClassifier c;
  c.update(true, 100, 100, 0);
  TouchEvent end = c.update(false, 102, 101, 100);
  TEST_ASSERT_EQUAL(TouchType::Tap, end.type);
}

void test_swipe_left_classification() {
  TouchClassifier c;
  c.update(true, 200, 100, 0);
  c.update(true, 130, 102, 80);
  TouchEvent end = c.update(false, 130, 102, 100);
  TEST_ASSERT_EQUAL(TouchType::SwipeLeft, end.type);
}

void test_long_press_classification() {
  TouchClassifier c;
  c.update(true, 50, 50, 0);
  TouchEvent mid = c.update(true, 52, 49, 700);
  TEST_ASSERT_EQUAL(TouchType::LongPress, mid.type);
}

void test_drag_event_while_active() {
  TouchClassifier c;
  c.update(true, 10, 10, 0);
  TouchEvent move = c.update(true, 20, 15, 50);
  TEST_ASSERT_EQUAL(TouchType::Drag, move.type);
  TEST_ASSERT_EQUAL(10, move.dx);
  TEST_ASSERT_EQUAL(5, move.dy);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_tap_classification);
  RUN_TEST(test_swipe_left_classification);
  RUN_TEST(test_long_press_classification);
  RUN_TEST(test_drag_event_while_active);
  return UNITY_END();
}
