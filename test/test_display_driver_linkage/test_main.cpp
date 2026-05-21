#include <unity.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>

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


void test_render_lcd_uses_relative_board_header_for_embedded_arduino_builds() {
  const std::string renderLcd = readFile("lib/epdiy/src/output_lcd/render_lcd.c");
  TEST_ASSERT_TRUE(renderLcd.find("#include \"../epd_board.h\"") != std::string::npos);
}


void test_embedded_epdiy_sources_use_self_resolving_local_includes() {
  const std::vector<std::string> embeddedSources = {
      "lib/epdiy/src/epdiy.c",
      "lib/epdiy/src/render.c",
      "lib/epdiy/src/output_lcd/render_lcd.c",
      "lib/epdiy/src/output_lcd/lcd_driver.c",
      "lib/epdiy/src/output_i2s/render_i2s.c",
      "lib/epdiy/src/output_i2s/rmt_pulse.c",
      "lib/epdiy/src/output_i2s/i2s_data_bus.c",
      "lib/epdiy/src/output_common/lut.c",
      "lib/epdiy/src/output_common/line_queue.c",
      "lib/epdiy/src/output_common/render_context.c",
      "lib/epdiy/src/font.c",
      "lib/epdiy/src/displays.c",
      "lib/epdiy/src/board_specific.c",
      "lib/epdiy/src/builtin_waveforms.c",
      "lib/epdiy/src/highlevel.c",
      "lib/epdiy/src/board/tps65185.c",
      "lib/epdiy/src/board/pca9555.c",
      "lib/epdiy/src/board/epd_board.c",
      "lib/epdiy/src/board/epd_board_common.c",
      "lib/epdiy/src/board/epd_board_v2_v3.c",
      "lib/epdiy/src/board/epd_board_v4.c",
      "lib/epdiy/src/board/epd_board_v5.c",
      "lib/epdiy/src/board/epd_board_v6.c",
      "lib/epdiy/src/board/epd_board_v7.c",
  };

  const std::regex includePattern(R"(^\s*#include\s+"([^"]+)")", std::regex::multiline);

  for (const auto& sourcePath : embeddedSources) {
    const std::string source = readFile(sourcePath.c_str());
    const std::filesystem::path sourceDir = std::filesystem::path(sourcePath).parent_path();

    for (std::sregex_iterator it(source.begin(), source.end(), includePattern), end; it != end; ++it) {
      const std::string includePath = (*it)[1].str();
      if (includePath.rfind("../", 0) == 0 || includePath.rfind("./", 0) == 0 || includePath.find('/') != std::string::npos) {
        continue;
      }
      const std::filesystem::path localHeader = sourceDir / includePath;
      TEST_ASSERT_TRUE_MESSAGE(std::filesystem::exists(localHeader), (sourcePath + " has non-resolving local include: " + includePath).c_str());
    }
  }
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


void test_repo_excludes_lilygo_epd47_references_from_build_paths() {
  const std::string boardHal = readFile("src/BoardHAL.cpp");
  TEST_ASSERT_TRUE(boardHal.find("epd_powerdown_lilygo_t5_47") == std::string::npos);
  TEST_ASSERT_TRUE(boardHal.find("ARDUINO_T5_E_PAPER_S3_V7") == std::string::npos);

}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_board_hal_includes_epdiy_header);
  RUN_TEST(test_arduino_build_embeds_epdiy_sources);
  RUN_TEST(test_render_lcd_uses_relative_board_header_for_embedded_arduino_builds);
  RUN_TEST(test_embedded_epdiy_sources_use_self_resolving_local_includes);
  RUN_TEST(test_include_and_root_epdiy_shims_exist_for_arduino_builds);
  RUN_TEST(test_repo_excludes_lilygo_epd47_references_from_build_paths);
  return UNITY_END();
}
