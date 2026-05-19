#include <unity.h>
#include <vector>
#include "SpringboardLogic.h"

void test_page_count_and_start() {
  TEST_ASSERT_EQUAL(0, springboardPageCount(0, 15));
  TEST_ASSERT_EQUAL(1, springboardPageCount(15, 15));
  TEST_ASSERT_EQUAL(2, springboardPageCount(16, 15));
  TEST_ASSERT_EQUAL(30, springboardPageStart(2, 15));
}

void test_tapped_index_for_page() {
  int idx0 = springboardTappedIndexForPage(30, 110, 0, 15);
  TEST_ASSERT_EQUAL(0, idx0);

  int idxPage1 = springboardTappedIndexForPage(30, 110, 1, 15);
  TEST_ASSERT_EQUAL(15, idxPage1);

  int outside = springboardTappedIndexForPage(900, 520, 0, 15);
  TEST_ASSERT_EQUAL(-1, outside);
}

void test_move_to_front() {
  std::vector<String> ids = {"gpsmap", "radio", "weather"};
  TEST_ASSERT_TRUE(springboardMoveAppToFront(ids, 2));
  TEST_ASSERT_EQUAL_STRING("weather", ids[0].c_str());
  TEST_ASSERT_FALSE(springboardMoveAppToFront(ids, 0));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_page_count_and_start);
  RUN_TEST(test_tapped_index_for_page);
  RUN_TEST(test_move_to_front);
  return UNITY_END();
}
