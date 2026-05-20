#include <unity.h>
#include <fstream>
#include <sstream>
#include <string>

std::string readFile(const char* path) {
  std::ifstream in(path);
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

void test_openapi_contains_required_paths() {
  std::string spec = readFile("docs/openapi.yaml");
  TEST_ASSERT_TRUE(spec.find("/api/health") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/status") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/apps/order") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/apps/install") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/apps/remove") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/apps/update") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/weather/cache") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/cache/stats") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/radio/scans") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/radio/control") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/meshtastic/stats") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/files/list") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/files/delete") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/maps/status") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/maps/prefetch") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/maps/cache") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/maps/cache/clear") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/gps/status") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/gps/dgps") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("Live raw and DGPS-aware GPS status snapshot") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("predicted correction/rate vectors") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("Invalid prefetch payload") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/gps/tracks") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/gps/tracks/clear") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/power/policy") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("SD not mounted") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("cache context unavailable") != std::string::npos);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_openapi_contains_required_paths);
  return UNITY_END();
}
