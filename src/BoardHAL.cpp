#include "BoardHAL.h"
#include "BoardConfig.h"
#include "DisplayFramebufferClipLogic.h"
#include "DisplayPixelPackingLogic.h"
#include "DisplayRenderLogic.h"
#include "DisplayUpdateModeLogic.h"
#include "TouchInputLogic.h"
#include "BatteryGaugeLogic.h"
#include "TouchProbeLogic.h"
#include "RtcProbeLogic.h"
#include <SPI.h>

#if __has_include(<epdiy.h>)
#include <epdiy.h>
#else
#error "epdiy.h not found. The paper display driver is not in the build."
#endif

namespace {
constexpr const EpdWaveform* kWaveform = EPD_BUILTIN_WAVEFORM;
EpdiyHighlevelState g_hl;
bool g_displayReady = false;
bool g_bootSplashDrawn = false;
int g_fbWidth = BoardConfig::SCREEN_W;
int g_fbHeight = BoardConfig::SCREEN_H;

inline void powerDownDisplayPeripherals() {
#if defined(ARDUINO_T5_E_PAPER_S3_V7)
  epd_powerdown_lilygo_t5_47();
#else
  epd_poweroff();
#endif
}

inline void setPixel4bpp(int x, int y, uint8_t gray) {
  if (!g_displayReady) return;
  if (x < 0 || y < 0 || x >= g_fbWidth || y >= g_fbHeight) return;
  uint8_t* fb = epd_hl_get_framebuffer(&g_hl);
  if (!fb) return;
  const int idx = y * g_fbWidth + x;
  const int byteIndex = idx >> 1;
  fb[byteIndex] = pack4bppPixel(fb[byteIndex], idx, gray);
}
}

bool BoardHAL::begin() {
  Serial.begin(115200);
  delay(250);
  Wire.begin(BoardConfig::I2C_SDA, BoardConfig::I2C_SCL);
  SPI.begin(BoardConfig::PIN_SPI_SCLK, BoardConfig::PIN_SPI_MISO, BoardConfig::PIN_SPI_MOSI);
  pinMode(BoardConfig::PIN_SD_CS, OUTPUT);
  digitalWrite(BoardConfig::PIN_SD_CS, HIGH);
  pinMode(BoardConfig::PIN_LORA_NSS, OUTPUT);
  digitalWrite(BoardConfig::PIN_LORA_NSS, HIGH);
  pinMode(BoardConfig::PIN_BL_EN, OUTPUT);
  if (BoardConfig::PIN_HOME_BUTTON >= 0) pinMode(BoardConfig::PIN_HOME_BUTTON, INPUT_PULLUP);
  if (BoardConfig::PIN_PWR_BUTTON >= 0) pinMode(BoardConfig::PIN_PWR_BUTTON, INPUT_PULLUP);
  if (BoardConfig::PIN_TOUCH_INT >= 0) pinMode(BoardConfig::PIN_TOUCH_INT, INPUT);
  if (BoardConfig::PIN_TOUCH_RST >= 0) {
    pinMode(BoardConfig::PIN_TOUCH_RST, OUTPUT);
    digitalWrite(BoardConfig::PIN_TOUCH_RST, LOW);
    delay(5);
    digitalWrite(BoardConfig::PIN_TOUCH_RST, HIGH);
    delay(55);
  }

  auto probeI2cAddress = [](uint8_t addr)->bool {
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
  };
  _touchAddr = selectGt911Address(probeI2cAddress(0x5D), probeI2cAddress(0x14));

  Wire.beginTransmission(BoardConfig::RTC_ADDR);
  _rtcAvailable = isRtcI2cProbeSuccess(static_cast<uint8_t>(Wire.endTransmission()));
  Serial.printf("RTC probe 0x%02X: %s\n", BoardConfig::RTC_ADDR, _rtcAvailable ? "ok" : "failed");
  if (_touchAddr == 0) {
    Serial.println("GT911 probe failed at 0x5D and 0x14");
  } else {
    Serial.printf("GT911 detected at 0x%02X\n", _touchAddr);
  }
  _lowlight.enabled = false;
  _lowlight.backlightOn = false;
  applyBacklightState();
  // Bring up the panel first and force a known-good smoke frame before other services.
  epd_init(&epd_board_v7, &ED047TC1, EPD_LUT_64K);
  g_hl = epd_hl_init(kWaveform);
  epd_set_rotation(EPD_ROT_LANDSCAPE);
  epd_set_lcd_pixel_clock_MHz(17);
  const int rotatedW = epd_rotated_display_width();
  const int rotatedH = epd_rotated_display_height();
  g_fbWidth = rotatedW;
  g_fbHeight = rotatedH;
  Serial.printf("EPD rotated size: %dx%d\n", rotatedW, rotatedH);
  if (rotatedW != BoardConfig::SCREEN_W || rotatedH != BoardConfig::SCREEN_H) {
    Serial.printf("EPD size mismatch: expected %dx%d\n", BoardConfig::SCREEN_W, BoardConfig::SCREEN_H);
  }
  epd_poweron();
  epd_clear();
  powerDownDisplayPeripherals();
  g_displayReady = true;
  clear(15);
  fillRect(40, 40, 300, 120, 0);
  drawText(60, 80, "PAPER GO", 15, 3);
  endFrame(true);
  g_bootSplashDrawn = true;
  Serial.println("EPD init: ok");
  beginSD();
  Serial.printf("T5 Field OS HAL online (boot splash: %s)\n", g_bootSplashDrawn ? "drawn" : "not-drawn");
  return true;
}

