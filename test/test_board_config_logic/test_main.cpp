#include <Arduino.h>
#include <unity.h>
#include "BoardConfig.h"

void test_board_pin_map_matches_t5_47_s3_pro() {
  TEST_ASSERT_EQUAL(44, BoardConfig::PIN_GPS_RX);
  TEST_ASSERT_EQUAL(43, BoardConfig::PIN_GPS_TX);
  TEST_ASSERT_EQUAL(39, BoardConfig::I2C_SDA);
  TEST_ASSERT_EQUAL(40, BoardConfig::I2C_SCL);

  TEST_ASSERT_EQUAL(12, BoardConfig::PIN_SD_CS);
  TEST_ASSERT_EQUAL(21, BoardConfig::PIN_SPI_MISO);
  TEST_ASSERT_EQUAL(13, BoardConfig::PIN_SPI_MOSI);
  TEST_ASSERT_EQUAL(14, BoardConfig::PIN_SPI_SCLK);
  TEST_ASSERT_EQUAL(46, BoardConfig::PIN_LORA_NSS);
  TEST_ASSERT_EQUAL(10, BoardConfig::PIN_LORA_DIO1);
  TEST_ASSERT_EQUAL(1, BoardConfig::PIN_LORA_RST);
  TEST_ASSERT_EQUAL(47, BoardConfig::PIN_LORA_BUSY);

  TEST_ASSERT_EQUAL(11, BoardConfig::PIN_BL_EN);
  TEST_ASSERT_EQUAL(3, BoardConfig::PIN_TOUCH_INT);
  TEST_ASSERT_EQUAL(9, BoardConfig::PIN_TOUCH_RST);
  TEST_ASSERT_EQUAL(2, BoardConfig::PIN_RTC_IRQ);
  TEST_ASSERT_EQUAL(0x20, BoardConfig::PCA9535_ADDR);
  TEST_ASSERT_EQUAL(38, BoardConfig::PIN_PCA9535_INT);
  TEST_ASSERT_EQUAL(12, BoardConfig::PCA9535_IO_BUTTON);
  TEST_ASSERT_EQUAL(48, BoardConfig::EP_CKV);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_board_pin_map_matches_t5_47_s3_pro);
  UNITY_END();
}

void loop() {}
