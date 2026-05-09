#pragma once
#include <Arduino.h>
#include "Types.h"

String buildStatusApiJson(const NetStatus& net, const GpsFix& gps, const BatteryStatus& batt, bool sdMounted, bool webRunning);
