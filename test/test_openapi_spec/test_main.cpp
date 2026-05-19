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
  TEST_ASSERT_TRUE(spec.find("/api/meshtastic/stats") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("/api/power/policy") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("SD not mounted") != std::string::npos);
  TEST_ASSERT_TRUE(spec.find("cache context unavailable") != std::string::npos);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_openapi_contains_required_paths);
  return UNITY_END();
}
