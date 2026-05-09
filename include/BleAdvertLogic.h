#pragma once
#include <Arduino.h>

String buildBleAdvertSummary(const String& name, int rssi, int serviceUuidCount, int manufacturerDataBytes);
