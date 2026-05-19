#include <unity.h>
#include "MinesweeperLogic.h"
void test_loss_on_mine(){ MinesweeperGame g; TEST_ASSERT_TRUE(g.begin(5,5,3)); TEST_ASSERT_TRUE(g.reveal(0,0)); TEST_ASSERT_EQUAL((int)MinesweeperGame::State::Lost,(int)g.state()); }
void test_flag_blocks_reveal(){ MinesweeperGame g; g.begin(5,5,1); TEST_ASSERT_TRUE(g.toggleFlag(4,4)); TEST_ASSERT_FALSE(g.reveal(4,4)); }
void test_win_state(){ MinesweeperGame g; g.begin(2,2,1); TEST_ASSERT_TRUE(g.reveal(1,0)); TEST_ASSERT_TRUE(g.reveal(0,1)); TEST_ASSERT_TRUE(g.reveal(1,1)); TEST_ASSERT_EQUAL((int)MinesweeperGame::State::Won,(int)g.state()); }
void test_save_and_resume(){
  MinesweeperGame g; TEST_ASSERT_TRUE(g.begin(4,4,2));
  TEST_ASSERT_TRUE(g.toggleFlag(3,3));
  TEST_ASSERT_TRUE(g.reveal(2,2));
  String state = g.serialize();
  TEST_ASSERT_TRUE(state.length() > 0);

  MinesweeperGame resumed;
  TEST_ASSERT_TRUE(resumed.deserialize(state));
  TEST_ASSERT_EQUAL(g.mineCount(), resumed.mineCount());
  TEST_ASSERT_EQUAL((int)g.state(), (int)resumed.state());
  TEST_ASSERT_EQUAL(g.isFlagged(3,3), resumed.isFlagged(3,3));
  TEST_ASSERT_EQUAL(g.isRevealed(2,2), resumed.isRevealed(2,2));
}
void test_reject_invalid_state(){ MinesweeperGame g; TEST_ASSERT_FALSE(g.deserialize("v1,4,4,2,0,abc")); }
void test_reject_non_numeric_metadata(){ MinesweeperGame g; TEST_ASSERT_FALSE(g.deserialize("v1,4x,4,2,0,000")); }
void test_reject_mine_count_mismatch(){
  MinesweeperGame g;
  String enc;
  for (int i = 0; i < 16; ++i) enc += "000";
  TEST_ASSERT_FALSE(g.deserialize(String("v1,4,4,2,0,") + enc));
}
int main(){ UNITY_BEGIN(); RUN_TEST(test_loss_on_mine); RUN_TEST(test_flag_blocks_reveal); RUN_TEST(test_win_state); RUN_TEST(test_save_and_resume); RUN_TEST(test_reject_invalid_state); RUN_TEST(test_reject_non_numeric_metadata); RUN_TEST(test_reject_mine_count_mismatch); return UNITY_END(); }
