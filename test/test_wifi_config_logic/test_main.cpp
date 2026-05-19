#include <Arduino.h>
#include <unity.h>
#include "WifiConfigLogic.h"

void test_parse_wifi_config_valid() {
  WifiConfig cfg = parseWifiConfig("{\"ssid\":\"FieldNet\",\"password\":\"s3cret\"}");
  TEST_ASSERT_TRUE(cfg.valid);
  TEST_ASSERT_EQUAL_STRING("FieldNet", cfg.ssid.c_str());
  TEST_ASSERT_EQUAL_STRING("s3cret", cfg.password.c_str());
}

void test_parse_wifi_config_invalid_without_ssid() {
  WifiConfig cfg = parseWifiConfig("{\"password\":\"s3cret\"}");
  TEST_ASSERT_FALSE(cfg.valid);
}

void test_masked_password() {
  TEST_ASSERT_EQUAL_STRING("******", maskedPassword("secret").c_str());
  TEST_ASSERT_EQUAL_STRING("", maskedPassword("").c_str());
}

void test_build_wifi_config_json() {
  String json = buildWifiConfigJson("MyWiFi", "pass1234");
  TEST_ASSERT_TRUE(json.indexOf("\"ssid\":\"MyWiFi\"") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"password\":\"pass1234\"") >= 0);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_parse_wifi_config_valid);
  RUN_TEST(test_parse_wifi_config_invalid_without_ssid);
  RUN_TEST(test_masked_password);
  RUN_TEST(test_build_wifi_config_json);
  UNITY_END();
}

void loop() {}
