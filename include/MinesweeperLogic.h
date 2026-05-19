#pragma once
#include <Arduino.h>
#include <vector>

class MinesweeperGame {
public:
  enum class State:uint8_t{InProgress=0,Won,Lost};
  bool begin(uint8_t w,uint8_t h,uint8_t mines);
  bool reveal(uint8_t x,uint8_t y);
  bool toggleFlag(uint8_t x,uint8_t y);
  bool isRevealed(uint8_t x,uint8_t y) const;
  bool isFlagged(uint8_t x,uint8_t y) const;
  uint8_t adjacent(uint8_t x,uint8_t y) const;
  uint8_t mineCount() const { return _mines; }
  State state() const { return _state; }
private:
  uint8_t _w=0,_h=0,_mines=0; State _state=State::InProgress;
  std::vector<uint8_t> _mine,_rev,_flag;
  int idx(uint8_t x,uint8_t y) const { return y*_w+x; }
  void flood(uint8_t x,uint8_t y);
  void updateWin();
};
