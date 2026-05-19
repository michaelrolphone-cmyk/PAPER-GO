#pragma once
#include "App.h"
#include "SystemServices.h"
#include "BoardConfig.h"
#include "Services.h"
#include "SettingsLogic.h"
#include <vector>
#include <vector>

class SimpleListApp : public App {
protected:
  int _scroll = 0;
  void titleBar(SystemServices& s, const String& t);
};

class SpringboardApp : public App {
public:
  const char* id() const override { return "springboard"; }
  const char* title() const override { return "Apps"; }
  void onStart(SystemServices& s) override;
  void render(SystemServices& s) override;
  void handleTouch(SystemServices& s, const TouchEvent& ev) override;
  bool handleHomeButton(SystemServices& s) override;
private:
  std::vector<String> _orderedIds;
  size_t _page = 0;
  bool _showOptions = false;
  int _selectedIndex = -1;
};
class LockScreenApp : public App {
public:
  const char* id() const override { return "lock"; }
  const char* title() const override { return "Lock"; }
  void onStart(SystemServices& s) override;
  void onStop(SystemServices& s) override;
  void render(SystemServices& s) override;
  void handleTouch(SystemServices& s, const TouchEvent& ev) override;
private:
  String _status;
  String _preview;
};
class GpsMapApp : public SimpleListApp {
public:
  const char* id() const override { return "gpsmap"; }
  const char* title() const override { return "GPS Map"; }
  void update(SystemServices& s, uint32_t now) override;
  void render(SystemServices& s) override;
};
class RadioScannerApp : public SimpleListApp {
public:
  const char* id() const override { return "radio"; }
  const char* title() const override { return "Radio Scanner"; }
  void onStart(SystemServices& s) override;
  void render(SystemServices& s) override;
private: std::vector<RadioSignal> _signals;
};
class MeshtasticApp : public SimpleListApp {
public:
  const char* id() const override { return "mesh"; }
  const char* title() const override { return "Meshtastic"; }
  void render(SystemServices& s) override;
};
class UrlFetcherApp : public SimpleListApp {
public:
  const char* id() const override { return "url"; }
  const char* title() const override { return "URL Fetcher"; }
  void onStart(SystemServices& s) override;
  void render(SystemServices& s) override;
private:
  String _status;
  String _preview;
};
class MarkdownReaderApp : public SimpleListApp {
public:
  const char* id() const override { return "markdown"; }
  const char* title() const override { return "Markdown"; }
  void onStart(SystemServices& s) override;
  void render(SystemServices& s) override;
  void handleTouch(SystemServices& s, const TouchEvent& ev) override;
private:
  int _startLine = 0;
  int _lastMaxStart = 0;
};
class FileExplorerApp : public SimpleListApp {
public:
  const char* id() const override { return "files"; }
  const char* title() const override { return "Files"; }
  void render(SystemServices& s) override;
  void handleTouch(SystemServices& s, const TouchEvent& ev) override;
private: String _path = "/";
};
class WeatherApp : public SimpleListApp {
public:
  const char* id() const override { return "weather"; }
  const char* title() const override { return "Weather"; }
  void render(SystemServices& s) override;
};
class WebServerApp : public SimpleListApp {
public:
  const char* id() const override { return "web"; }
  const char* title() const override { return "Web Server"; }
  void render(SystemServices& s) override;
  void handleTouch(SystemServices& s, const TouchEvent& ev) override;
};
class GamesApp : public SimpleListApp {
public:
  const char* id() const override { return "games"; }
  const char* title() const override { return "Games"; }
  void render(SystemServices& s) override;
};
class SettingsApp : public SimpleListApp {
public:
  const char* id() const override { return "settings"; }
  const char* title() const override { return "Settings"; }
  void onStart(SystemServices& s) override;
  void render(SystemServices& s) override;
  void handleTouch(SystemServices& s, const TouchEvent& ev) override;
private:
  SettingsViewState _state;
  void loadFromConfig(SystemServices& s);
  void savePowerConfig(SystemServices& s);
};
