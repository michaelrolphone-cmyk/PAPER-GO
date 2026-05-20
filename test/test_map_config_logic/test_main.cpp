#include <unity.h>
#include "MapConfigLogic.h"

void test_parse_map_config_and_clamp() {
  String j = "{\"provider\":\"osm\",\"zoom\":30,\"minZoom\":2,\"maxZoom\":20,\"prefetchRadius\":9,\"offlineOnly\":false,\"tileSizePx\":0}";
  MapConfig c = parseMapConfig(j);
  TEST_ASSERT_TRUE(c.valid);
  TEST_ASSERT_EQUAL_STRING("osm", c.provider.c_str());
  TEST_ASSERT_EQUAL(20, c.zoom);
  TEST_ASSERT_EQUAL(5, c.prefetchRadius);
  TEST_ASSERT_EQUAL(256, c.tileSizePx);
}

int main(int argc, char** argv){UNITY_BEGIN(); RUN_TEST(test_parse_map_config_and_clamp); return UNITY_END();}
