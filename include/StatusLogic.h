#pragma once

#include <stdint.h>
#include "Types.h"

enum class TimeSource {
  RTC,
  GPS,
  Network,
  Saved,
  Unknown
};

enum class GpsStatusState {
  Off,
  Searching,
  Fix2D,
  Fix3D,
  Degraded,
  NoData
};

struct TimeSourceInputs {
  bool rtcAvailable = false;
  bool gpsTimeAvailable = false;
  bool networkTimeAvailable = false;
  bool savedTimeAvailable = false;
};

TimeSource resolveTimeSource(const TimeSourceInputs& inputs);
const char* timeSourceLabel(TimeSource source);
GpsStatusState deriveGpsStatus(const GpsFix& fix, uint32_t staleAgeMs, bool gpsPowered);
const char* gpsStatusLabel(GpsStatusState state);