bool BoardHAL::beginSD() {
  _sdMounted = SD.begin(BoardConfig::PIN_SD_CS);
  Serial.printf("SD mount: %s\n", _sdMounted ? "ok" : "failed");
  return _sdMounted;
}

void BoardHAL::beginFrame() {}
void BoardHAL::endFrame(bool fullRefresh) {
  if (!g_displayReady) return;
  epd_poweron();
  const int mode = selectDisplayUpdateMode(fullRefresh, MODE_GC16, MODE_GL16);
  epd_hl_update_screen(&g_hl, static_cast<enum EpdDrawMode>(mode), epd_ambient_temperature());
  powerDownDisplayPeripherals();
}

void BoardHAL::clear(uint8_t gray) {
  uint8_t safeGray = clampGrayLevel(gray);
  const int h = g_fbHeight;
  const int w = g_fbWidth;
  for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) setPixel4bpp(x,y,safeGray);
}

void BoardHAL::fillRect(int x,int y,int w,int h,uint8_t gray) {
  RenderRect rect{x,y,w,h};
  if (!clipRectToFramebuffer(rect, g_fbWidth, g_fbHeight)) return;
  uint8_t safeGray = clampGrayLevel(gray);
  for (int yy = rect.y; yy < rect.y + rect.h; ++yy) for (int xx = rect.x; xx < rect.x + rect.w; ++xx) setPixel4bpp(xx,yy,safeGray);
}

