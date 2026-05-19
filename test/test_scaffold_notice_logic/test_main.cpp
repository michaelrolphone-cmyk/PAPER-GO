#include <unity.h>
#include "ScaffoldNoticeLogic.h"

void test_notices_are_explicit_not_implemented() {
  TEST_ASSERT_NOT_EQUAL(-1, String(mapAppNoticeText()).indexOf("not implemented"));
  TEST_ASSERT_NOT_EQUAL(-1, String(weatherAppNoticeText()).indexOf("not implemented"));
  TEST_ASSERT_NOT_EQUAL(-1, String(meshtasticAppNoticeText()).indexOf("not implemented"));
  TEST_ASSERT_NOT_EQUAL(-1, String(gamesAppNoticeText()).indexOf("not implemented"));
  TEST_ASSERT_NOT_EQUAL(-1, String(settingsAppNoticeText()).indexOf("not implemented"));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_notices_are_explicit_not_implemented);
  return UNITY_END();
}
