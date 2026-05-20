#pragma once

#include <Arduino.h>

uint16_t decodeLittleEndianWord(uint8_t low, uint8_t high);
float bq27220MilliVoltsToVolts(uint16_t milliVolts);
float bq27220CurrentRawToMilliamps(uint16_t rawCurrent);
int clampBatteryPercent(int percent);
bool bq25896ChargeStatusIndicatesCharging(uint16_t chargeStatusWord);
