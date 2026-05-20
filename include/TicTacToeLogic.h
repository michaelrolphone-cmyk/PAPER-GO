#pragma once

#include <Arduino.h>

class TicTacToeGame {
public:
  static constexpr int kBoardSize = 9;

  enum class Cell : uint8_t {
    Empty = 0,
    X,
    O
  };

  enum class State : uint8_t {
    InProgress = 0,
    XWon,
    OWon,
    Draw
  };

  void reset();
  bool playMove(uint8_t cellIndex);
  Cell currentPlayer() const { return _currentPlayer; }
  Cell cellAt(uint8_t cellIndex) const;
  State state() const { return _state; }
  bool isFinished() const { return _state != State::InProgress; }

  String serialize() const;
  bool deserialize(const String& payload);

  // Optional simple AI helper: chooses a legal move for the current player.
  // Returns false when no legal move is available or game is finished.
  bool chooseSimpleAiMove(uint8_t& outCellIndex) const;

private:
  Cell _board[kBoardSize] = {Cell::Empty, Cell::Empty, Cell::Empty, Cell::Empty, Cell::Empty, Cell::Empty, Cell::Empty, Cell::Empty, Cell::Empty};
  Cell _currentPlayer = Cell::X;
  State _state = State::InProgress;

  void updateState();
};
