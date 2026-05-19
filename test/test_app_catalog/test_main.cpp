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


void test_offline_mode_support_matrix() {
  TEST_ASSERT_TRUE(appSupportsOfflineMode("gpsmap"));
  TEST_ASSERT_TRUE(appSupportsOfflineMode("files"));
  TEST_ASSERT_FALSE(appSupportsOfflineMode("url"));
  TEST_ASSERT_FALSE(appSupportsOfflineMode("weather"));
}

void test_parse_invalid_json_returns_empty() {
  auto ids = parseOrderedAppIds("not-json");
  TEST_ASSERT_EQUAL(0, ids.size());
}

void test_serialize_then_parse_round_trip_preserves_order() {
  std::vector<String> ids = {"weather", "gpsmap", "settings", "radio"};
  String json = serializeOrderedAppIds(ids);
  auto parsed = parseOrderedAppIds(json);
  TEST_ASSERT_TRUE(parsed.size() >= ids.size());
  for (size_t i = 0; i < ids.size(); ++i) {
    TEST_ASSERT_EQUAL_STRING(ids[i].c_str(), parsed[i].c_str());
  }
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_parse_order_filters_unknown_and_duplicates);
  RUN_TEST(test_offline_mode_support_matrix);
  RUN_TEST(test_parse_invalid_json_returns_empty);
  RUN_TEST(test_serialize_then_parse_round_trip_preserves_order);
  return UNITY_END();
}
