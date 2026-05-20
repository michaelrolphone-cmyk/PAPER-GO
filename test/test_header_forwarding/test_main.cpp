#include <Arduino.h>
#include <unity.h>
#include "BatteryGaugeLogic.h"
#include "NetworkConnectLogic.h"
#include "RtcProbeLogic.h"
#include "TouchInputLogic.h"
#include "TouchProbeLogic.h"

void test_forwarding_headers_compile() { TEST_ASSERT_TRUE(true); }

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_forwarding_headers_compile);
  UNITY_END();
}

void loop() {}

