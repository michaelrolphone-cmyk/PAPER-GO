#include <unity.h>
#include "TouchInputLogic.h"

void test_decode_single_point_payload() {
  uint8_t payload[] = {
    0x81,
    0x01, 0x34, 0x12, 0x78, 0x56, 0x10, 0x00, 0x00
  };
  TouchPointSample out;
  TEST_ASSERT_TRUE(decodeGt911TouchPayload(payload, sizeof(payload), out));
  TEST_ASSERT_TRUE(out.touching);
  TEST_ASSERT_FALSE(out.twoPoint);
  TEST_ASSERT_EQUAL(0x1234, out.x1);
  TEST_ASSERT_EQUAL(0x5678, out.y1);
}

void test_decode_two_point_payload() {
  uint8_t payload[] = {
    0x82,
    0x01, 0x10, 0x00, 0x20, 0x00, 0, 0, 0,
    0x02, 0x30, 0x00, 0x40, 0x00, 0, 0, 0
  };
  TouchPointSample out;
  TEST_ASSERT_TRUE(decodeGt911TouchPayload(payload, sizeof(payload), out));
  TEST_ASSERT_TRUE(out.touching);
  TEST_ASSERT_TRUE(out.twoPoint);
  TEST_ASSERT_EQUAL(16, out.x1);
  TEST_ASSERT_EQUAL(32, out.y1);
  TEST_ASSERT_EQUAL(48, out.x2);
  TEST_ASSERT_EQUAL(64, out.y2);
}

void test_decode_not_ready_returns_false() {
  uint8_t payload[] = {0x01, 0, 0, 0, 0, 0, 0, 0, 0};
  TouchPointSample out;
  TEST_ASSERT_FALSE(decodeGt911TouchPayload(payload, sizeof(payload), out));
}

void test_decode_ready_zero_touch_returns_release_state() {
  uint8_t payload[] = {0x80};
  TouchPointSample out;
  TEST_ASSERT_TRUE(decodeGt911TouchPayload(payload, sizeof(payload), out));
  TEST_ASSERT_FALSE(out.touching);
  TEST_ASSERT_FALSE(out.twoPoint);
}

void test_map_touch_to_landscape_rotates_and_clamps() {
  int16_t x = 0, y = 0;
  mapTouchToLandscape(960, 540, 539, 959, 100, 50, x, y);
  TEST_ASSERT_EQUAL(50, x);
  TEST_ASSERT_EQUAL(439, y);

  mapTouchToLandscape(960, 540, 539, 959, -100, 9999, x, y);
  TEST_ASSERT_EQUAL(959, x);
  TEST_ASSERT_EQUAL(539, y);
}

void test_map_touch_scales_non_native_touch_range() {
  int16_t x = 0, y = 0;
  mapTouchToLandscape(960, 540, 1080, 1920, 540, 960, x, y);
  TEST_ASSERT_INT_WITHIN(1, 479, x);
  TEST_ASSERT_INT_WITHIN(1, 269, y);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_decode_single_point_payload);
  RUN_TEST(test_decode_two_point_payload);
  RUN_TEST(test_decode_not_ready_returns_false);
  RUN_TEST(test_decode_ready_zero_touch_returns_release_state);
  RUN_TEST(test_map_touch_to_landscape_rotates_and_clamps);
  RUN_TEST(test_map_touch_scales_non_native_touch_range);
  return UNITY_END();
}
