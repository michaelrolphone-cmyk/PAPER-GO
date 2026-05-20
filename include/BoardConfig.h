#pragma once
#include <Arduino.h>

namespace BoardConfig {
static constexpr uint16_t SCREEN_W = 540;
static constexpr uint16_t SCREEN_H = 960;
static constexpr uint8_t STATUS_BAR_H = 36;
static constexpr uint8_t GRAYS = 16;

// LILYGO T5-4.7-S3 Pro (H752-01) pin map.
static constexpr int PIN_SPI_MISO = 21;
static constexpr int PIN_SPI_MOSI = 13;
static constexpr int PIN_SPI_SCLK = 14;
static constexpr int PIN_SD_CS = 12;
static constexpr int PIN_HOME_BUTTON = 0;
static constexpr int PIN_PWR_BUTTON = -1;
static constexpr int PIN_GPS_RX = 44;
static constexpr int PIN_GPS_TX = 43;
static constexpr uint32_t GPS_BAUD_PRIMARY = 9600;
static constexpr uint32_t GPS_BAUD_ALT = 38400;
static constexpr uint32_t GPS_BAUD = GPS_BAUD_PRIMARY;

static constexpr int PIN_LORA_NSS = 46;
static constexpr int PIN_LORA_DIO1 = 10;
static constexpr int PIN_LORA_RST = 1;
static constexpr int PIN_LORA_BUSY = 47;
static constexpr float LORA_FREQ_MHZ = 915.0;

static constexpr int I2C_SDA = 39;
static constexpr int I2C_SCL = 40;
static constexpr uint8_t GT911_ADDR = 0x5D;
static constexpr int PIN_TOUCH_INT = 3;
static constexpr int PIN_TOUCH_RST = 9;
static constexpr uint16_t TOUCH_MAX_X = 539;
static constexpr uint16_t TOUCH_MAX_Y = 959;
static constexpr uint8_t RTC_ADDR = 0x51;
static constexpr int PIN_RTC_IRQ = 2;
static constexpr uint8_t BQ25896_ADDR = 0x6B;
static constexpr uint8_t BQ27220_ADDR = 0x55;
static constexpr uint8_t TPS65185_ADDR = 0x68;
static constexpr uint8_t PCA9535_ADDR = 0x20;
static constexpr int PIN_PCA9535_INT = 38;
static constexpr uint8_t PCA9535_IO_BUTTON = 12;
static constexpr int PIN_BOOT_BTN = 0;

// Frontlight enable.
static constexpr int PIN_BL_EN = 11;

// ED047TC1 timing/data pins.
static constexpr int EP_D7  = 8;
static constexpr int EP_D6  = 18;
static constexpr int EP_D5  = 17;
static constexpr int EP_D4  = 16;
static constexpr int EP_D3  = 15;
static constexpr int EP_D2  = 7;
static constexpr int EP_D1  = 6;
static constexpr int EP_D0  = 5;
static constexpr int EP_CKV = 48;
static constexpr int EP_STH = 41;
static constexpr int EP_LEH = 42;
static constexpr int EP_STV = 45;
static constexpr int EP_CKH = 4;
static constexpr int EP_INTR = 38;
}
