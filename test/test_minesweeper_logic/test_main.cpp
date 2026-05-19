#include <unity.h>
#include "MinesweeperLogic.h"
void test_loss_on_mine(){ MinesweeperGame g; TEST_ASSERT_TRUE(g.begin(5,5,3)); TEST_ASSERT_TRUE(g.reveal(0,0)); TEST_ASSERT_EQUAL((int)MinesweeperGame::State::Lost,(int)g.state()); }
void test_flag_blocks_reveal(){ MinesweeperGame g; g.begin(5,5,1); TEST_ASSERT_TRUE(g.toggleFlag(4,4)); TEST_ASSERT_FALSE(g.reveal(4,4)); }
void test_win_state(){ MinesweeperGame g; g.begin(2,2,1); TEST_ASSERT_TRUE(g.reveal(1,0)); TEST_ASSERT_TRUE(g.reveal(0,1)); TEST_ASSERT_TRUE(g.reveal(1,1)); TEST_ASSERT_EQUAL((int)MinesweeperGame::State::Won,(int)g.state()); }
int main(){ UNITY_BEGIN(); RUN_TEST(test_loss_on_mine); RUN_TEST(test_flag_blocks_reveal); RUN_TEST(test_win_state); return UNITY_END(); }
