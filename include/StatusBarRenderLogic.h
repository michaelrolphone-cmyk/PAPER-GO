#pragma once

#include <Arduino.h>
#include "StatusLogic.h"

struct StatusBarSnapshot {
  bool wifi = false;
  bool sdMounted = false;
  int batteryPercent = -1;
  bool charging = false;
  GpsStatusState gpsState = GpsStatusState::NoData;
  TimeSource timeSource = TimeSource::Unknown;
  String activeTitle;
};

bool statusBarSnapshotEqual(const StatusBarSnapshot& a, const StatusBarSnapshot& b);
bool shouldRenderStatusBar(const StatusBarSnapshot* previous, const StatusBarSnapshot& current);
