#pragma once
#include <Arduino.h>
#include <vector>
#include "Types.h"

String formatRadioScanLogLine(const RadioSignal& signal, uint32_t timestampMs, const GpsFix* fix);
String buildRadioScanLog(const std::vector<RadioSignal>& signals, uint32_t timestampMs, const GpsFix* fix);
