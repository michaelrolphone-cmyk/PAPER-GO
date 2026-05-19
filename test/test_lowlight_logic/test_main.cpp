#include <unity.h>
#include "LowlightLogic.h"

void test_lowlight_toggle_only_when_enabled() {
  LowlightState s{};
  s.enabled = false;
  s.backlightOn = true;
  toggleLowlightBacklight(s);
  TEST_ASSERT_TRUE(s.backlightOn);

  s.enabled = true;
  s.backlightOn = true;
  toggleLowlightBacklight(s);
  TEST_ASSERT_FALSE(s.backlightOn);
  toggleLowlightBacklight(s);
  TEST_ASSERT_TRUE(s.backlightOn);
}

void test_should_backlight_be_on() {
  LowlightState s{};
  s.enabled = false;
  s.backlightOn = false;
  TEST_ASSERT_TRUE(shouldBacklightBeOn(s));

  s.enabled = true;
  s.backlightOn = false;
  TEST_ASSERT_FALSE(shouldBacklightBeOn(s));

  s.backlightOn = true;
  TEST_ASSERT_TRUE(shouldBacklightBeOn(s));
}

void test_set_lowlight_mode_preserves_backlight_state() {
  LowlightState s{};
  s.enabled = true;
  s.backlightOn = false;

  setLowlightMode(s, false);
  TEST_ASSERT_FALSE(s.enabled);
  TEST_ASSERT_FALSE(s.backlightOn);
  TEST_ASSERT_TRUE(shouldBacklightBeOn(s));

  s.backlightOn = true;
  setLowlightMode(s, true);
  TEST_ASSERT_TRUE(s.enabled);
  TEST_ASSERT_TRUE(s.backlightOn);
  TEST_ASSERT_TRUE(shouldBacklightBeOn(s));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_lowlight_toggle_only_when_enabled);
  RUN_TEST(test_should_backlight_be_on);
  RUN_TEST(test_set_lowlight_mode_preserves_backlight_state);
  return UNITY_END();
}
