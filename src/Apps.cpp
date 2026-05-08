#include "Apps.h"
#include <SD.h>

static const char* APP_IDS[] = {"gpsmap","radio","mesh","url","markdown","files","weather","web","games","settings"};
static const char* APP_NAMES[] = {"GPS Map","Radio","Mesh","URL","Markdown","Files","Weather","Web","Games","Settings"};
static const int APP_COUNT = 10;

void SimpleListApp::titleBar(SystemServices& s, const String& t) {
  s.board->fillRect(0, BoardConfig::STATUS_BAR_H, BoardConfig::SCREEN_W, 40, 14);
  s.board->drawText(12, BoardConfig::STATUS_BAR_H+12, t, 0, 2);
  s.board->drawText(BoardConfig::SCREEN_W-90, BoardConfig::STATUS_BAR_H+12, "Home", 0, 1);
}

void SpringboardApp::render(SystemServices& s) {
  s.board->clear(15);
  s.board->drawText(20, BoardConfig::STATUS_BAR_H + 18, "T5 Field OS", 0, 3);
  const int cols=5, cellW=180, cellH=130, startX=25, startY=105;
  for(int i=0;i<APP_COUNT;i++) {
    int c=i%cols, r=i/cols, x=startX+c*cellW, y=startY+r*cellH;
    s.board->drawRect(x,y,150,100,0);
    s.board->drawText(x+18,y+30,String(APP_NAMES[i]).substring(0,10),0,2);
    s.board->drawText(x+18,y+70,String(APP_IDS[i]),7,1);
  }
}
void SpringboardApp::handleTouch(SystemServices& s, const TouchEvent& ev) {
  if(ev.type!=TouchType::Tap) return;
  const int cols=5, cellW=180, cellH=130, startX=25, startY=105;
  int c=(ev.x-startX)/cellW, r=(ev.y-startY)/cellH;
  if(c<0||c>=cols||r<0) return;
  int i=r*cols+c;
  if(i>=0 && i<APP_COUNT) s.requestOpenApp = APP_IDS[i];
}

void LockScreenApp::render(SystemServices& s) {
  s.board->clear(15);
  GpsFix f=s.gps->fix();
  s.board->drawText(40,80,"LOCK",0,2);
  s.board->drawText(40,135,"Time: RTC/GPS",0,3);
  s.board->drawRect(40,210,420,260,0);
  s.board->drawText(60,240,"Cached map preview",0,2);
  if(f.valid) s.board->drawText(60,290,String(f.lat,6)+", "+String(f.lon,6),0,1);
  else s.board->drawText(60,290,"GPS searching / last known unavailable",4,1);
  s.board->drawText(40,500,"Tap/gesture to unlock",0,1);
}

void GpsMapApp::update(SystemServices& s, uint32_t now) { s.gps->update(); }
void GpsMapApp::render(SystemServices& s) {
  titleBar(s, "GPS Map");
  GpsFix f=s.gps->fix();
  s.board->drawRect(20,95,620,390,0);
  s.board->drawText(260,280,"MAP TILE AREA",7,2);
  s.board->drawLine(330,290,370,290,0); s.board->drawLine(350,270,350,310,0);
  int x=670, y=105;
  s.board->drawText(x,y,"Fix",0,2); y+=35;
  s.board->drawText(x,y,f.valid?"valid":"no fix",f.valid?0:5,1); y+=25;
  s.board->drawText(x,y,"Lat: "+String(f.lat,6),0,1); y+=22;
  s.board->drawText(x,y,"Lon: "+String(f.lon,6),0,1); y+=22;
  s.board->drawText(x,y,"Sats: "+String(f.sats),0,1); y+=22;
  s.board->drawText(x,y,"HDOP: "+String(f.hdop,1),0,1); y+=22;
  if(s.gps->headingReliable()) s.board->drawText(x,y,"Heading: "+String(s.gps->computedHeadingDeg(),0),0,1);
  else s.board->drawText(x,y,"Heading: moving only",5,1);
  y+=35;
  s.board->drawText(x,y,"Tiles: cache first",0,1); y+=22;
  s.board->drawText(x,y,"Offline: missing shown",0,1);
}

