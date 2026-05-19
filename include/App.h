#pragma once
#include <Arduino.h>
#include "Types.h"
class SystemServices;

class App {
public:
  virtual ~App() = default;
  virtual const char* id() const = 0;
  virtual const char* title() const = 0;
  virtual void onStart(SystemServices& s) {}
  virtual void onStop(SystemServices& s) {}
  virtual void update(SystemServices& s, uint32_t now) {}
  virtual void render(SystemServices& s) = 0;
  virtual void handleTouch(SystemServices& s, const TouchEvent& ev) {}
  virtual bool handleHomeButton(SystemServices& s) { (void)s; return false; }
};
