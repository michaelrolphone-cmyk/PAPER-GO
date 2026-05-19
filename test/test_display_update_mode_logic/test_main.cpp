#include <unity.h>
#include "DisplayUpdateModeLogic.h"

void test_select_display_update_mode_prefers_full_mode_when_partial_requested() {
  const int mode = selectDisplayUpdateMode(false, 100, 200);
  TEST_ASSERT_EQUAL(100, mode);
}

void test_select_display_update_mode_prefers_full_mode_when_full_requested() {
  const int mode = selectDisplayUpdateMode(true, 100, 200);
  TEST_ASSERT_EQUAL(100, mode);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_select_display_update_mode_prefers_full_mode_when_partial_requested);
  RUN_TEST(test_select_display_update_mode_prefers_full_mode_when_full_requested);
  return UNITY_END();
}
