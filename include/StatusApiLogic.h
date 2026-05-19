#pragma once
#include <Arduino.h>
#include "Types.h"

String buildStatusApiJson(const NetStatus& net, const GpsFix& gps, const BatteryStatus& batt, bool sdMounted, bool webRunning, bool unreadMessages, bool cacheActivity);
bool hasUnreadMeshtasticMessages(size_t messageFiles);
bool hasRecentCacheActivity(uint32_t lastLookupMs, uint32_t nowMs, uint32_t activeWindowMs);
