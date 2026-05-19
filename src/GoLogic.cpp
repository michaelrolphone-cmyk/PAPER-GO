#include "GoLogic.h"
#include <cstdlib>

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

bool GoGame::begin(uint8_t s){ if(s<5||s>19)return false; _size=s; _turn=Stone::Black; _consecutivePass=0; _b.assign(s*s,Stone::Empty); return true; }
GoGame::Stone GoGame::at(uint8_t x,uint8_t y) const { return inb(x,y)?_b[idx(x,y)]:Stone::Empty; }
void GoGame::collect(int x,int y,Stone c,std::vector<int>& grp,std::vector<uint8_t>& vis) const{ if(!inb(x,y)) return; int i=idx(x,y); if(vis[i]||_b[i]!=c) return; vis[i]=1; grp.push_back(i); collect(x+1,y,c,grp,vis); collect(x-1,y,c,grp,vis); collect(x,y+1,c,grp,vis); collect(x,y-1,c,grp,vis); }
bool GoGame::hasLiberty(int x,int y) const{ Stone c=at(x,y); if(c==Stone::Empty) return false; std::vector<int> g; std::vector<uint8_t> vis(_b.size(),0); collect(x,y,c,g,vis); for(int i:g){ int xx=i%_size, yy=i/_size; int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1}; for(int k=0;k<4;k++){int nx=xx+dx[k],ny=yy+dy[k]; if(inb(nx,ny)&&at(nx,ny)==Stone::Empty) return true;}} return false; }
bool GoGame::place(uint8_t x,uint8_t y){ if(!inb(x,y)||at(x,y)!=Stone::Empty) return false; GoGame tmp=*this; tmp._b[tmp.idx(x,y)]=_turn; Stone opp=_turn==Stone::Black?Stone::White:Stone::Black; int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1}; for(int k=0;k<4;k++){int nx=x+dx[k],ny=y+dy[k]; if(tmp.inb(nx,ny)&&tmp.at(nx,ny)==opp&&!tmp.hasLiberty(nx,ny)){ std::vector<int> g; std::vector<uint8_t> vis(tmp._b.size(),0); tmp.collect(nx,ny,opp,g,vis); for(int i:g) tmp._b[i]=Stone::Empty; }} if(!tmp.hasLiberty(x,y)) return false; *this=tmp; _turn=opp; _consecutivePass=0; return true; }
bool GoGame::pass(){ _turn=_turn==Stone::Black?Stone::White:Stone::Black; _consecutivePass++; return true; }

String GoGame::serialize() const{
  if (_b.empty() || _b.size() != static_cast<size_t>(_size) * _size) return String();
  String out = String("v1,") + String(_size) + "," + String(static_cast<int>(_turn)) + "," + String(_consecutivePass) + ",";
  out.reserve(out.length() + _b.size());
  for (Stone cell : _b) {
    out += static_cast<char>('0' + static_cast<int>(cell));
  }
  return out;
}

bool GoGame::deserialize(const String& data){
  if (!data.startsWith("v1,")) return false;
  int p1 = data.indexOf(',', 3);
  int p2 = p1 >= 0 ? data.indexOf(',', p1 + 1) : -1;
  int p3 = p2 >= 0 ? data.indexOf(',', p2 + 1) : -1;
  if (p1 < 0 || p2 < 0 || p3 < 0) return false;
  long sizeVal = 0;
  long turnVal = 0;
  long passVal = 0;
  if (!parseLongStrict(data.substring(3, p1), sizeVal)) return false;
  if (!parseLongStrict(data.substring(p1 + 1, p2), turnVal)) return false;
  if (!parseLongStrict(data.substring(p2 + 1, p3), passVal)) return false;
  String board = data.substring(p3 + 1);
  if (sizeVal < 5 || sizeVal > 19) return false;
  if (turnVal < static_cast<int>(Stone::Black) || turnVal > static_cast<int>(Stone::White)) return false;
  size_t expected = static_cast<size_t>(sizeVal) * static_cast<size_t>(sizeVal);
  if (board.length() != static_cast<int>(expected)) return false;
  std::vector<Stone> next;
  next.reserve(expected);
  for (size_t i = 0; i < expected; ++i) {
    char ch = board[i];
    if (ch < '0' || ch > '2') return false;
    next.push_back(static_cast<Stone>(ch - '0'));
  }
  _size = static_cast<uint8_t>(sizeVal);
  _turn = static_cast<Stone>(turnVal);
  if (passVal < 0 || passVal > 255) return false;
  _consecutivePass = static_cast<uint8_t>(passVal);
  _b = std::move(next);
  return true;
}
