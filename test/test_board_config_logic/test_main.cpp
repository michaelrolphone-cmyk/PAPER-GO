#include <Arduino.h>
#include <unity.h>
#include "BoardConfig.h"

void test_board_pin_map_matches_t5_47_s3_pro() {
  TEST_ASSERT_EQUAL(44, BoardConfig::PIN_GPS_RX);
  TEST_ASSERT_EQUAL(43, BoardConfig::PIN_GPS_TX);
  TEST_ASSERT_EQUAL(39, BoardConfig::I2C_SDA);
  TEST_ASSERT_EQUAL(40, BoardConfig::I2C_SCL);

  TEST_ASSERT_EQUAL(12, BoardConfig::PIN_SD_CS);
  TEST_ASSERT_EQUAL(46, BoardConfig::PIN_LORA_NSS);
  TEST_ASSERT_EQUAL(10, BoardConfig::PIN_LORA_DIO1);
  TEST_ASSERT_EQUAL(1, BoardConfig::PIN_LORA_RST);
  TEST_ASSERT_EQUAL(47, BoardConfig::PIN_LORA_BUSY);

  TEST_ASSERT_EQUAL(48, BoardConfig::PIN_BACKLIGHT);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_board_pin_map_matches_t5_47_s3_pro);
  UNITY_END();
}

void loop() {}
