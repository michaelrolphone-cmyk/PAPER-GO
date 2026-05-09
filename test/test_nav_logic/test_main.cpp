#include <unity.h>
#include "NavLogic.h"

void test_push_and_pop_back_target() {
  NavigationStack nav;
  nav.onOpen("springboard", "gpsmap");
  nav.onOpen("gpsmap", "weather");
  TEST_ASSERT_TRUE(nav.hasBack());
  TEST_ASSERT_EQUAL_STRING("gpsmap", nav.popBackTarget().c_str());
  TEST_ASSERT_EQUAL_STRING("springboard", nav.popBackTarget().c_str());
  TEST_ASSERT_EQUAL_STRING("", nav.popBackTarget().c_str());
}

void test_ignore_same_app_open() {
  NavigationStack nav;
  nav.onOpen("gpsmap", "gpsmap");
  TEST_ASSERT_FALSE(nav.hasBack());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_push_and_pop_back_target);
  RUN_TEST(test_ignore_same_app_open);
  return UNITY_END();
}
