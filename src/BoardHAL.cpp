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

#include "epd_driver.h"

namespace {
TwoWire g_boardI2C(1);
bool g_displayReady = false;
bool g_bootSplashDrawn = false;
int g_fbWidth = BoardConfig::SCREEN_W;
int g_fbHeight = BoardConfig::SCREEN_H;
int g_panelWidth = BoardConfig::SCREEN_H;
int g_panelHeight = BoardConfig::SCREEN_W;
enum class PanelMapMode : uint8_t { Direct, RotateCW, RotateCCW };
PanelMapMode g_panelMapMode = PanelMapMode::RotateCW;
uint8_t* g_framebuffer = nullptr;
bool g_frameActive = false;
bool g_frameDirty = false;
uint16_t g_partialFramesSinceFull = 0;
constexpr uint16_t kMaxPartialFramesBeforeFull = 12;


inline void powerDownDisplayPeripherals() {
#if defined(ARDUINO_T5_E_PAPER_S3_V7)
  epd_powerdown_lilygo_t5_47();
#else
  epd_poweroff();
#endif
}


inline bool mapLogicalToPanelPixel(int lx, int ly, int& px, int& py) {
  if (lx < 0 || ly < 0 || lx >= g_fbWidth || ly >= g_fbHeight) return false;
    if (g_panelMapMode == PanelMapMode::Direct) {
    px = lx;
    py = ly;
  } else if (g_panelMapMode == PanelMapMode::RotateCCW) {
    px = (g_fbHeight - 1) - ly;
    py = lx;
  } else {
    // Logical UI portrait (540x960) mapped into landscape panel memory (960x540).
    px = ly;
    py = (g_fbWidth - 1) - lx;
  }
  return px >= 0 && py >= 0 && px < g_panelWidth && py < g_panelHeight;
}

inline void setPixel4bpp(int x, int y, uint8_t gray) {
  if (!g_displayReady) return;
  uint8_t* fb = g_framebuffer;
  if (!fb) return;
  int px = 0, py = 0;
  if (!mapLogicalToPanelPixel(x, y, px, py)) return;
  const int idx = py * g_panelWidth + px;
  const int byteIndex = idx >> 1;
  fb[byteIndex] = pack4bppPixel(fb[byteIndex], idx, gray);
  g_frameDirty = true;
}
}

