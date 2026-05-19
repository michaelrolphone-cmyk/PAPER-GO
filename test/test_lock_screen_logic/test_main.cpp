#include <unity.h>
#include "LockScreenLogic.h"

void test_tile_path_requires_valid_fix() {
  GpsFix f{};
  TEST_ASSERT_EQUAL_STRING("", lockScreenMapTilePath(f).c_str());
}

void test_tile_path_and_preview() {
  GpsFix f{};
  f.valid = true;
  f.lat = 37.7749;
  f.lon = -122.4194;
  String p = lockScreenMapTilePath(f);
  TEST_ASSERT_TRUE(p.startsWith("/cache/maps/default/12/"));
  TEST_ASSERT_TRUE(p.endsWith(".tile"));

  LockScreenPreviewInfo preview = buildLockScreenPreviewInfo(f, true);
  TEST_ASSERT_EQUAL(CacheCoverageState::FullyCached, preview.coverage);
  TEST_ASSERT_TRUE(preview.summary.indexOf("Location:") == 0);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_tile_path_requires_valid_fix);
  RUN_TEST(test_tile_path_and_preview);
  return UNITY_END();
}
