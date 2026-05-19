#include <unity.h>
#include "TicTacToeLogic.h"

void test_new_game_starts_empty_with_x_turn() {
  TicTacToeGame game;
  game.reset();
  for (int i = 0; i < TicTacToeGame::kBoardSize; ++i) {
    TEST_ASSERT_EQUAL(TicTacToeGame::Cell::Empty, game.cellAt(i));
  }
  TEST_ASSERT_EQUAL(TicTacToeGame::Cell::X, game.currentPlayer());
  TEST_ASSERT_EQUAL(TicTacToeGame::State::InProgress, game.state());
}

void test_win_detection_and_invalid_replay() {
  TicTacToeGame game;
  game.reset();
  TEST_ASSERT_TRUE(game.playMove(0));
  TEST_ASSERT_TRUE(game.playMove(3));
  TEST_ASSERT_TRUE(game.playMove(1));
  TEST_ASSERT_TRUE(game.playMove(4));
  TEST_ASSERT_TRUE(game.playMove(2));
  TEST_ASSERT_EQUAL(TicTacToeGame::State::XWon, game.state());
  TEST_ASSERT_FALSE(game.playMove(5));
}

void test_draw_detection() {
  TicTacToeGame game;
  game.reset();
  uint8_t moves[] = {0,1,2,4,3,5,7,6,8};
  for (uint8_t m : moves) TEST_ASSERT_TRUE(game.playMove(m));
  TEST_ASSERT_EQUAL(TicTacToeGame::State::Draw, game.state());
}

void test_save_and_resume_roundtrip() {
  TicTacToeGame game;
  game.reset();
  game.playMove(0);
  game.playMove(4);
  game.playMove(8);

  String saved = game.serialize();
  TicTacToeGame resumed;
  TEST_ASSERT_TRUE(resumed.deserialize(saved));
  TEST_ASSERT_EQUAL(TicTacToeGame::Cell::X, resumed.cellAt(0));
  TEST_ASSERT_EQUAL(TicTacToeGame::Cell::O, resumed.cellAt(4));
  TEST_ASSERT_EQUAL(TicTacToeGame::Cell::X, resumed.cellAt(8));
  TEST_ASSERT_EQUAL(TicTacToeGame::Cell::O, resumed.currentPlayer());
  TEST_ASSERT_EQUAL(TicTacToeGame::State::InProgress, resumed.state());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_new_game_starts_empty_with_x_turn);
  RUN_TEST(test_win_detection_and_invalid_replay);
  RUN_TEST(test_draw_detection);
  RUN_TEST(test_save_and_resume_roundtrip);
  return UNITY_END();
}
