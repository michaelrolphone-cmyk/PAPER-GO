#include "BoardHAL.h"
#include "BoardConfig.h"

bool BoardHAL::begin() {
  Serial.begin(115200);
  delay(250);
  Wire.begin(BoardConfig::I2C_SDA, BoardConfig::I2C_SCL);
  beginSD();
  Serial.println("T5 Field OS HAL online");
  return true;
}

bool BoardHAL::beginSD() {
  _sdMounted = SD.begin(BoardConfig::PIN_SD_CS);
  Serial.printf("SD mount: %s\n", _sdMounted ? "ok" : "failed");
  return _sdMounted;
}

void BoardHAL::beginFrame() { Serial.println("[DISPLAY] begin frame"); }
void BoardHAL::endFrame(bool fullRefresh) { Serial.printf("[DISPLAY] end frame refresh=%s\n", fullRefresh ? "full" : "partial"); }
void BoardHAL::clear(uint8_t gray) { Serial.printf("[DISPLAY] clear gray=%u\n", gray); }
void BoardHAL::fillRect(int x,int y,int w,int h,uint8_t gray) { Serial.printf("[RECTF] %d,%d %dx%d g%u\n",x,y,w,h,gray); }
void BoardHAL::drawRect(int x,int y,int w,int h,uint8_t gray) { Serial.printf("[RECT] %d,%d %dx%d g%u\n",x,y,w,h,gray); }
void BoardHAL::drawText(int x,int y,const String& text,uint8_t gray,uint8_t size) { Serial.printf("[TEXT] %d,%d g%u s%u %s\n",x,y,gray,size,text.c_str()); }
void BoardHAL::drawLine(int x1,int y1,int x2,int y2,uint8_t gray) { Serial.printf("[LINE] %d,%d -> %d,%d g%u\n",x1,y1,x2,y2,gray); }
TouchEvent BoardHAL::pollTouch() { return {}; }

BatteryStatus BoardHAL::battery() {
  BatteryStatus b;
  b.percent = -1;
  b.charging = false;
  // Wire real BQ27220/BQ25896 reads here.
  return b;
}

void BoardHAL::sleepSeconds(uint32_t seconds) {
  esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
  esp_deep_sleep_start();
}
