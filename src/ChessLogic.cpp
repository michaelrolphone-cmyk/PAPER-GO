#include "ChessLogic.h"
namespace { int r(int i){return i/8;} int c(int i){return i%8;} bool inb(int x){return x>=0&&x<64;} }

void ChessGame::reset(){ for(auto &s:_b)s={Piece::Empty,Color::None}; _turn=Color::White;
  auto set=[&](int i,Piece p,Color col){_b[i]={p,col};};
  Piece back[8]={Piece::Rook,Piece::Knight,Piece::Bishop,Piece::Queen,Piece::King,Piece::Bishop,Piece::Knight,Piece::Rook};
  for(int i=0;i<8;i++){ set(i,back[i],Color::Black); set(8+i,Piece::Pawn,Color::Black); set(48+i,Piece::Pawn,Color::White); set(56+i,back[i],Color::White);} }
ChessGame::Square ChessGame::at(uint8_t i) const { return i<64?_b[i]:Square{Piece::Empty,Color::None}; }
bool ChessGame::clearPath(uint8_t f,uint8_t t) const { int dr=(r(t)>r(f))-(r(t)<r(f)), dc=(c(t)>c(f))-(c(t)<c(f)); int x=f+dr*8+dc; while(x!=t){ if(_b[x].piece!=Piece::Empty)return false; x+=dr*8+dc;} return true; }
bool ChessGame::pseudoLegal(uint8_t f,uint8_t t) const { if(!inb(f)||!inb(t)||f==t)return false; auto a=_b[f], b=_b[t]; if(a.piece==Piece::Empty||a.color==b.color)return false; int dr=r(t)-r(f), dc=c(t)-c(f), adr=abs(dr), adc=abs(dc); int dir=a.color==Color::White?-1:1;
 switch(a.piece){ case Piece::Pawn: if(dc==0&&b.piece==Piece::Empty){ if(dr==dir) return true; if((r(f)==6&&a.color==Color::White)||(r(f)==1&&a.color==Color::Black)) return dr==2*dir&&_b[f+dir*8].piece==Piece::Empty; } if(adr==1&&adc==1&&dr==dir&&b.piece!=Piece::Empty) return true; return false;
 case Piece::Knight: return (adr==2&&adc==1)||(adr==1&&adc==2);
 case Piece::Bishop: return adr==adc&&clearPath(f,t);
 case Piece::Rook: return (dr==0||dc==0)&&clearPath(f,t);
 case Piece::Queen: return ((adr==adc)||(dr==0||dc==0))&&clearPath(f,t);
 case Piece::King: return adr<=1&&adc<=1;
 default:return false; }
}
int ChessGame::kingPos(Color c0) const { for(int i=0;i<64;i++) if(_b[i].piece==Piece::King&&_b[i].color==c0) return i; return -1; }
bool ChessGame::inCheck(Color c0) const { int k=kingPos(c0); if(k<0) return false; Color opp=c0==Color::White?Color::Black:Color::White; for(int i=0;i<64;i++) if(_b[i].color==opp&&pseudoLegal(i,k)) return true; return false; }
bool ChessGame::legal(uint8_t f,uint8_t t) const { if(!pseudoLegal(f,t) || _b[f].color!=_turn) return false; ChessGame tmp=*this; tmp._b[t]=tmp._b[f]; tmp._b[f]={Piece::Empty,Color::None}; if(tmp._b[t].piece==Piece::Pawn && (r(t)==0||r(t)==7)) tmp._b[t].piece=Piece::Queen; return !tmp.inCheck(_turn); }
bool ChessGame::playMove(uint8_t f,uint8_t t){ if(!legal(f,t)) return false; _b[t]=_b[f]; _b[f]={Piece::Empty,Color::None}; if(_b[t].piece==Piece::Pawn && (r(t)==0||r(t)==7)) _b[t].piece=Piece::Queen; _turn=_turn==Color::White?Color::Black:Color::White; return true; }
bool ChessGame::isCheckmate(Color c0) const { if(!inCheck(c0)) return false; ChessGame tmp=*this; tmp._turn=c0; for(int f=0;f<64;f++) if(_b[f].color==c0) for(int t=0;t<64;t++) if(tmp.legal(f,t)) return false; return true; }
String ChessGame::serialize() const { String s="ch:"; const char* pcs=".PNBRQK"; for(int i=0;i<64;i++){ char p=pcs[(int)_b[i].piece]; if(_b[i].color==Color::Black&&p!='.') p+=32; s+=p;} s+=":"; s+=(_turn==Color::White?'w':'b'); return s; }
bool ChessGame::deserialize(const String& s){ if(!s.startsWith("ch:")||s.length()<68) return false; String b=s.substring(3,67); if(s.charAt(67)!=':') return false; for(int i=0;i<64;i++){ char ch=b.charAt(i); Color col=Color::None; if(ch>='a'&&ch<='z'){col=Color::Black; ch-=32;} else if(ch>='A'&&ch<='Z') col=Color::White; Piece p=Piece::Empty; if(ch=='P')p=Piece::Pawn; else if(ch=='N')p=Piece::Knight; else if(ch=='B')p=Piece::Bishop; else if(ch=='R')p=Piece::Rook; else if(ch=='Q')p=Piece::Queen; else if(ch=='K')p=Piece::King; _b[i]={p,p==Piece::Empty?Color::None:col}; }
 _turn=s.charAt(68)=='b'?Color::Black:Color::White; return true; }
