#pragma once

#include <Arduino.h>
#include "Types.h"
#include "DgpsLogic.h"

String dgpsQualityStateLabel(DgpsQualityState state);
String buildDgpsStatusJson(const GpsFix& rawFix, const GpsFix& activeFix, bool usingDgps, DgpsQualityState quality, uint32_t ageMs, const RoverCorrectionState& state, const DgpsPacketStats& stats);
String buildDgpsPacketJson(const RoverCorrectionState& state, uint32_t ageMs);
