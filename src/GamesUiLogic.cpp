#include "GamesUiLogic.h"

GamesMenuSelection gamesMenuSelectionAt(int16_t x, int16_t y) {
  if (y < 140 || y > 280) return GamesMenuSelection::None;
  for (int i = 0; i < 4; ++i) {
    int left = 40 + i * 220;
    if (x >= left && x <= left + 180) {
      if (i == 0) return GamesMenuSelection::Chess;
      if (i == 1) return GamesMenuSelection::Go;
      if (i == 2) return GamesMenuSelection::TicTacToe;
      return GamesMenuSelection::Minesweeper;
    }
  }
  return GamesMenuSelection::None;
}

int8_t ticTacToeCellAt(int16_t x, int16_t y) {
  if (x < 40 || y < 150) return -1;
  int cx = (x - 40) / 120;
  int cy = (y - 150) / 120;
  if (cx < 0 || cx >= 3 || cy < 0 || cy >= 3) return -1;
  return static_cast<int8_t>(cy * 3 + cx);
}

bool minesweeperCellAt(int16_t x, int16_t y, uint8_t& outX, uint8_t& outY) {
  if (x < 40 || y < 140) return false;
  int cx = (x - 40) / 55;
  int cy = (y - 140) / 55;
  if (cx < 0 || cx >= 8 || cy < 0 || cy >= 8) return false;
  outX = static_cast<uint8_t>(cx);
  outY = static_cast<uint8_t>(cy);
  return true;
}

bool chessCellAt(int16_t x, int16_t y, uint8_t& outIdx) {
  if (x < 40 || y < 130) return false;
  int cx = (x - 40) / 55;
  int cy = (y - 130) / 55;
  if (cx < 0 || cx >= 8 || cy < 0 || cy >= 8) return false;
  outIdx = static_cast<uint8_t>(cy * 8 + cx);
  return true;
}

bool goCellAt(int16_t x, int16_t y, uint8_t& outX, uint8_t& outY) {
  if (x < 60 || y < 150) return false;
  int cx = (x - 60) / 50;
  int cy = (y - 150) / 50;
  if (cx < 0 || cx >= 9 || cy < 0 || cy >= 9) return false;
  outX = static_cast<uint8_t>(cx);
  outY = static_cast<uint8_t>(cy);
  return true;
}

const char* ticTacToeStateLabel(TicTacToeGame::State state) {
  if (state == TicTacToeGame::State::InProgress) return "In Progress";
  if (state == TicTacToeGame::State::XWon) return "X Won";
  if (state == TicTacToeGame::State::OWon) return "O Won";
  return "Draw";
}

const char* minesweeperStateLabel(MinesweeperGame::State state) {
  if (state == MinesweeperGame::State::InProgress) return "In Progress";
  if (state == MinesweeperGame::State::Won) return "Won";
  return "Lost";
}
