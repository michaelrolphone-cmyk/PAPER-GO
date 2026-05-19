#pragma once
#include <Arduino.h>

namespace BoardConfig {
static constexpr uint16_t SCREEN_W = 960;
static constexpr uint16_t SCREEN_H = 540;
static constexpr uint8_t STATUS_BAR_H = 36;
static constexpr uint8_t GRAYS = 16;

// LILYGO T5-4.7-S3 Pro pin map.
static constexpr int PIN_SD_CS = 12;
static constexpr int PIN_GPS_RX = 44;
static constexpr int PIN_GPS_TX = 43;
static constexpr uint32_t GPS_BAUD = 9600;

static constexpr int PIN_LORA_NSS = 46;
static constexpr int PIN_LORA_DIO1 = 10;
static constexpr int PIN_LORA_RST = 1;
static constexpr int PIN_LORA_BUSY = 47;
static constexpr float LORA_FREQ_MHZ = 915.0;

static constexpr int I2C_SDA = 39;
static constexpr int I2C_SCL = 40;
static constexpr uint8_t GT911_ADDR = 0x5D;
static constexpr uint8_t RTC_ADDR = 0x51;
static constexpr uint8_t BQ25896_ADDR = 0x6B;
static constexpr uint8_t BQ27220_ADDR = 0x55;

// Display control line for ED047TC1 timing.
static constexpr int PIN_BACKLIGHT = 48;
}
