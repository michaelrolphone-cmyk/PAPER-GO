#include <unity.h>
#include "WebUtils.h"

void test_mime_type_resolution() {
  TEST_ASSERT_EQUAL_STRING("text/html", mimeTypeForPath("/webroot/index.html").c_str());
  TEST_ASSERT_EQUAL_STRING("application/json", mimeTypeForPath("/webroot/apps.json").c_str());
  TEST_ASSERT_EQUAL_STRING("application/octet-stream", mimeTypeForPath("/webroot/blob.bin").c_str());
}

void test_uri_safety_and_mapping() {
  TEST_ASSERT_TRUE(isSafeWebUri("/index.html"));
  TEST_ASSERT_FALSE(isSafeWebUri("../etc/passwd"));
  TEST_ASSERT_FALSE(isSafeWebUri("/../../secret"));
  TEST_ASSERT_EQUAL_STRING("/webroot/index.html", mapUriToWebrootPath("/").c_str());
  TEST_ASSERT_EQUAL_STRING("/webroot/assets/a.css", mapUriToWebrootPath("/assets/a.css").c_str());
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_mime_type_resolution);
  RUN_TEST(test_uri_safety_and_mapping);
  return UNITY_END();
}
