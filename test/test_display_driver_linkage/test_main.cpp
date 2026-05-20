#include <unity.h>
#include <fstream>
#include <sstream>
#include <string>

static std::string readFile(const char* path) {
  std::ifstream in(path);
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

void test_board_hal_includes_epdiy_header() {
  const std::string boardHal = readFile("src/BoardHAL.cpp");
  TEST_ASSERT_TRUE(boardHal.find("#include <epdiy.h>") != std::string::npos);
}


void test_arduino_build_embeds_epdiy_sources() {
  const std::string embedded = readFile("src/epdiy_arduino_linker.cpp");
  TEST_ASSERT_TRUE(embedded.find("#if defined(ARDUINO)") != std::string::npos);
  TEST_ASSERT_TRUE(embedded.find("epd_board_v7.c") != std::string::npos);
  TEST_ASSERT_TRUE(embedded.find("highlevel.c") != std::string::npos);
}

void test_include_and_root_epdiy_shims_exist_for_arduino_builds() {
  const std::string includeShim = readFile("include/epdiy.h");
  const std::string rootShim = readFile("epdiy.h");
  TEST_ASSERT_TRUE(includeShim.find("../lib/epdiy/src/epdiy.h") != std::string::npos);
  TEST_ASSERT_TRUE(rootShim.find("include/epdiy.h") != std::string::npos);

  std::ifstream includeHdr("include/epdiy.h");
  std::ifstream rootHdr("epdiy.h");
  std::ifstream vendorHdr("lib/epdiy/src/epdiy.h");
  TEST_ASSERT_TRUE(includeHdr.good());
  TEST_ASSERT_TRUE(rootHdr.good());
  TEST_ASSERT_TRUE(vendorHdr.good());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_board_hal_includes_epdiy_header);
  RUN_TEST(test_arduino_build_embeds_epdiy_sources);
  RUN_TEST(test_include_and_root_epdiy_shims_exist_for_arduino_builds);
  return UNITY_END();
}