bool BoardHAL::begin() {
  Serial.begin(115200);
  delay(250);
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

  _lowlight.enabled = false;
  _lowlight.backlightOn = false;
  applyBacklightState();
  // Bring up the panel first and force a known-good smoke frame before other services.
  epd_init();
  const int rotatedW = EPD_HEIGHT;
  const int rotatedH = EPD_WIDTH;
  g_panelWidth = rotatedW;
  g_panelHeight = rotatedH;
  g_fbWidth = BoardConfig::SCREEN_W;
  g_fbHeight = BoardConfig::SCREEN_H;
  if (rotatedW == g_fbWidth && rotatedH == g_fbHeight) {
    g_panelMapMode = PanelMapMode::Direct;
  } else if (rotatedW == g_fbHeight && rotatedH == g_fbWidth) {
    g_panelMapMode = BoardConfig::PANEL_ROTATE_CLOCKWISE ? PanelMapMode::RotateCW : PanelMapMode::RotateCCW;
  } else {
    g_panelMapMode = PanelMapMode::RotateCW;
    Serial.printf("EPD unexpected size %dx%d; defaulting to RotateCW mapping\n", rotatedW, rotatedH);
  }
  Serial.printf("EPD rotated size: %dx%d (logic framebuffer: %dx%d, mapMode=%u, cwPref=%s)\n",
                rotatedW, rotatedH, g_fbWidth, g_fbHeight, static_cast<unsigned>(g_panelMapMode),
                BoardConfig::PANEL_ROTATE_CLOCKWISE ? "true" : "false");
  // Avoid panel-wide clear here; it adds a second long refresh and can look like a boot freeze.
  // We draw the splash into framebuffer and perform a single explicit refresh via endFrame(true).
  g_framebuffer = (uint8_t*)ps_calloc(sizeof(uint8_t), static_cast<size_t>(EPD_WIDTH) * static_cast<size_t>(EPD_HEIGHT) / 2);
  if (!g_framebuffer) {
    Serial.println("EPD framebuffer allocation failed");
    return false;
  }
  g_displayReady = true;
  clear(15);
  // Orientation probes: distinct corner markers help validate logical->panel rotation on hardware.
  fillRect(0, 0, 28, 28, 0);
  fillRect(g_fbWidth - 28, 0, 28, 28, 4);
  fillRect(0, g_fbHeight - 28, 28, 28, 8);
  fillRect(g_fbWidth - 28, g_fbHeight - 28, 28, 28, 12);
  fillRect(40, 40, 300, 120, 0);
  drawText(60, 80, "PAPER GO", 15, 3);
  drawText(60, 125, String("MAP ") + static_cast<unsigned>(g_panelMapMode), 15, 1);
  uint32_t splashStart = millis();
  endFrame(true);
  Serial.printf("Boot splash refresh took %lums\n", static_cast<unsigned long>(millis() - splashStart));
  g_bootSplashDrawn = true;
  Serial.println("EPD init: ok");

  // Bring I2C up after EPD init because the display stack configures PMIC/GPIO over I2C.
  // Initializing Wire first can contend for the bus and trigger "common: acquire bus failed".
  Serial.println("I2C app bus: starting on controller 1");
  if (!g_boardI2C.begin(BoardConfig::I2C_SDA, BoardConfig::I2C_SCL, 400000)) {
    Serial.println("I2C app bus: begin failed");
  } else {
    Serial.println("I2C app bus: begin ok");
  }
  auto probeI2cAddress = [](uint8_t addr, void* ctx)->bool {
    TwoWire* wire = static_cast<TwoWire*>(ctx);
    wire->beginTransmission(addr);
    return wire->endTransmission() == 0;
  };
  _touchAddr = probeGt911Address(probeI2cAddress, &g_boardI2C);

  _tps65185Available = probeI2cAddress(BoardConfig::TPS65185_ADDR, &g_boardI2C);
  _pca9535Available = probeI2cAddress(BoardConfig::PCA9535_ADDR, &g_boardI2C);
  _bq25896Available = probeI2cAddress(BoardConfig::BQ25896_ADDR, &g_boardI2C);
  _bq27220Available = probeI2cAddress(BoardConfig::BQ27220_ADDR, &g_boardI2C);

  g_boardI2C.beginTransmission(BoardConfig::RTC_ADDR);
  _rtcAvailable = isRtcI2cProbeSuccess(static_cast<uint8_t>(g_boardI2C.endTransmission()));
  Serial.printf("RTC probe 0x%02X: %s\n", BoardConfig::RTC_ADDR, _rtcAvailable ? "ok" : "failed");
  Serial.printf("TPS65185 probe 0x%02X: %s\n", BoardConfig::TPS65185_ADDR, _tps65185Available ? "ok" : "failed");
  Serial.printf("PCA9535 probe 0x%02X: %s\n", BoardConfig::PCA9535_ADDR, _pca9535Available ? "ok" : "failed");
  Serial.printf("BQ25896 probe 0x%02X: %s\n", BoardConfig::BQ25896_ADDR, _bq25896Available ? "ok" : "failed");
  Serial.printf("BQ27220 probe 0x%02X: %s\n", BoardConfig::BQ27220_ADDR, _bq27220Available ? "ok" : "failed");
  if (_touchAddr == 0) {
    Serial.println("GT911 probe failed at 0x14 and 0x5D");
  } else {
    Serial.printf("GT911 detected at 0x%02X\n", _touchAddr);
  }

  beginSD();
  Serial.printf("T5 Field OS HAL online (boot splash: %s)\n", g_bootSplashDrawn ? "drawn" : "not-drawn");
  return true;
}

bool BoardHAL::beginSD() {
  _sdMounted = SD.begin(BoardConfig::PIN_SD_CS);
  Serial.printf("SD mount: %s\n", _sdMounted ? "ok" : "failed");
  return _sdMounted;
}

void BoardHAL::beginFrame() {
  g_frameActive = true;
  g_frameDirty = false;
}
void BoardHAL::endFrame(bool fullRefresh) {
  if (!g_displayReady || !g_framebuffer) return;
  bool requireFull = fullRefresh || (g_partialFramesSinceFull >= kMaxPartialFramesBeforeFull);
  if (!requireFull && !g_frameDirty) return;
  epd_poweron();
  if (requireFull) {
    epd_clear();
    g_partialFramesSinceFull = 0;
  }
  epd_draw_grayscale_image(epd_full_screen(), g_framebuffer);
  powerDownDisplayPeripherals();
  if (!requireFull) g_partialFramesSinceFull++;
  g_frameActive = false;
  g_frameDirty = false;
}

