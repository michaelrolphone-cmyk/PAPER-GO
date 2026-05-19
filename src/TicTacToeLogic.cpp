#include "TicTacToeLogic.h"

namespace {
char cellToChar(TicTacToeGame::Cell cell) {
  if (cell == TicTacToeGame::Cell::X) return 'X';
  if (cell == TicTacToeGame::Cell::O) return 'O';
  return '.';
}

TicTacToeGame::Cell charToCell(char c) {
  if (c == 'X') return TicTacToeGame::Cell::X;
  if (c == 'O') return TicTacToeGame::Cell::O;
  return TicTacToeGame::Cell::Empty;
}
}

void TicTacToeGame::reset() {
  for (int i = 0; i < kBoardSize; ++i) _board[i] = Cell::Empty;
  _currentPlayer = Cell::X;
  _state = State::InProgress;
}

bool TicTacToeGame::playMove(uint8_t cellIndex) {
  if (_state != State::InProgress || cellIndex >= kBoardSize || _board[cellIndex] != Cell::Empty) return false;
  _board[cellIndex] = _currentPlayer;
  updateState();
  if (_state == State::InProgress) {
    _currentPlayer = (_currentPlayer == Cell::X) ? Cell::O : Cell::X;
  }
  return true;
}

TicTacToeGame::Cell TicTacToeGame::cellAt(uint8_t cellIndex) const {
  if (cellIndex >= kBoardSize) return Cell::Empty;
  return _board[cellIndex];
}

String TicTacToeGame::serialize() const {
  String out = "ttt:";
  for (int i = 0; i < kBoardSize; ++i) out += cellToChar(_board[i]);
  out += ":";
  out += (_currentPlayer == Cell::X ? 'X' : 'O');
  out += ":";
  out += String(static_cast<int>(_state));
  return out;
}

bool TicTacToeGame::deserialize(const String& payload) {
  if (!payload.startsWith("ttt:") || payload.length() < 16) return false;
  String board = payload.substring(4, 13);
  if (board.length() != 9 || payload.charAt(13) != ':') return false;
  for (int i = 0; i < kBoardSize; ++i) _board[i] = charToCell(board.charAt(i));

  _currentPlayer = payload.charAt(14) == 'O' ? Cell::O : Cell::X;
  if (payload.charAt(15) != ':') return false;
  int stateInt = payload.substring(16).toInt();
  if (stateInt < 0 || stateInt > 3) return false;
  _state = static_cast<State>(stateInt);
  return true;
}

void TicTacToeGame::updateState() {
  static constexpr uint8_t lines[8][3] = {
    {0,1,2}, {3,4,5}, {6,7,8},
    {0,3,6}, {1,4,7}, {2,5,8},
    {0,4,8}, {2,4,6}
  };
  for (const auto& line : lines) {
    Cell a = _board[line[0]];
    if (a != Cell::Empty && a == _board[line[1]] && a == _board[line[2]]) {
      _state = a == Cell::X ? State::XWon : State::OWon;
      return;
    }
  }

  for (int i = 0; i < kBoardSize; ++i) {
    if (_board[i] == Cell::Empty) {
      _state = State::InProgress;
      return;
    }
  }
  _state = State::Draw;
}
