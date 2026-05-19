#include "StatusBarRenderLogic.h"

bool statusBarSnapshotEqual(const StatusBarSnapshot& a, const StatusBarSnapshot& b) {
  return a.wifi == b.wifi &&
         a.sdMounted == b.sdMounted &&
         a.batteryPercent == b.batteryPercent &&
         a.charging == b.charging &&
         a.gpsState == b.gpsState &&
         a.timeSource == b.timeSource &&
         a.activeTitle == b.activeTitle;
}

bool shouldRenderStatusBar(const StatusBarSnapshot* previous, const StatusBarSnapshot& current) {
  if (previous == nullptr) return true;
  return !statusBarSnapshotEqual(*previous, current);
}
