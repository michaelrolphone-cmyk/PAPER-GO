#pragma once
#include <Arduino.h>
#include <vector>
#include "App.h"
#include "SystemServices.h"
#include "NavLogic.h"

class AppManager {
public:
  void add(App* app);
  void begin(SystemServices& s, const String& startId);
  void open(SystemServices& s, const String& id);
  void update(SystemServices& s);
  void render(SystemServices& s, bool full=false);
  App* active() { return _active; }
private:
  std::vector<App*> _apps;
  App* _active=nullptr;
  uint32_t _lastRender=0;
  NavigationStack _nav;
  App* find(const String& id);
};
