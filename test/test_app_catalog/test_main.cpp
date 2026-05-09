#include <unity.h>
#include "AppCatalog.h"

void test_parse_order_filters_unknown_and_duplicates() {
  String json = "{\"order\":[\"games\",\"url\",\"games\",\"invalid\"]}";
  auto ids = parseOrderedAppIds(json);
  TEST_ASSERT_TRUE(ids.size() >= 10);
  TEST_ASSERT_EQUAL_STRING("games", ids[0].c_str());
  TEST_ASSERT_EQUAL_STRING("url", ids[1].c_str());

  int gamesCount = 0;
  for (const auto& id : ids) {
    if (id == "games") gamesCount++;
  }
  TEST_ASSERT_EQUAL(1, gamesCount);
}

void test_parse_invalid_json_returns_empty() {
  auto ids = parseOrderedAppIds("not-json");
  TEST_ASSERT_EQUAL(0, ids.size());
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_parse_order_filters_unknown_and_duplicates);
  RUN_TEST(test_parse_invalid_json_returns_empty);
  return UNITY_END();
}
