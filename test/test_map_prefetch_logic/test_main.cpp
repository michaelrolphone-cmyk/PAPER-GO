#include <unity.h>
#include "MapPrefetchLogic.h"

void test_parse_prefetch_valid_and_clamped() {
  MapPrefetchRequest r = parseMapPrefetchRequest("{\"lat\":1.23,\"lon\":4.56,\"zoom\":99,\"radius\":99}");
  TEST_ASSERT_TRUE(r.valid);
  TEST_ASSERT_EQUAL(22, r.zoom);
  TEST_ASSERT_EQUAL(8, r.radius);
}

void test_parse_prefetch_invalid_payload() {
  MapPrefetchRequest r = parseMapPrefetchRequest("{}");
  TEST_ASSERT_FALSE(r.valid);
}

int main(int argc, char** argv){UNITY_BEGIN(); RUN_TEST(test_parse_prefetch_valid_and_clamped); RUN_TEST(test_parse_prefetch_invalid_payload); return UNITY_END();}
