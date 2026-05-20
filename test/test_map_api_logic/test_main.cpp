#include <unity.h>
#include "MapApiLogic.h"

void test_build_maps_status_json() {
  MapConfig c; c.provider="default"; c.zoom=12; c.offlineOnly=true;
  MapTileCoord t; t.valid=true; t.zoom=12; t.x=655; t.y=1583;
  String j = buildMapsStatusJson(c,t,CacheCoverageState::PartiallyCached,10,2,true,"DGPS_GOOD");
  TEST_ASSERT_TRUE(j.indexOf("\"provider\":\"default\"")>=0);
  TEST_ASSERT_TRUE(j.indexOf("\"coverage\":\"partially cached\"")>=0);
  TEST_ASSERT_TRUE(j.indexOf("\"dgpsMode\":\"DGPS\"")>=0);
  TEST_ASSERT_TRUE(j.indexOf("\"dgpsQuality\":\"DGPS_GOOD\"")>=0);
}

void test_build_maps_cache_clear_json() {
  String j = buildMapsCacheClearJson(true, "default", 42);
  TEST_ASSERT_TRUE(j.indexOf("\"ok\":true")>=0);
  TEST_ASSERT_TRUE(j.indexOf("\"provider\":\"default\"")>=0);
  TEST_ASSERT_TRUE(j.indexOf("\"filesRemoved\":42")>=0);
}

int main(int argc, char** argv){UNITY_BEGIN(); RUN_TEST(test_build_maps_status_json); RUN_TEST(test_build_maps_cache_clear_json); return UNITY_END();}
