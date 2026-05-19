#pragma once

#include <Arduino.h>
#include "TicTacToeLogic.h"
#include "MinesweeperLogic.h"

enum class GamesMenuSelection : uint8_t { None = 0, Chess, Go, TicTacToe, Minesweeper };

GamesMenuSelection gamesMenuSelectionAt(int16_t x, int16_t y);
int8_t ticTacToeCellAt(int16_t x, int16_t y);
bool minesweeperCellAt(int16_t x, int16_t y, uint8_t& outX, uint8_t& outY);
bool chessCellAt(int16_t x, int16_t y, uint8_t& outIdx);
bool goCellAt(int16_t x, int16_t y, uint8_t& outX, uint8_t& outY);

const char* ticTacToeStateLabel(TicTacToeGame::State state);
const char* minesweeperStateLabel(MinesweeperGame::State state);
