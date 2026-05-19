#include "AppManager.h"
#include "BoardConfig.h"
#include "Services.h"
#include "StatusLogic.h"
#include "StatusBarRenderLogic.h"

void AppManager::add(App* app) { _apps.push_back(app); }
App* AppManager::find(const String& id) { for(auto* a:_apps) if(id == a->id()) return a; return nullptr; }
void AppManager::begin(SystemServices& s, const String& startId) { open(s, startId); }
void AppManager::open(SystemServices& s, const String& id) {
  App* next=find(id); if(!next) return;
  String currentId = _active ? String(_active->id()) : String("");
  if(_active) _active->onStop(s);
  _nav.onOpen(currentId, id);
  _active=next; s.activeAppId=id; _active->onStart(s); render(s, true);
}
void AppManager::update(SystemServices& s) {
  if(!_active) return;
  uint32_t now=millis();
  _active->update(s, now);
  TouchEvent ev=s.board->pollTouch();
  if(ev.type == TouchType::SwipeDown) s.requestHome = true;
  else if(ev.type == TouchType::SwipeRight && String(_active->id()) != "springboard") s.requestBack = true;
  else if(ev.type != TouchType::None) _active->handleTouch(s, ev);
  if(s.requestHome) { s.requestHome=false; _nav.clear(); open(s,"springboard"); return; }
  if(s.requestBack) { s.requestBack=false; String backId = _nav.popBackTarget(); if(backId.length()) open(s, backId); else open(s,"springboard"); return; }
  if(s.requestOpenApp.length()) { String id=s.requestOpenApp; s.requestOpenApp=""; open(s,id); return; }
  if(now - _lastRender > 5000) render(s, false);
}
void AppManager::render(SystemServices& s, bool full) {
  if(!_active) return;
  _lastRender=millis();
  s.board->beginFrame();
  BatteryStatus b=s.board->battery();
  NetStatus n=s.net->status();
  GpsFix g=s.gps->fix();
  GpsStatusState gpsState = deriveGpsStatus(g, 15000, true);
  TimeSource timeSource = resolveTimeSource({false, g.valid, n.wifi, false});
  StatusBarSnapshot snapshot;
  snapshot.wifi = n.wifi;
  snapshot.sdMounted = s.board->sdMounted();
  snapshot.batteryPercent = b.percent;
  snapshot.charging = b.charging;
  snapshot.gpsState = gpsState;
  snapshot.timeSource = timeSource;
  snapshot.activeTitle = _active->title();

  bool renderBar = full || shouldRenderStatusBar(_havePreviousStatusBar ? &_previousStatusBar : nullptr, snapshot);
  if (renderBar) {
    s.board->fillRect(0,0,BoardConfig::SCREEN_W,BoardConfig::STATUS_BAR_H,14);
    s.board->drawText(8,10,"T5 Field OS",0,1);
    s.board->drawText(160,10,String("WiFi:")+(n.wifi?"on":"off"),0,1);
    s.board->drawText(260,10,String("GPS:")+gpsStatusLabel(gpsState),0,1);
    s.board->drawText(380,10,String("SD:")+(snapshot.sdMounted?"ok":"fail"),0,1);
    s.board->drawText(470,10,String("BAT:")+(b.percent>=0?String(b.percent)+"%":"?"),0,1);
    s.board->drawText(580,10,b.charging?"charging":"",0,1);
    s.board->drawText(700,10,String("TIME:")+timeSourceLabel(timeSource),0,1);
    s.board->drawText(850,10,_active->title(),0,1);
    _previousStatusBar = snapshot;
    _havePreviousStatusBar = true;
  }
  _active->render(s);
  s.board->endFrame(full);
}
