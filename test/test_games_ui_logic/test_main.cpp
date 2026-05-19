#include <unity.h>
#include "GamesUiLogic.h"

void test_menu_selection_mapping() {
  TEST_ASSERT_EQUAL((int)GamesMenuSelection::Chess, (int)gamesMenuSelectionAt(60, 200));
  TEST_ASSERT_EQUAL((int)GamesMenuSelection::Go, (int)gamesMenuSelectionAt(280, 200));
  TEST_ASSERT_EQUAL((int)GamesMenuSelection::TicTacToe, (int)gamesMenuSelectionAt(500, 200));
  TEST_ASSERT_EQUAL((int)GamesMenuSelection::Minesweeper, (int)gamesMenuSelectionAt(720, 200));
  TEST_ASSERT_EQUAL((int)GamesMenuSelection::None, (int)gamesMenuSelectionAt(10, 10));
}

void test_tictactoe_cell_mapping_and_bounds() {
  TEST_ASSERT_EQUAL(0, ticTacToeCellAt(40, 150));
  TEST_ASSERT_EQUAL(8, ticTacToeCellAt(399, 509));
  TEST_ASSERT_EQUAL(-1, ticTacToeCellAt(39, 200));
  TEST_ASSERT_EQUAL(-1, ticTacToeCellAt(500, 510));
}

void test_other_board_mappings() {
  uint8_t x = 0, y = 0, idx = 0;
  TEST_ASSERT_TRUE(minesweeperCellAt(40, 140, x, y));
  TEST_ASSERT_EQUAL(0, x); TEST_ASSERT_EQUAL(0, y);
  TEST_ASSERT_FALSE(minesweeperCellAt(39, 140, x, y));

  TEST_ASSERT_TRUE(chessCellAt(40, 130, idx));
  TEST_ASSERT_EQUAL(0, idx);
  TEST_ASSERT_TRUE(chessCellAt(479, 569, idx));
  TEST_ASSERT_EQUAL(63, idx);

  TEST_ASSERT_TRUE(goCellAt(60, 150, x, y));
  TEST_ASSERT_EQUAL(0, x); TEST_ASSERT_EQUAL(0, y);
  TEST_ASSERT_TRUE(goCellAt(509, 599, x, y));
  TEST_ASSERT_EQUAL(8, x); TEST_ASSERT_EQUAL(8, y);
  TEST_ASSERT_FALSE(goCellAt(59, 200, x, y));
}

void test_state_labels() {
  TEST_ASSERT_EQUAL_STRING("In Progress", ticTacToeStateLabel(TicTacToeGame::State::InProgress));
  TEST_ASSERT_EQUAL_STRING("X Won", ticTacToeStateLabel(TicTacToeGame::State::XWon));
  TEST_ASSERT_EQUAL_STRING("Draw", ticTacToeStateLabel(TicTacToeGame::State::Draw));

  TEST_ASSERT_EQUAL_STRING("In Progress", minesweeperStateLabel(MinesweeperGame::State::InProgress));
  TEST_ASSERT_EQUAL_STRING("Won", minesweeperStateLabel(MinesweeperGame::State::Won));
  TEST_ASSERT_EQUAL_STRING("Lost", minesweeperStateLabel(MinesweeperGame::State::Lost));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_menu_selection_mapping);
  RUN_TEST(test_tictactoe_cell_mapping_and_bounds);
  RUN_TEST(test_other_board_mappings);
  RUN_TEST(test_state_labels);
  return UNITY_END();
}
