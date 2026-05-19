#pragma once
#include <Arduino.h>
#include <WString.h>
#include <vector>

class GoGame {
public:
  enum class Stone:uint8_t{Empty=0,Black,White};
  bool begin(uint8_t size=9);
  bool place(uint8_t x,uint8_t y);
  bool pass();
  Stone at(uint8_t x,uint8_t y) const;
  Stone turn() const { return _turn; }
  uint8_t size() const { return _size; }
  String serialize() const;
  bool deserialize(const String& data);
private:
  uint8_t _size=9; Stone _turn=Stone::Black; uint8_t _consecutivePass=0;
  std::vector<Stone> _b;
  int idx(int x,int y) const { return y*_size+x; }
  bool inb(int x,int y) const { return x>=0&&y>=0&&x<_size&&y<_size; }
  void collect(int x,int y,Stone c,std::vector<int>& grp,std::vector<uint8_t>& vis) const;
  bool hasLiberty(int x,int y) const;
};
