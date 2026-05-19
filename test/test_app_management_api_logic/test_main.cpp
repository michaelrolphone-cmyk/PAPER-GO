#include <unity.h>
#include "AppManagementApiLogic.h"

void test_parse_request_and_validate_id() {
  AppManagementRequest req;
  TEST_ASSERT_TRUE(parseAppManagementRequest("{\"id\":\"weather-plus\",\"sourceUrl\":\"https://example.com/app.zip\",\"version\":\"1.0.2\"}", req));
  TEST_ASSERT_EQUAL_STRING("weather-plus", req.id.c_str());
  TEST_ASSERT_EQUAL_STRING("1.0.2", req.version.c_str());

  TEST_ASSERT_FALSE(parseAppManagementRequest("{\"id\":\"../../bad\"}", req));
  TEST_ASSERT_FALSE(isValidAppId(""));
}

void test_build_result_json() {
  String body = buildAppManagementResultJson("install", "weather-plus", true, "installed");
  TEST_ASSERT_TRUE(body.indexOf("\"action\":\"install\"") >= 0);
  TEST_ASSERT_TRUE(body.indexOf("\"id\":\"weather-plus\"") >= 0);
  TEST_ASSERT_TRUE(body.indexOf("\"ok\":true") >= 0);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_parse_request_and_validate_id);
  RUN_TEST(test_build_result_json);
  return UNITY_END();
}
