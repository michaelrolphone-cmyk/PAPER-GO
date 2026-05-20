#include <unity.h>
#include <vector>
#include "SpringboardLogic.h"

void test_page_count_and_start() {
  TEST_ASSERT_EQUAL(0, springboardPageCount(0, 10));
  TEST_ASSERT_EQUAL(1, springboardPageCount(10, 10));
  TEST_ASSERT_EQUAL(2, springboardPageCount(11, 10));
  TEST_ASSERT_EQUAL(20, springboardPageStart(2, 10));
}

void test_tapped_index_for_page() {
  int idx0 = springboardTappedIndexForPage(40, 100, 0, 10);
  TEST_ASSERT_EQUAL(0, idx0);

  int idxPage1 = springboardTappedIndexForPage(40, 100, 1, 10);
  TEST_ASSERT_EQUAL(10, idxPage1);

  int lastTileIdx = springboardTappedIndexForPage(760, 290, 0, 10);
  TEST_ASSERT_EQUAL(9, lastTileIdx);

  int outside = springboardTappedIndexForPage(900, 520, 0, 10);
  TEST_ASSERT_EQUAL(-1, outside);
}

void test_online_required_unavailable_state() {
  TEST_ASSERT_TRUE(springboardOnlineRequiredUnavailable(false, false));
  TEST_ASSERT_FALSE(springboardOnlineRequiredUnavailable(false, true));
  TEST_ASSERT_FALSE(springboardOnlineRequiredUnavailable(true, false));
}

void test_can_open_app_matrix() {
  TEST_ASSERT_FALSE(springboardCanOpenApp(false, false));
  TEST_ASSERT_TRUE(springboardCanOpenApp(false, true));
  TEST_ASSERT_TRUE(springboardCanOpenApp(true, false));
  TEST_ASSERT_TRUE(springboardCanOpenApp(true, true));
}

void test_move_to_front() {
  std::vector<String> ids = {"gpsmap", "radio", "weather"};
  TEST_ASSERT_TRUE(springboardMoveAppToFront(ids, 2));
  TEST_ASSERT_EQUAL_STRING("weather", ids[0].c_str());
  TEST_ASSERT_FALSE(springboardMoveAppToFront(ids, 0));
}

void test_home_press_moves_to_first_page_only_when_needed() {
  size_t page = 2;
  TEST_ASSERT_TRUE(springboardHandleHomePress(page));
  TEST_ASSERT_EQUAL(0, page);

  TEST_ASSERT_FALSE(springboardHandleHomePress(page));
  TEST_ASSERT_EQUAL(0, page);
}


void test_swipe_page_wrap_navigation() {
  size_t page = 0;
  TEST_ASSERT_TRUE(springboardRetreatPage(page, 3));
  TEST_ASSERT_EQUAL(2, page);

  TEST_ASSERT_TRUE(springboardAdvancePage(page, 3));
  TEST_ASSERT_EQUAL(0, page);

  TEST_ASSERT_TRUE(springboardAdvancePage(page, 3));
  TEST_ASSERT_EQUAL(1, page);
}

void test_swipe_page_wrap_noop_for_single_page() {
  size_t page = 0;
  TEST_ASSERT_FALSE(springboardAdvancePage(page, 1));
  TEST_ASSERT_FALSE(springboardRetreatPage(page, 1));
  TEST_ASSERT_EQUAL(0, page);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_page_count_and_start);
  RUN_TEST(test_tapped_index_for_page);
  RUN_TEST(test_online_required_unavailable_state);
  RUN_TEST(test_can_open_app_matrix);
  RUN_TEST(test_move_to_front);
  RUN_TEST(test_home_press_moves_to_first_page_only_when_needed);
  RUN_TEST(test_swipe_page_wrap_navigation);
  RUN_TEST(test_swipe_page_wrap_noop_for_single_page);
  return UNITY_END();
}
