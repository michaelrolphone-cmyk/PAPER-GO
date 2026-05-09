#include "TimeFormatLogic.h"
#include <time.h>

static bool toTm(uint64_t epoch, tm& out) {
  if (epoch == 0) return false;
  time_t t = (time_t)epoch;
  gmtime_r(&t, &out);
  return true;
}

String formatUtcDate(uint64_t epoch) {
  tm t{};
  if (!toTm(epoch, t)) return "Date: unknown";
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%d", &t);
  return String(buf);
}

String formatUtcTime(uint64_t epoch) {
  tm t{};
  if (!toTm(epoch, t)) return "Time: unknown";
  char buf[32];
  strftime(buf, sizeof(buf), "%H:%M:%S UTC", &t);
  return String(buf);
}
