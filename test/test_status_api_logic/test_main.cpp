#include <unity.h>
#include "StatusApiLogic.h"

void test_build_status_json_contains_fields() {
  NetStatus net; net.wifi = true; net.ip.fromString("192.168.1.55");
  GpsFix gps; gps.valid = true; gps.lat = 1.2; gps.lon = 3.4;
  BatteryStatus b; b.percent = 80; b.charging = true;
  String json = buildStatusApiJson(net, gps, b, true, false, true, true);
  TEST_ASSERT_TRUE(json.indexOf("\"wifi\":true") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"ip\":\"192.168.1.55\"") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"batteryPercent\":80") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"sdMounted\":true") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"unreadMessages\":true") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"cacheActivity\":true") >= 0);
}

void test_unread_and_cache_indicator_helpers() {
  TEST_ASSERT_TRUE(hasUnreadMeshtasticMessages(1));
  TEST_ASSERT_FALSE(hasUnreadMeshtasticMessages(0));

  TEST_ASSERT_TRUE(hasRecentCacheActivity(1000, 2000, 2000));
  TEST_ASSERT_FALSE(hasRecentCacheActivity(1000, 4001, 3000));
  TEST_ASSERT_FALSE(hasRecentCacheActivity(0, 4001, 3000));
  TEST_ASSERT_FALSE(hasRecentCacheActivity(5000, 4001, 3000));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_build_status_json_contains_fields);
  RUN_TEST(test_unread_and_cache_indicator_helpers);
  return UNITY_END();
}
