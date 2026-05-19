#include <unity.h>
#include "GoLogic.h"
void test_place_and_turn(){ GoGame g; TEST_ASSERT_TRUE(g.begin(9)); TEST_ASSERT_TRUE(g.place(4,4)); TEST_ASSERT_EQUAL((int)GoGame::Stone::White,(int)g.turn()); TEST_ASSERT_FALSE(g.place(4,4)); }
void test_capture(){ GoGame g; g.begin(9); g.place(1,1); g.place(0,1); g.place(4,4); g.place(1,0); g.place(5,5); g.place(2,1); g.place(6,6); g.place(1,2); TEST_ASSERT_EQUAL((int)GoGame::Stone::Empty,(int)g.at(1,1)); }
void test_save_and_resume(){
  GoGame g; TEST_ASSERT_TRUE(g.begin(9));
  TEST_ASSERT_TRUE(g.place(2,2));
  TEST_ASSERT_TRUE(g.place(3,3));
  TEST_ASSERT_TRUE(g.pass());
  String state = g.serialize();
  TEST_ASSERT_TRUE(state.length() > 0);

  GoGame resumed;
  TEST_ASSERT_TRUE(resumed.deserialize(state));
  TEST_ASSERT_EQUAL(g.size(), resumed.size());
  TEST_ASSERT_EQUAL((int)g.turn(), (int)resumed.turn());
  TEST_ASSERT_EQUAL((int)g.at(2,2), (int)resumed.at(2,2));
  TEST_ASSERT_EQUAL((int)g.at(3,3), (int)resumed.at(3,3));
}
void test_reject_invalid_saved_state(){ GoGame g; TEST_ASSERT_FALSE(g.deserialize("v1,9,1,0,012")); }
void test_reject_non_numeric_metadata(){ GoGame g; TEST_ASSERT_FALSE(g.deserialize("v1,9x,1,0,0000000000000000000000000")); }
void test_reject_out_of_range_pass_counter(){
  GoGame g;
  String board;
  for (int i = 0; i < 81; ++i) board += '0';
  TEST_ASSERT_FALSE(g.deserialize(String("v1,9,1,999,") + board));
}
int main(){ UNITY_BEGIN(); RUN_TEST(test_place_and_turn); RUN_TEST(test_capture); RUN_TEST(test_save_and_resume); RUN_TEST(test_reject_invalid_saved_state); RUN_TEST(test_reject_non_numeric_metadata); RUN_TEST(test_reject_out_of_range_pass_counter); return UNITY_END(); }
