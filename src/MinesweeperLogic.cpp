#include "MinesweeperLogic.h"

namespace {
bool parseLongStrict(const String& value, long& out) {
  if (value.isEmpty()) return false;
  const char* raw = value.c_str();
  char* end = nullptr;
  long parsed = strtol(raw, &end, 10);
  if (end == raw || *end != '\0') return false;
  out = parsed;
  return true;
}
}

bool MinesweeperGame::begin(uint8_t w,uint8_t h,uint8_t mines){ if(!w||!h||mines>=w*h) return false; _w=w;_h=h;_mines=mines;_state=State::InProgress; _mine.assign(w*h,0);_rev.assign(w*h,0);_flag.assign(w*h,0); for(int i=0;i<mines;i++) _mine[i]=1; return true; }
bool MinesweeperGame::isRevealed(uint8_t x,uint8_t y) const{ return x<_w&&y<_h&&_rev[idx(x,y)]; }
bool MinesweeperGame::isFlagged(uint8_t x,uint8_t y) const{ return x<_w&&y<_h&&_flag[idx(x,y)]; }
uint8_t MinesweeperGame::adjacent(uint8_t x,uint8_t y) const{ uint8_t n=0; for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){ if(!dx&&!dy)continue; int nx=x+dx,ny=y+dy; if(nx>=0&&ny>=0&&nx<_w&&ny<_h) n+=_mine[idx(nx,ny)]; } return n; }
bool MinesweeperGame::toggleFlag(uint8_t x,uint8_t y){ if(_state!=State::InProgress||x>=_w||y>=_h||_rev[idx(x,y)]) return false; _flag[idx(x,y)]^=1; return true; }
void MinesweeperGame::flood(uint8_t x,uint8_t y){ if(x>=_w||y>=_h) return; int i=idx(x,y); if(_rev[i]||_flag[i]) return; _rev[i]=1; if(adjacent(x,y)!=0) return; for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){ if(dx||dy) flood(x+dx,y+dy);} }
void MinesweeperGame::updateWin(){ int safe=0,rev=0; for(size_t i=0;i<_mine.size();i++){ if(!_mine[i]) safe++; if(_rev[i]&&!_mine[i]) rev++; } if(rev==safe) _state=State::Won; }
bool MinesweeperGame::reveal(uint8_t x,uint8_t y){ if(_state!=State::InProgress||x>=_w||y>=_h||_flag[idx(x,y)]) return false; if(_mine[idx(x,y)]){ _rev[idx(x,y)]=1; _state=State::Lost; return true;} flood(x,y); updateWin(); return true; }

String MinesweeperGame::serialize() const{
  size_t cells = static_cast<size_t>(_w) * _h;
  if (!cells || _mine.size() != cells || _rev.size() != cells || _flag.size() != cells) return String();
  String out = String("v1,") + String(_w) + "," + String(_h) + "," + String(_mines) + "," + String(static_cast<int>(_state)) + ",";
  out.reserve(out.length() + cells * 3);
  for (size_t i = 0; i < cells; ++i) {
    out += static_cast<char>('0' + (_mine[i] ? 1 : 0));
    out += static_cast<char>('0' + (_rev[i] ? 1 : 0));
    out += static_cast<char>('0' + (_flag[i] ? 1 : 0));
  }
  return out;
}

bool MinesweeperGame::deserialize(const String& data){
  if (!data.startsWith("v1,")) return false;
  int p1 = data.indexOf(',', 3);
  int p2 = p1 >= 0 ? data.indexOf(',', p1 + 1) : -1;
  int p3 = p2 >= 0 ? data.indexOf(',', p2 + 1) : -1;
  int p4 = p3 >= 0 ? data.indexOf(',', p3 + 1) : -1;
  if (p1 < 0 || p2 < 0 || p3 < 0 || p4 < 0) return false;
  long wVal = 0;
  long hVal = 0;
  long minesVal = 0;
  long stateVal = 0;
  if (!parseLongStrict(data.substring(3, p1), wVal)) return false;
  if (!parseLongStrict(data.substring(p1 + 1, p2), hVal)) return false;
  if (!parseLongStrict(data.substring(p2 + 1, p3), minesVal)) return false;
  if (!parseLongStrict(data.substring(p3 + 1, p4), stateVal)) return false;
  if (wVal <= 0 || hVal <= 0 || wVal > 255 || hVal > 255) return false;
  size_t cells = static_cast<size_t>(wVal) * static_cast<size_t>(hVal);
  if (minesVal < 0 || minesVal >= static_cast<long>(cells)) return false;
  if (stateVal < static_cast<int>(State::InProgress) || stateVal > static_cast<int>(State::Lost)) return false;
  String enc = data.substring(p4 + 1);
  if (enc.length() != static_cast<int>(cells * 3)) return false;
  std::vector<uint8_t> mine(cells,0), rev(cells,0), flag(cells,0);
  for (size_t i = 0; i < cells; ++i) {
    char m = enc[i*3], r = enc[i*3+1], f = enc[i*3+2];
    if ((m != '0' && m != '1') || (r != '0' && r != '1') || (f != '0' && f != '1')) return false;
    mine[i] = static_cast<uint8_t>(m - '0');
    rev[i] = static_cast<uint8_t>(r - '0');
    flag[i] = static_cast<uint8_t>(f - '0');
  }
  size_t mineCount = 0;
  for (size_t i = 0; i < cells; ++i) mineCount += mine[i] ? 1U : 0U;
  if (mineCount != static_cast<size_t>(minesVal)) return false;
  _w = static_cast<uint8_t>(wVal); _h = static_cast<uint8_t>(hVal); _mines = static_cast<uint8_t>(minesVal);
  _state = static_cast<State>(stateVal);
  _mine = std::move(mine); _rev = std::move(rev); _flag = std::move(flag);
  return true;
}
