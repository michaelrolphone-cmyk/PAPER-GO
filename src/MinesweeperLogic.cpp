#include "MinesweeperLogic.h"
bool MinesweeperGame::begin(uint8_t w,uint8_t h,uint8_t mines){ if(!w||!h||mines>=w*h) return false; _w=w;_h=h;_mines=mines;_state=State::InProgress; _mine.assign(w*h,0);_rev.assign(w*h,0);_flag.assign(w*h,0); for(int i=0;i<mines;i++) _mine[i]=1; return true; }
bool MinesweeperGame::isRevealed(uint8_t x,uint8_t y) const{ return x<_w&&y<_h&&_rev[idx(x,y)]; }
bool MinesweeperGame::isFlagged(uint8_t x,uint8_t y) const{ return x<_w&&y<_h&&_flag[idx(x,y)]; }
uint8_t MinesweeperGame::adjacent(uint8_t x,uint8_t y) const{ uint8_t n=0; for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){ if(!dx&&!dy)continue; int nx=x+dx,ny=y+dy; if(nx>=0&&ny>=0&&nx<_w&&ny<_h) n+=_mine[idx(nx,ny)]; } return n; }
bool MinesweeperGame::toggleFlag(uint8_t x,uint8_t y){ if(_state!=State::InProgress||x>=_w||y>=_h||_rev[idx(x,y)]) return false; _flag[idx(x,y)]^=1; return true; }
void MinesweeperGame::flood(uint8_t x,uint8_t y){ if(x>=_w||y>=_h) return; int i=idx(x,y); if(_rev[i]||_flag[i]) return; _rev[i]=1; if(adjacent(x,y)!=0) return; for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){ if(dx||dy) flood(x+dx,y+dy);} }
void MinesweeperGame::updateWin(){ int safe=0,rev=0; for(size_t i=0;i<_mine.size();i++){ if(!_mine[i]) safe++; if(_rev[i]&&!_mine[i]) rev++; } if(rev==safe) _state=State::Won; }
bool MinesweeperGame::reveal(uint8_t x,uint8_t y){ if(_state!=State::InProgress||x>=_w||y>=_h||_flag[idx(x,y)]) return false; if(_mine[idx(x,y)]){ _rev[idx(x,y)]=1; _state=State::Lost; return true;} flood(x,y); updateWin(); return true; }