void RadioScannerApp::onStart(SystemServices& s) {
  _signals.clear();
  auto wifi=s.net->scanWifi(); _signals.insert(_signals.end(), wifi.begin(), wifi.end());
  auto ble=s.radio->scanBLE(1500); _signals.insert(_signals.end(), ble.begin(), ble.end());
  auto lora=s.radio->scanLoRaWindow(500); _signals.insert(_signals.end(), lora.begin(), lora.end());
}
void RadioScannerApp::render(SystemServices& s) {
  titleBar(s,"Radio Scanner"); int y=100;
  s.board->drawText(20,y,"Kind  RSSI  Channel/Protocol  Name/Address",0,1); y+=26;
  for(auto& sig:_signals){
    s.board->drawText(20,y,sig.kind+"  "+String(sig.rssi)+"  "+String(sig.channel)+" "+sig.protocol+"  "+sig.name+" "+sig.address,0,1); y+=22; if(y>520)break;
  }
  if(_signals.empty()) s.board->drawText(20,y,"No signals captured in scan window.",5,1);
}

void MeshtasticApp::render(SystemServices& s) { titleBar(s,"Meshtastic"); s.board->drawText(20,110,"Meshtastic app boundary created.",0,2); s.board->drawText(20,150,"Storage: /meshtastic/messages and /meshtastic/nodes",0,1); s.board->drawText(20,180,"Next: integrate Meshtastic protocol/InkHUD-compatible layer.",0,1); }
void UrlFetcherApp::render(SystemServices& s) { titleBar(s,"URL Fetcher"); s.board->drawText(20,110,"Fetch URL over Wi-Fi, render text, cache under /cache/http.",0,1); s.board->drawText(20,145,"Input UI not wired yet; service boundary is ready.",5,1); }
void MarkdownReaderApp::render(SystemServices& s) { titleBar(s,"Markdown Reader"); String sample="# Markdown Reader\n\n- headings\n- lists\n- code blocks\n- scroll state"; s.board->drawText(20,110,sample,0,1); }

void FileExplorerApp::render(SystemServices& s) {
  titleBar(s,"Files: "+_path); int y=105;
  File root=SD.open(_path); if(!root){s.board->drawText(20,y,"Cannot open path",5,1); return;}
  File file=root.openNextFile();
  while(file && y<520){ s.board->drawText(20,y,String(file.isDirectory()?"[D] ":"[F] ")+file.name()+"  "+String(file.size())+" bytes",0,1); y+=22; file=root.openNextFile(); }
  root.close();
}

void WeatherApp::render(SystemServices& s) { titleBar(s,"Weather"); s.board->drawText(20,110,"Current weather + 5-day forecast by GPS location.",0,1); s.board->drawText(20,140,"Cache: /cache/weather, stale data must be labeled.",0,1); s.board->drawRect(20,180,880,260,0); s.board->drawText(50,220,"Weather provider fetcher not configured yet.",5,2); }
void WebServerApp::render(SystemServices& s) { titleBar(s,"Web Server"); NetStatus ns=s.net->status(); s.board->drawText(20,110,String("Status: ")+(s.web->running()?"running":"stopped"),0,2); s.board->drawText(20,150,"IP: "+ns.ip.toString(),0,1); s.board->drawText(20,180,"Serving: /webroot",0,1); s.board->drawText(20,230,"Tap upper-left content area to toggle",5,1); }
void WebServerApp::handleTouch(SystemServices& s, const TouchEvent& ev) { if(ev.type==TouchType::Tap){ if(s.web->running()) s.web->stop(); else s.web->start(); } }
void GamesApp::render(SystemServices& s) { titleBar(s,"Games"); const char* g[]={"Chess","Go","Tic-Tac-Toe","Minesweeper"}; for(int i=0;i<4;i++){ int x=40+i*220; s.board->drawRect(x,140,180,140,0); s.board->drawText(x+20,200,g[i],0,2);} }
void SettingsApp::render(SystemServices& s) { titleBar(s,"Settings"); int y=110; s.board->drawText(20,y,"Wi-Fi / GPS / LoRa / Display / Cache / Power / About",0,1); y+=30; s.board->drawText(20,y,"Developer mode: diagnostics for GPS, LoRa, display, touch, battery, SD",0,1); }
