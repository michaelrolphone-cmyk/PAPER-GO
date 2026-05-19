#include <unity.h>
#include "MapTileLogic.h"

void test_invalid_fix_returns_invalid_tile() {
  GpsFix fix{};
  fix.valid = false;
  MapTileCoord tile = mapTileFromFix(fix, 12);
  TEST_ASSERT_FALSE(tile.valid);
  TEST_ASSERT_EQUAL_STRING("n/a", mapTileLabel(tile).c_str());
}

void test_valid_fix_maps_to_web_mercator_tile() {
  GpsFix fix{};
  fix.valid = true;
  fix.lat = 37.7749;
  fix.lon = -122.4194;
  MapTileCoord tile = mapTileFromFix(fix, 12);
  TEST_ASSERT_TRUE(tile.valid);
  TEST_ASSERT_EQUAL_UINT8(12, tile.zoom);
  TEST_ASSERT_TRUE(tile.x > 0);
  TEST_ASSERT_TRUE(tile.y > 0);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_invalid_fix_returns_invalid_tile);
  RUN_TEST(test_valid_fix_maps_to_web_mercator_tile);
  return UNITY_END();
}