void BoardHAL::clear(uint8_t gray) {
  uint8_t safeGray = clampGrayLevel(gray);
  uint8_t* fb = g_framebuffer;
  if (!fb) return;
  const uint8_t packed = static_cast<uint8_t>((safeGray << 4) | safeGray);
  const size_t bytes = static_cast<size_t>(g_panelWidth) * static_cast<size_t>(g_panelHeight) / 2;
  memset(fb, packed, bytes);
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
  g_boardI2C.beginTransmission(_touchAddr);
  g_boardI2C.write(static_cast<uint8_t>((kGt911StatusReg >> 8) & 0xFF));
  g_boardI2C.write(static_cast<uint8_t>(kGt911StatusReg & 0xFF));
  if (g_boardI2C.endTransmission(false) == 0 && g_boardI2C.requestFrom(static_cast<int>(_touchAddr), 1) == 1) {
    status = g_boardI2C.read();
    uint8_t count = status & 0x0F;
    if (status & 0x80) {
      hadSample = true;
      size_t toRead = 1 + ((count >= 2 ? 2 : 1) * 8);
      if (toRead > kMaxRead) toRead = kMaxRead;
      uint8_t payload[kMaxRead];
      payload[0] = status;
      if (count > 0) {
        g_boardI2C.beginTransmission(_touchAddr);
        g_boardI2C.write(static_cast<uint8_t>((kGt911Point1Reg >> 8) & 0xFF));
        g_boardI2C.write(static_cast<uint8_t>(kGt911Point1Reg & 0xFF));
        if (g_boardI2C.endTransmission(false) == 0 && g_boardI2C.requestFrom(static_cast<int>(_touchAddr), static_cast<int>(toRead - 1)) == static_cast<int>(toRead - 1)) {
          for (size_t i = 1; i < toRead; ++i) payload[i] = g_boardI2C.read();
        }
      }
      TouchPointSample sample;
      if (decodeGt911TouchPayload(payload, toRead, sample)) {
        _touching = sample.touching;
        _touchTwoPoint = sample.twoPoint;
        mapTouchToLandscape(BoardConfig::SCREEN_W, BoardConfig::SCREEN_H, BoardConfig::TOUCH_MAX_X, BoardConfig::TOUCH_MAX_Y, sample.x1, sample.y1, _touchX, _touchY, BoardConfig::TOUCH_SWAP_XY, BoardConfig::TOUCH_MIRROR_X, BoardConfig::TOUCH_MIRROR_Y);
        if (sample.twoPoint) mapTouchToLandscape(BoardConfig::SCREEN_W, BoardConfig::SCREEN_H, BoardConfig::TOUCH_MAX_X, BoardConfig::TOUCH_MAX_Y, sample.x2, sample.y2, _touchX2, _touchY2, BoardConfig::TOUCH_SWAP_XY, BoardConfig::TOUCH_MIRROR_X, BoardConfig::TOUCH_MIRROR_Y);
      } else {
        _touching = false;
        _touchTwoPoint = false;
      }
      g_boardI2C.beginTransmission(_touchAddr);
      g_boardI2C.write(static_cast<uint8_t>((kGt911StatusReg >> 8) & 0xFF));
      g_boardI2C.write(static_cast<uint8_t>(kGt911StatusReg & 0xFF));
      g_boardI2C.write(static_cast<uint8_t>(0x00));
      g_boardI2C.endTransmission();
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
    g_boardI2C.beginTransmission(device);
    g_boardI2C.write(reg);
    if (g_boardI2C.endTransmission(false) != 0) return false;
    if (g_boardI2C.requestFrom(static_cast<int>(device), 2) != 2) return false;
    uint8_t lo = g_boardI2C.read();
    uint8_t hi = g_boardI2C.read();
    out = decodeLittleEndianWord(lo, hi);
    return true;
  };

  bool haveGaugeSoc = false;
  uint16_t soc = 0;
  if (readWord(BoardConfig::BQ27220_ADDR, 0x2C, soc)) {
    b.percent = clampBatteryPercent(static_cast<int>(soc));
    haveGaugeSoc = true;
  }

  bool haveGaugeMv = false;
  uint16_t mv = 0;
  if (readWord(BoardConfig::BQ27220_ADDR, 0x08, mv)) {
    b.voltage = bq27220MilliVoltsToVolts(mv);
    haveGaugeMv = true;
  }

  uint16_t currentRaw = 0;
  if (readWord(BoardConfig::BQ27220_ADDR, 0x14, currentRaw)) {
    b.currentMa = bq27220CurrentRawToMilliamps(currentRaw);
  }

  bool haveCharger = false;
  uint16_t chargeStatus = 0;
  if (readWord(BoardConfig::BQ25896_ADDR, 0x0B, chargeStatus)) {
    b.charging = bq25896ChargeStatusIndicatesCharging(chargeStatus);
    haveCharger = true;
  }

  bool usedAdcFallback = false;
  if ((!haveGaugeSoc || !haveGaugeMv) && BoardConfig::PIN_BAT_ADC >= 0) {
    int raw = analogRead(BoardConfig::PIN_BAT_ADC);
    if (raw >= 0) {
      constexpr float kAdcRef = 3.3f;
      constexpr float kAdcMax = 4095.0f;
      constexpr float kDivider = 2.0f;
      float measured = (static_cast<float>(raw) / kAdcMax) * kAdcRef * kDivider;
      if (!haveGaugeMv) b.voltage = measured;
      if (!haveGaugeSoc) {
        float pct = (measured - 3.3f) / (4.2f - 3.3f) * 100.0f;
        b.percent = clampBatteryPercent(static_cast<int>(pct + (pct >= 0 ? 0.5f : -0.5f)));
      }
      usedAdcFallback = true;
    }
  }

  static uint32_t lastBatteryLogMs = 0;
  uint32_t now = millis();
  if (now - lastBatteryLogMs > 30000) {
    lastBatteryLogMs = now;
    Serial.printf("[BAT] gauge=%s charger=%s adcFallback=%s pct=%d volt=%.3f curr=%.1f charging=%s\n",
                  (haveGaugeSoc || haveGaugeMv) ? "yes" : "no",
                  haveCharger ? "yes" : "no",
                  usedAdcFallback ? "yes" : "no",
                  b.percent, b.voltage, b.currentMa,
                  b.charging ? "yes" : "no");
  }

  return b;
}

static uint8_t bcdToDec(uint8_t v) { return static_cast<uint8_t>(((v >> 4) * 10) + (v & 0x0F)); }

String BoardHAL::rtcTime() {
  if (!_rtcAvailable) return "";
  // PCF8563 time regs start at 0x02: sec, min, hour, day, weekday, month, year
  g_boardI2C.beginTransmission(BoardConfig::RTC_ADDR);
  g_boardI2C.write(static_cast<uint8_t>(0x02));
  if (g_boardI2C.endTransmission(false) != 0) return "";
  if (g_boardI2C.requestFrom(static_cast<int>(BoardConfig::RTC_ADDR), 7) != 7) return "";
  uint8_t secRaw = g_boardI2C.read();
  uint8_t minRaw = g_boardI2C.read();
  uint8_t hourRaw = g_boardI2C.read();
  uint8_t dayRaw = g_boardI2C.read();
  (void)g_boardI2C.read();
  uint8_t monthRaw = g_boardI2C.read();
  uint8_t yearRaw = g_boardI2C.read();

  uint8_t sec = bcdToDec(secRaw & 0x7F);
  uint8_t min = bcdToDec(minRaw & 0x7F);
  uint8_t hour = bcdToDec(hourRaw & 0x3F);
  uint8_t day = bcdToDec(dayRaw & 0x3F);
  uint8_t month = bcdToDec(monthRaw & 0x1F);
  uint8_t year = bcdToDec(yearRaw);
  char buf[32];
  snprintf(buf, sizeof(buf), "%02u-%02u-20%02u %02u:%02u:%02u", day, month, year, hour, min, sec);
  return String(buf);
}

void BoardHAL::sleepSeconds(uint32_t seconds) { esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL); esp_deep_sleep_start(); }
void BoardHAL::setLowlightMode(bool enabled) {
  ::setLowlightMode(_lowlight, enabled);
  applyBacklightState();
}
void BoardHAL::toggleBacklight() { toggleLowlightBacklight(_lowlight); applyBacklightState(); }
void BoardHAL::applyBacklightState() { digitalWrite(BoardConfig::PIN_BL_EN, shouldBacklightBeOn(_lowlight) ? HIGH : LOW); Serial.printf("[BACKLIGHT] lowlight=%s state=%s pin=%d\n", _lowlight.enabled ? "on" : "off", shouldBacklightBeOn(_lowlight) ? "on" : "off", BoardConfig::PIN_BL_EN); }
