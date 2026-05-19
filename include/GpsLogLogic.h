#pragma once

#include <Arduino.h>
#include "Types.h"

bool shouldLogGpsFix(const GpsFix& fix, const GpsFix& previousLoggedFix, uint32_t minMoveMeters);
String buildGpsTrackCsvLine(const GpsFix& fix, double headingDeg, bool headingReliable);

uint64_t buildGpsEpochSeconds(int year, int month, int day, int hour, int minute, int second);
