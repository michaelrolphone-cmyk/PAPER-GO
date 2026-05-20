#include "BatteryGaugeLogic.h"

uint16_t decodeLittleEndianWord(uint8_t low, uint8_t high) {
  return static_cast<uint16_t>(low) | (static_cast<uint16_t>(high) << 8);
}

float bq27220MilliVoltsToVolts(uint16_t milliVolts) {
  return static_cast<float>(milliVolts) / 1000.0f;
}

float bq27220CurrentRawToMilliamps(uint16_t rawCurrent) {
  return static_cast<float>(static_cast<int16_t>(rawCurrent));
}

int clampBatteryPercent(int percent) {
  if (percent < 0) return 0;
  if (percent > 100) return 100;
  return percent;
}


bool bq25896ChargeStatusIndicatesCharging(uint16_t chargeStatusWord) {
  uint8_t stat = static_cast<uint8_t>((chargeStatusWord >> 3) & 0x03);
  return stat == 0x01 || stat == 0x02;
}
