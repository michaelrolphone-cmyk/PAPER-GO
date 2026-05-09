#include <unity.h>
#include "GpsBestFitLogic.h"

void test_best_fit_rejects_outlier() {
  GpsFix a; a.valid=true; a.lat=10.0; a.lon=20.0; a.hdop=1;
  GpsFix b; b.valid=true; b.lat=10.001; b.lon=20.001; b.hdop=2;
  GpsFix out; out.valid=true; out.lat=50.0; out.lon=80.0; out.hdop=9;
  std::vector<GpsFix> v{a,b,out};
  GpsFix r = computeBestFitFix(v, 0.02);
  TEST_ASSERT_TRUE(r.valid);
  TEST_ASSERT_FLOAT_WITHIN(0.01, 10.0005, r.lat);
  TEST_ASSERT_FLOAT_WITHIN(0.01, 20.0005, r.lon);
}

void test_best_fit_empty_invalid() {
  std::vector<GpsFix> v;
  GpsFix r = computeBestFitFix(v, 0.02);
  TEST_ASSERT_FALSE(r.valid);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_best_fit_rejects_outlier);
  RUN_TEST(test_best_fit_empty_invalid);
  return UNITY_END();
}
