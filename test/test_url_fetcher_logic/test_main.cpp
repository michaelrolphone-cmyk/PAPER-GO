#include <unity.h>
#include "UrlFetcherLogic.h"

void test_parse_valid_config() {
  String raw = "{\"url\":\"https://example.com/api\",\"timeoutMs\":3000}";
  UrlFetcherConfig cfg = parseUrlFetcherConfig(raw);
  TEST_ASSERT_TRUE(cfg.valid);
  TEST_ASSERT_EQUAL_STRING("https://example.com/api", cfg.url.c_str());
  TEST_ASSERT_EQUAL(3000, cfg.timeoutMs);
}

void test_reject_non_http_scheme() {
  String raw = "{\"url\":\"ftp://example.com\"}";
  UrlFetcherConfig cfg = parseUrlFetcherConfig(raw);
  TEST_ASSERT_FALSE(cfg.valid);
}

void test_cache_path_deterministic() {
  String p1 = cachePathForUrl("https://example.com/a");
  String p2 = cachePathForUrl("https://example.com/a");
  String p3 = cachePathForUrl("https://example.com/b");
  TEST_ASSERT_EQUAL_STRING(p1.c_str(), p2.c_str());
  TEST_ASSERT_NOT_EQUAL(0, p1.compareTo(p3));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_parse_valid_config);
  RUN_TEST(test_reject_non_http_scheme);
  RUN_TEST(test_cache_path_deterministic);
  return UNITY_END();
}