void BoardHAL::drawRect(int x,int y,int w,int h,uint8_t gray) {
  fillRect(x,y,w,1,gray); fillRect(x,y+h-1,w,1,gray); fillRect(x,y,1,h,gray); fillRect(x+w-1,y,1,h,gray);
}
void BoardHAL::drawText(int x,int y,const String& text,uint8_t gray,uint8_t size) {
  uint8_t safeGray = clampGrayLevel(gray);
  int cursorX = x;
  for (size_t i = 0; i < text.length(); ++i) {
    char c = text.charAt(i);
    for (int row = 0; row < 7; ++row) {
      uint8_t bits = glyph5x7Row(c, static_cast<uint8_t>(row));
      for (int col = 0; col < 5; ++col) {
        if ((bits >> (4 - col)) & 0x01) {
          fillRect(cursorX + col * size, y + row * size, size, size, safeGray);
        }
      }
    }
    cursorX += 6 * size;
  }
}
void BoardHAL::drawLine(int x1,int y1,int x2,int y2,uint8_t gray) {
  if (!clipLineToFramebuffer(x1, y1, x2, y2, g_fbWidth, g_fbHeight)) return;
  uint8_t safeGray = clampGrayLevel(gray);
  int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
  int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
  int err = dx + dy;
  while (true) {
    setPixel4bpp(x1, y1, safeGray);
    if (x1 == x2 && y1 == y2) break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x1 += sx; }
    if (e2 <= dx) { err += dx; y1 += sy; }
  }
}
// rest unchanged
TouchEvent BoardHAL::pollTouch() {
  constexpr uint16_t kGt911StatusReg = 0x814E;
  constexpr uint16_t kGt911Point1Reg = 0x814F;
  constexpr size_t kMaxRead = 1 + (8 * 2);
  uint8_t status = 0;
  bool hadSample = false;
  if (_touchAddr == 0) return _touchClassifier.update(false, _touchX, _touchY, millis());
  Wire.beginTransmission(_touchAddr);
  Wire.write(static_cast<uint8_t>((kGt911StatusReg >> 8) & 0xFF));
  Wire.write(static_cast<uint8_t>(kGt911StatusReg & 0xFF));
  if (Wire.endTransmission(false) == 0 && Wire.requestFrom(static_cast<int>(_touchAddr), 1) == 1) {
    status = Wire.read();
    uint8_t count = status & 0x0F;
    if (status & 0x80) {
      hadSample = true;
      size_t toRead = 1 + ((count >= 2 ? 2 : 1) * 8);
      if (toRead > kMaxRead) toRead = kMaxRead;
      uint8_t payload[kMaxRead];
      payload[0] = status;
      if (count > 0) {
        Wire.beginTransmission(_touchAddr);
        Wire.write(static_cast<uint8_t>((kGt911Point1Reg >> 8) & 0xFF));
        Wire.write(static_cast<uint8_t>(kGt911Point1Reg & 0xFF));
        if (Wire.endTransmission(false) == 0 && Wire.requestFrom(static_cast<int>(_touchAddr), static_cast<int>(toRead - 1)) == static_cast<int>(toRead - 1)) {
          for (size_t i = 1; i < toRead; ++i) payload[i] = Wire.read();
        }
      }
      TouchPointSample sample;
      if (decodeGt911TouchPayload(payload, toRead, sample)) {
        _touching = sample.touching;
        _touchTwoPoint = sample.twoPoint;
        mapTouchToLandscape(BoardConfig::SCREEN_W, BoardConfig::SCREEN_H, BoardConfig::TOUCH_MAX_X, BoardConfig::TOUCH_MAX_Y, sample.x1, sample.y1, _touchX, _touchY);
        if (sample.twoPoint) mapTouchToLandscape(BoardConfig::SCREEN_W, BoardConfig::SCREEN_H, BoardConfig::TOUCH_MAX_X, BoardConfig::TOUCH_MAX_Y, sample.x2, sample.y2, _touchX2, _touchY2);
      } else {
        _touching = false;
        _touchTwoPoint = false;
      }
      Wire.beginTransmission(_touchAddr);
      Wire.write(static_cast<uint8_t>((kGt911StatusReg >> 8) & 0xFF));
      Wire.write(static_cast<uint8_t>(kGt911StatusReg & 0xFF));
      Wire.write(static_cast<uint8_t>(0x00));
      Wire.endTransmission();
    } else {
      _touching = false;
      _touchTwoPoint = false;
    }
  }
  if (!hadSample && BoardConfig::PIN_TOUCH_INT >= 0 && digitalRead(BoardConfig::PIN_TOUCH_INT) == HIGH) {
    _touching = false;
    _touchTwoPoint = false;
  }
  if (_touchTwoPoint) return _touchClassifier.updateTwoPoint(_touching, _touchX, _touchY, _touchX2, _touchY2, millis());
  return _touchClassifier.update(_touching, _touchX, _touchY, millis());
}
bool BoardHAL::pollHomeButtonPressed() { bool pressed = _homeButtonOverride ? _homeButtonPressed : (BoardConfig::PIN_HOME_BUTTON >= 0 && digitalRead(BoardConfig::PIN_HOME_BUTTON) == LOW); bool risingEdge = pressed && !_homeButtonLast; _homeButtonLast = pressed; return risingEdge; }
bool BoardHAL::pollPowerButtonPressed() { bool pressed = _powerButtonOverride ? _powerButtonPressed : (BoardConfig::PIN_PWR_BUTTON >= 0 && digitalRead(BoardConfig::PIN_PWR_BUTTON) == LOW); bool risingEdge = pressed && !_powerButtonLast; _powerButtonLast = pressed; return risingEdge; }
void BoardHAL::setTouchSample(bool touching, int16_t x, int16_t y) { _touchTwoPoint = false; _touching = touching; _touchX = x; _touchY = y; }
void BoardHAL::setHomeButtonSample(bool pressed) { _homeButtonOverride = true; _homeButtonPressed = pressed; }
void BoardHAL::setPowerButtonSample(bool pressed) { _powerButtonOverride = true; _powerButtonPressed = pressed; }
void BoardHAL::setTouchSampleTwoPoint(bool touching, int16_t x1, int16_t y1, int16_t x2, int16_t y2) { _touchTwoPoint = true; _touching = touching; _touchX = x1; _touchY = y1; _touchX2 = x2; _touchY2 = y2; }
BatteryStatus BoardHAL::battery() {
  BatteryStatus b;
  b.percent = -1;
  b.charging = false;
  b.voltage = 0;
  b.currentMa = 0;

  auto readWord = [](uint8_t device, uint8_t reg, uint16_t& out)->bool {
    Wire.beginTransmission(device);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(static_cast<int>(device), 2) != 2) return false;
    uint8_t lo = Wire.read();
    uint8_t hi = Wire.read();
    out = decodeLittleEndianWord(lo, hi);
    return true;
  };

  uint16_t soc = 0;
  if (readWord(BoardConfig::BQ27220_ADDR, 0x2C, soc)) {
    b.percent = clampBatteryPercent(static_cast<int>(soc));
  }

  uint16_t mv = 0;
  if (readWord(BoardConfig::BQ27220_ADDR, 0x08, mv)) {
    b.voltage = bq27220MilliVoltsToVolts(mv);
  }

  uint16_t currentRaw = 0;
  if (readWord(BoardConfig::BQ27220_ADDR, 0x14, currentRaw)) {
    b.currentMa = bq27220CurrentRawToMilliamps(currentRaw);
  }

  uint16_t chargeStatus = 0;
  if (readWord(BoardConfig::BQ25896_ADDR, 0x0B, chargeStatus)) {
    b.charging = bq25896ChargeStatusIndicatesCharging(chargeStatus);
  }

  return b;
}
void BoardHAL::sleepSeconds(uint32_t seconds) { esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL); esp_deep_sleep_start(); }
void BoardHAL::setLowlightMode(bool enabled) {
  setLowlightMode(_lowlight, enabled);
  applyBacklightState();
}
void BoardHAL::toggleBacklight() { toggleLowlightBacklight(_lowlight); applyBacklightState(); }
void BoardHAL::applyBacklightState() { digitalWrite(BoardConfig::PIN_BL_EN, shouldBacklightBeOn(_lowlight) ? HIGH : LOW); Serial.printf("[BACKLIGHT] lowlight=%s state=%s pin=%d\n", _lowlight.enabled ? "on" : "off", shouldBacklightBeOn(_lowlight) ? "on" : "off", BoardConfig::PIN_BL_EN); }
