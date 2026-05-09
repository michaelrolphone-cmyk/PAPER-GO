#include "StatusLogic.h"

TimeSource resolveTimeSource(const TimeSourceInputs& inputs) {
  if (inputs.rtcAvailable) return TimeSource::RTC;
  if (inputs.gpsTimeAvailable) return TimeSource::GPS;
  if (inputs.networkTimeAvailable) return TimeSource::Network;
  if (inputs.savedTimeAvailable) return TimeSource::Saved;
  return TimeSource::Unknown;
}

const char* timeSourceLabel(TimeSource source) {
  switch (source) {
    case TimeSource::RTC: return "RTC";
    case TimeSource::GPS: return "GPS";
    case TimeSource::Network: return "NET";
    case TimeSource::Saved: return "SAVED";
    default: return "UNK";
  }
}

GpsStatusState deriveGpsStatus(const GpsFix& fix, uint32_t staleAgeMs, bool gpsPowered) {
  if (!gpsPowered) return GpsStatusState::Off;
  if (!fix.valid) return GpsStatusState::Searching;
  if (fix.ageMs > staleAgeMs) return GpsStatusState::Degraded;
  if (fix.sats >= 4) return GpsStatusState::Fix3D;
  if (fix.sats >= 3) return GpsStatusState::Fix2D;
  return GpsStatusState::NoData;
}

const char* gpsStatusLabel(GpsStatusState state) {
  switch (state) {
    case GpsStatusState::Off: return "off";
    case GpsStatusState::Searching: return "search";
    case GpsStatusState::Fix2D: return "2D";
    case GpsStatusState::Fix3D: return "3D";
    case GpsStatusState::Degraded: return "stale";
    case GpsStatusState::NoData: return "nodata";
    default: return "unknown";
  }
}
