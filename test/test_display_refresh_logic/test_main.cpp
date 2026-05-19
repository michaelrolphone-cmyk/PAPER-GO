#include <Arduino.h>
#include <unity.h>
#include "DisplayRefreshLogic.h"

void test_full_refresh_required_when_threshold_zero() {
  DisplayRefreshState state;
  state.partialSinceFull = 0;
  TEST_ASSERT_TRUE(shouldForceFullRefresh(state, 0));
}

void test_full_refresh_required_after_threshold_partials() {
  DisplayRefreshState state;
  state.partialSinceFull = 5;
  TEST_ASSERT_TRUE(shouldForceFullRefresh(state, 5));
  state.partialSinceFull = 4;
  TEST_ASSERT_FALSE(shouldForceFullRefresh(state, 5));
}

void test_record_refresh_resets_and_increments() {
  DisplayRefreshState state;
  state.partialSinceFull = 3;

  recordDisplayRefresh(state, false);
  TEST_ASSERT_EQUAL_UINT8(4, state.partialSinceFull);

  recordDisplayRefresh(state, true);
  TEST_ASSERT_EQUAL_UINT8(0, state.partialSinceFull);
}


void test_should_render_frame_when_any_driver_active() {
  RenderDecisionInput input;
  TEST_ASSERT_FALSE(shouldRenderFrame(input));

  input.forceFullRefresh = true;
  TEST_ASSERT_TRUE(shouldRenderFrame(input));

  input = RenderDecisionInput{};
  input.statusBarChanged = true;
  TEST_ASSERT_TRUE(shouldRenderFrame(input));

  input = RenderDecisionInput{};
  input.appRequestedRender = true;
  TEST_ASSERT_TRUE(shouldRenderFrame(input));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_full_refresh_required_when_threshold_zero);
  RUN_TEST(test_full_refresh_required_after_threshold_partials);
  RUN_TEST(test_record_refresh_resets_and_increments);
  RUN_TEST(test_should_render_frame_when_any_driver_active);
  UNITY_END();
}

void loop() {}
