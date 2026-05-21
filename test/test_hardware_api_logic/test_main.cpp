#include <Arduino.h>
#include <unity.h>
#include "HardwareApiLogic.h"

void test_build_hardware_status_json_all_fields_present() {
  HardwareStatus hs;
  hs.flashOk = true;
  hs.psramOk = true;
  hs.sdMounted = false;
  hs.wifiReady = true;
  hs.bleReady = false;
  hs.loraReady = true;
  hs.gpsReady = true;
  hs.epdReady = true;
  hs.touchReady = false;
  hs.rtcReady = true;
  hs.tps65185Ready = true;
  hs.bq25896Ready = false;
  hs.bq27220Ready = true;
  hs.pca9535Ready = false;

  String j = buildHardwareStatusJson(hs);
  TEST_ASSERT_TRUE(j.indexOf("\"flash_ok\":true") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"psram_ok\":true") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"sd_mounted\":false") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"wifi_ready\":true") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"ble_ready\":false") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"lora_ready\":true") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"gps_ready\":true") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"epd_ready\":true") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"touch_ready\":false") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"rtc_ready\":true") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"tps65185_ready\":true") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"bq25896_ready\":false") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"bq27220_ready\":true") >= 0);
  TEST_ASSERT_TRUE(j.indexOf("\"pca9535_ready\":false") >= 0);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_build_hardware_status_json_all_fields_present);
  UNITY_END();
}

void loop() {}
