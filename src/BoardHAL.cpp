#include "BoardHAL.h"
#include "BoardConfig.h"
#include "DisplayRenderLogic.h"
#include <SPI.h>

#if __has_include(<epdiy.h>)
#include <epdiy.h>
#define PAPERGO_HAS_EPDIY 1
#else
#define PAPERGO_HAS_EPDIY 0
#endif

namespace {
#if PAPERGO_HAS_EPDIY
constexpr const EpdWaveform* kWaveform = EPD_BUILTIN_WAVEFORM;
EpdiyHighlevelState g_hl;
bool g_displayReady = false;

inline void setPixel4bpp(int x, int y, uint8_t gray) {
  if (!g_displayReady) return;
  if (x < 0 || y < 0 || x >= BoardConfig::SCREEN_W || y >= BoardConfig::SCREEN_H) return;
  uint8_t* fb = epd_hl_get_framebuffer(&g_hl);
  if (!fb) return;
  const int idx = y * BoardConfig::SCREEN_W + x;
  const int byteIndex = idx >> 1;
  if ((idx & 1) == 0) fb[byteIndex] = (fb[byteIndex] & 0x0F) | (gray << 4);
  else fb[byteIndex] = (fb[byteIndex] & 0xF0) | (gray & 0x0F);
}
#endif
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
  _lowlight.enabled = false;
  _lowlight.backlightOn = true;
  applyBacklightState();
  beginSD();
#if PAPERGO_HAS_EPDIY
  epd_init(&epd_board_v7, &ED047TC1, EPD_LUT_64K);
  g_hl = epd_hl_init(kWaveform);
  epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);
  epd_set_lcd_pixel_clock_MHz(17);
  epd_poweron();
  epd_clear();
  epd_poweroff();
  g_displayReady = true;
  Serial.println("EPD init: ok");
#else
  Serial.println("EPD init: skipped (epdiy.h unavailable)");
#endif
  Serial.println("T5 Field OS HAL online");
  return true;
}

bool BoardHAL::beginSD() {
  _sdMounted = SD.begin(BoardConfig::PIN_SD_CS);
  Serial.printf("SD mount: %s\n", _sdMounted ? "ok" : "failed");
  return _sdMounted;
}

void BoardHAL::beginFrame() {}
void BoardHAL::endFrame(bool fullRefresh) {
#if PAPERGO_HAS_EPDIY
  if (!g_displayReady) return;
  epd_poweron();
  epd_hl_update_screen(&g_hl, fullRefresh ? MODE_GC16 : MODE_GL16, epd_ambient_temperature());
  epd_poweroff();

#endif
}

void BoardHAL::clear(uint8_t gray) {
  uint8_t safeGray = clampGrayLevel(gray);
#if PAPERGO_HAS_EPDIY
  for (int y = 0; y < BoardConfig::SCREEN_H; ++y) for (int x = 0; x < BoardConfig::SCREEN_W; ++x) setPixel4bpp(x,y,safeGray);

#endif
}

void BoardHAL::fillRect(int x,int y,int w,int h,uint8_t gray) {
  RenderRect rect{x,y,w,h};
  if (!clipRectToDisplay(rect)) return;
  uint8_t safeGray = clampGrayLevel(gray);
#if PAPERGO_HAS_EPDIY
  for (int yy = rect.y; yy < rect.y + rect.h; ++yy) for (int xx = rect.x; xx < rect.x + rect.w; ++xx) setPixel4bpp(xx,yy,safeGray);

#endif
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
  if (!clipLineToDisplay(x1, y1, x2, y2)) return;
  uint8_t safeGray = clampGrayLevel(gray);
  int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
  int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
  int err = dx + dy;
  while (true) {
#if PAPERGO_HAS_EPDIY
    setPixel4bpp(x1, y1, safeGray);
#endif
    if (x1 == x2 && y1 == y2) break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x1 += sx; }
    if (e2 <= dx) { err += dx; y1 += sy; }
  }
}
// rest unchanged
TouchEvent BoardHAL::pollTouch() {
  if (_touchTwoPoint) return _touchClassifier.updateTwoPoint(_touching, _touchX, _touchY, _touchX2, _touchY2, millis());
  return _touchClassifier.update(_touching, _touchX, _touchY, millis());
}
bool BoardHAL::pollHomeButtonPressed() { bool pressed = _homeButtonOverride ? _homeButtonPressed : (BoardConfig::PIN_HOME_BUTTON >= 0 && digitalRead(BoardConfig::PIN_HOME_BUTTON) == LOW); bool risingEdge = pressed && !_homeButtonLast; _homeButtonLast = pressed; return risingEdge; }
bool BoardHAL::pollPowerButtonPressed() { bool pressed = _powerButtonOverride ? _powerButtonPressed : (BoardConfig::PIN_PWR_BUTTON >= 0 && digitalRead(BoardConfig::PIN_PWR_BUTTON) == LOW); bool risingEdge = pressed && !_powerButtonLast; _powerButtonLast = pressed; return risingEdge; }
void BoardHAL::setTouchSample(bool touching, int16_t x, int16_t y) { _touchTwoPoint = false; _touching = touching; _touchX = x; _touchY = y; }
void BoardHAL::setHomeButtonSample(bool pressed) { _homeButtonOverride = true; _homeButtonPressed = pressed; }
void BoardHAL::setPowerButtonSample(bool pressed) { _powerButtonOverride = true; _powerButtonPressed = pressed; }
void BoardHAL::setTouchSampleTwoPoint(bool touching, int16_t x1, int16_t y1, int16_t x2, int16_t y2) { _touchTwoPoint = true; _touching = touching; _touchX = x1; _touchY = y1; _touchX2 = x2; _touchY2 = y2; }
BatteryStatus BoardHAL::battery() { BatteryStatus b; b.percent = -1; b.charging = false; return b; }
void BoardHAL::sleepSeconds(uint32_t seconds) { esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL); esp_deep_sleep_start(); }
void BoardHAL::setLowlightMode(bool enabled) { _lowlight.enabled = enabled; if (!enabled) _lowlight.backlightOn = true; applyBacklightState(); }
void BoardHAL::toggleBacklight() { toggleLowlightBacklight(_lowlight); applyBacklightState(); }
void BoardHAL::applyBacklightState() { digitalWrite(BoardConfig::PIN_BL_EN, shouldBacklightBeOn(_lowlight) ? HIGH : LOW); Serial.printf("[BACKLIGHT] lowlight=%s state=%s pin=%d\n", _lowlight.enabled ? "on" : "off", shouldBacklightBeOn(_lowlight) ? "on" : "off", BoardConfig::PIN_BL_EN); }
