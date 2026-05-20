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

void test_swipe_right_classification() {
  TouchClassifier c;
  c.update(true, 100, 100, 0);
  c.update(true, 170, 98, 80);
  TouchEvent end = c.update(false, 170, 98, 100);
  TEST_ASSERT_EQUAL(TouchType::SwipeRight, end.type);
}

void test_swipe_up_classification() {
  TouchClassifier c;
  c.update(true, 100, 200, 0);
  c.update(true, 102, 120, 80);
  TouchEvent end = c.update(false, 102, 120, 100);
  TEST_ASSERT_EQUAL(TouchType::SwipeUp, end.type);
}

void test_swipe_down_classification() {
  TouchClassifier c;
  c.update(true, 100, 100, 0);
  c.update(true, 99, 180, 80);
  TouchEvent end = c.update(false, 99, 180, 100);
  TEST_ASSERT_EQUAL(TouchType::SwipeDown, end.type);
}

void test_release_position_is_used_for_final_classification() {
  TouchClassifier c;
  c.update(true, 200, 100, 0);
  c.update(true, 180, 100, 30);
  TouchEvent end = c.update(false, 120, 100, 60);
  TEST_ASSERT_EQUAL(TouchType::SwipeLeft, end.type);
  TEST_ASSERT_EQUAL(120, end.x);
  TEST_ASSERT_EQUAL(-80, end.dx);
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

void test_two_point_pinch_out_and_in() {
  TouchClassifier c;
  c.updateTwoPoint(true, 100, 100, 120, 120, 0);
  TouchEvent out = c.updateTwoPoint(true, 90, 90, 140, 140, 30);
  TEST_ASSERT_EQUAL(TouchType::PinchOut, out.type);
  TEST_ASSERT_EQUAL(2, out.touchPoints);

  TouchClassifier c2;
  c2.updateTwoPoint(true, 80, 80, 160, 160, 0);
  TouchEvent in = c2.updateTwoPoint(true, 105, 105, 135, 135, 30);
  TEST_ASSERT_EQUAL(TouchType::PinchIn, in.type);
  TEST_ASSERT_EQUAL(2, in.touchPoints);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_tap_classification);
  RUN_TEST(test_swipe_left_classification);
  RUN_TEST(test_swipe_right_classification);
  RUN_TEST(test_swipe_up_classification);
  RUN_TEST(test_swipe_down_classification);
  RUN_TEST(test_release_position_is_used_for_final_classification);
  RUN_TEST(test_long_press_classification);
  RUN_TEST(test_drag_event_while_active);
  RUN_TEST(test_two_point_pinch_out_and_in);
  return UNITY_END();
}
