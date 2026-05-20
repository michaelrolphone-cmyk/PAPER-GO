#include "AppManager.h"
#include "BoardConfig.h"
#include "Services.h"
#include "StatusLogic.h"
#include "StatusBarRenderLogic.h"
#include "AppRenderDecisionLogic.h"

static StatusBarSnapshot captureStatusSnapshot(SystemServices& s, App* active) {
  BatteryStatus b=s.board->battery();
  NetStatus n=s.net->status();
  GpsFix g=s.gps->fix();
  StatusBarSnapshot snapshot;
  snapshot.wifi = n.wifi;
  snapshot.sdMounted = s.board->sdMounted();
  snapshot.batteryPercent = b.percent;
  snapshot.charging = b.charging;
  snapshot.gpsState = deriveGpsStatus(g, 15000, true);
  snapshot.timeSource = resolveTimeSource({s.board ? s.board->rtcAvailable() : false, g.valid, n.wifi, false});
  snapshot.activeTitle = active ? active->title() : "";
  return snapshot;
}

void AppManager::add(App* app) { _apps.push_back(app); }
App* AppManager::find(const String& id) { for(auto* a:_apps) if(id == a->id()) return a; return nullptr; }
void AppManager::begin(SystemServices& s, const String& startId) {
  _powerState.lastInteractionMs = millis();
  if (s.cache) {
    String raw = s.cache->readText("/config/power.json", 512);
    PowerConfig cfg = parsePowerConfig(raw);
    if (cfg.valid) _powerPolicy = cfg.policy;
  }
  open(s, startId);
}
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
  bool powerPressed = s.board->pollPowerButtonPressed();
  bool homePressed = false;
  if (powerPressed) {
    _powerState.lastInteractionMs = now;
    String activeId = String(_active->id());
    if (shouldPowerButtonReturnToOnline(activeId)) {
      if (!s.net->status().wifi) s.net->connectSaved();
      s.requestHome = true;
    } else if (shouldPowerButtonEnterLowPower(activeId)) {
      open(s, "lock");
      return;
    }
  }
  homePressed = s.board->pollHomeButtonPressed();
  if (homePressed) {
    _powerState.lastInteractionMs = now;
    if (_active->handleHomeButton(s)) {
      render(s, true);
      return;
    }
    s.requestHome = true;
  }
  TouchEvent ev=s.board->pollTouch();
  if(ev.type != TouchType::None) _powerState.lastInteractionMs = now;
  if(ev.type == TouchType::SwipeDown) s.requestHome = true;
  else if(ev.type == TouchType::SwipeRight && String(_active->id()) != "springboard") s.requestBack = true;
  else if(ev.type != TouchType::None) _active->handleTouch(s, ev);
  bool interactionRenderRequested = shouldRenderAfterInputEvent(ev.type, homePressed, powerPressed);
  if(s.requestHome) { s.requestHome=false; _nav.clear(); open(s,"springboard"); return; }
  if(s.requestBack) { s.requestBack=false; String backId = _nav.popBackTarget(); if(backId.length()) open(s, backId); else open(s,"springboard"); return; }
  if(s.requestOpenApp.length()) { String id=s.requestOpenApp; s.requestOpenApp=""; open(s,id); return; }
  _powerState.lockScreenActive = String(_active->id()) == "lock";
  PowerAction action = evaluatePowerAction(_powerPolicy, _powerState, now);
  if (action == PowerAction::EnterLockScreen && !_powerState.lockScreenActive) {
    _powerState.lastInteractionMs = now;
    open(s, "lock");
    return;
  }
  if (action == PowerAction::EnterDeepSleep) {
    s.board->sleepSeconds(_powerPolicy.deepSleepDurationSec);
    return;
  }

  BatteryStatus batt = s.board->battery();
  if (shouldDisableWifiForLowPower(_powerState.lockScreenActive, batt.charging) && s.net->status().wifi) {
    s.net->disconnect();
  }

  if (interactionRenderRequested) {
    render(s, false);
    return;
  }

  if(now - _lastRender > 5000) {
    bool forceFull = shouldForceFullRefresh(_refreshState, 12);
    StatusBarSnapshot snapshot = captureStatusSnapshot(s, _active);
    bool statusChanged = shouldRenderStatusBar(_havePreviousStatusBar ? &_previousStatusBar : nullptr, snapshot);
    RenderDecisionInput decision{forceFull, statusChanged, false};
    if (shouldRenderFrame(decision)) render(s, false);
  }
}
void AppManager::render(SystemServices& s, bool full) {
  if(!_active) return;
  _lastRender=millis();
  bool fullRefresh = full || shouldForceFullRefresh(_refreshState, 12);
  s.board->beginFrame();
  BatteryStatus b=s.board->battery();
  NetStatus n=s.net->status();
  GpsFix g=s.gps->fix();
  GpsStatusState gpsState = deriveGpsStatus(g, 15000, true);
  TimeSource timeSource = resolveTimeSource({s.board ? s.board->rtcAvailable() : false, g.valid, n.wifi, false});
  StatusBarSnapshot snapshot = captureStatusSnapshot(s, _active);

  bool renderBar = fullRefresh || shouldRenderStatusBar(_havePreviousStatusBar ? &_previousStatusBar : nullptr, snapshot);
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
  s.board->endFrame(fullRefresh);
  recordDisplayRefresh(_refreshState, fullRefresh);
}
