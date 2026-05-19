#include <Arduino.h>
#include <unity.h>
#include "StatusBarRenderLogic.h"

void test_snapshot_equal_when_identical() {
  StatusBarSnapshot a;
  a.wifi = true;
  a.sdMounted = true;
  a.batteryPercent = 81;
  a.charging = false;
  a.gpsState = GpsStatusState::Fix3D;
  a.timeSource = TimeSource::GPS;
  a.activeTitle = "Springboard";

  StatusBarSnapshot b = a;
  TEST_ASSERT_TRUE(statusBarSnapshotEqual(a, b));
}

void test_should_render_on_first_snapshot() {
  StatusBarSnapshot current;
  current.activeTitle = "Lock";
  TEST_ASSERT_TRUE(shouldRenderStatusBar(nullptr, current));
}

void test_should_not_render_when_unchanged() {
  StatusBarSnapshot previous;
  previous.wifi = true;
  previous.activeTitle = "Map";

  StatusBarSnapshot current = previous;
  TEST_ASSERT_FALSE(shouldRenderStatusBar(&previous, current));
}

void test_should_render_when_any_field_changes() {
  StatusBarSnapshot previous;
  previous.wifi = true;
  previous.batteryPercent = 60;
  previous.activeTitle = "Map";

  StatusBarSnapshot current = previous;
  current.batteryPercent = 59;
  TEST_ASSERT_TRUE(shouldRenderStatusBar(&previous, current));

  current = previous;
  current.activeTitle = "Weather";
  TEST_ASSERT_TRUE(shouldRenderStatusBar(&previous, current));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_snapshot_equal_when_identical);
  RUN_TEST(test_should_render_on_first_snapshot);
  RUN_TEST(test_should_not_render_when_unchanged);
  RUN_TEST(test_should_render_when_any_field_changes);
  UNITY_END();
}

void loop() {}
