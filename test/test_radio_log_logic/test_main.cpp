#include <unity.h>
#include "RadioLogLogic.h"

void test_format_line_with_gps() {
  RadioSignal s;
  s.kind = "WiFi";
  s.name = "test";
  s.address = "aa:bb";
  s.rssi = -55;
  s.channel = 6;
  s.protocol = "secured";

  GpsFix fix;
  fix.valid = true;
  fix.lat = 12.345678;
  fix.lon = -45.678901;

  String line = formatRadioScanLogLine(s, 1234, &fix);
  TEST_ASSERT_TRUE(line.indexOf("kind=WiFi") >= 0);
  TEST_ASSERT_TRUE(line.indexOf("lat=12.345678") >= 0);
  TEST_ASSERT_TRUE(line.indexOf("lon=-45.678901") >= 0);
}

void test_build_log_line_count() {
  RadioSignal s1; s1.kind = "BLE";
  RadioSignal s2; s2.kind = "LoRa";
  std::vector<RadioSignal> signals = {s1, s2};
  String body = buildRadioScanLog(signals, 1, nullptr);
  int lines = 0;
  for (size_t i=0; i<body.length(); ++i) if (body[i] == '\n') lines++;
  TEST_ASSERT_EQUAL(2, lines);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_format_line_with_gps);
  RUN_TEST(test_build_log_line_count);
  return UNITY_END();
}
