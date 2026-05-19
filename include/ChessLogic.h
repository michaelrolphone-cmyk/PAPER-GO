#pragma once
#include <Arduino.h>
#include <vector>

class ChessGame {
public:
  enum class Color:uint8_t{None=0,White,Black};
  enum class Piece:uint8_t{Empty=0,Pawn,Knight,Bishop,Rook,Queen,King};
  struct Square{Piece piece; Color color;};
  struct Move{uint8_t from; uint8_t to;};

  void reset();
  bool playMove(uint8_t from, uint8_t to);
  Square at(uint8_t idx) const;
  Color turn() const { return _turn; }
  bool inCheck(Color c) const;
  bool isCheckmate(Color c) const;
  String serialize() const;
  bool deserialize(const String& s);
private:
  Square _b[64]{};
  Color _turn = Color::White;
  bool pseudoLegal(uint8_t f,uint8_t t) const;
  bool clearPath(uint8_t f,uint8_t t) const;
  bool legal(uint8_t f,uint8_t t) const;
  int kingPos(Color c) const;
};
