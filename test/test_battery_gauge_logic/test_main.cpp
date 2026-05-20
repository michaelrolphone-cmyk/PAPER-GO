#include <unity.h>
#include "BatteryGaugeLogic.h"

void test_decode_little_endian_word() {
  TEST_ASSERT_EQUAL_UINT16(0x1234, decodeLittleEndianWord(0x34, 0x12));
}

void test_voltage_conversion() {
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.700f, bq27220MilliVoltsToVolts(3700));
}

void test_current_signed_conversion() {
  TEST_ASSERT_FLOAT_WITHIN(0.001f, -100.0f, bq27220CurrentRawToMilliamps(0xFF9C));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 250.0f, bq27220CurrentRawToMilliamps(250));
}

void test_clamp_battery_percent() {
  TEST_ASSERT_EQUAL_INT(0, clampBatteryPercent(-5));
  TEST_ASSERT_EQUAL_INT(55, clampBatteryPercent(55));
  TEST_ASSERT_EQUAL_INT(100, clampBatteryPercent(140));
}



void test_charge_status_decode() {
  TEST_ASSERT_FALSE(bq25896ChargeStatusIndicatesCharging(0x0000)); // stat=0
  TEST_ASSERT_TRUE(bq25896ChargeStatusIndicatesCharging(static_cast<uint16_t>(0x01 << 3))); // stat=1
  TEST_ASSERT_TRUE(bq25896ChargeStatusIndicatesCharging(static_cast<uint16_t>(0x02 << 3))); // stat=2
  TEST_ASSERT_FALSE(bq25896ChargeStatusIndicatesCharging(static_cast<uint16_t>(0x03 << 3))); // stat=3
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_decode_little_endian_word);
  RUN_TEST(test_voltage_conversion);
  RUN_TEST(test_current_signed_conversion);
  RUN_TEST(test_clamp_battery_percent);
  RUN_TEST(test_charge_status_decode);
  return UNITY_END();
}
