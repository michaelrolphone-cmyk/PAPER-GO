#include <unity.h>
#include "MapCacheLogic.h"

void test_cache_coverage_states() {
  TEST_ASSERT_EQUAL(CacheCoverageState::Uncached, deriveCacheCoverageState(0, 0));
  TEST_ASSERT_EQUAL(CacheCoverageState::Uncached, deriveCacheCoverageState(4, 0));
  TEST_ASSERT_EQUAL(CacheCoverageState::PartiallyCached, deriveCacheCoverageState(4, 1));
  TEST_ASSERT_EQUAL(CacheCoverageState::FullyCached, deriveCacheCoverageState(4, 4));
  TEST_ASSERT_EQUAL(CacheCoverageState::FullyCached, deriveCacheCoverageState(4, 9));
}

void test_cache_coverage_labels() {
  TEST_ASSERT_EQUAL_STRING("fully cached", cacheCoverageLabel(CacheCoverageState::FullyCached));
  TEST_ASSERT_EQUAL_STRING("partially cached", cacheCoverageLabel(CacheCoverageState::PartiallyCached));
  TEST_ASSERT_EQUAL_STRING("uncached", cacheCoverageLabel(CacheCoverageState::Uncached));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_cache_coverage_states);
  RUN_TEST(test_cache_coverage_labels);
  return UNITY_END();
}
