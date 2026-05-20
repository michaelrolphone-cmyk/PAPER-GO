#include <unity.h>

#include <fstream>
#include <sstream>
#include <string>

static std::string loadBoardHalSource() {
  std::ifstream in("src/BoardHAL.cpp");
  TEST_ASSERT_TRUE_MESSAGE(in.good(), "Unable to open src/BoardHAL.cpp");
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

void test_board_hal_uses_dedicated_i2c_controller_instance() {
  const std::string source = loadBoardHalSource();
  TEST_ASSERT_NOT_EQUAL(std::string::npos, source.find("TwoWire g_boardI2C(1);"));
}

void test_board_hal_does_not_call_default_wire_directly() {
  const std::string source = loadBoardHalSource();
  TEST_ASSERT_EQUAL(std::string::npos, source.find("Wire.begin("));
  TEST_ASSERT_EQUAL(std::string::npos, source.find("Wire.beginTransmission("));
  TEST_ASSERT_EQUAL(std::string::npos, source.find("Wire.requestFrom("));
}

void test_board_hal_logs_i2c_app_bus_startup() {
  const std::string source = loadBoardHalSource();
  TEST_ASSERT_NOT_EQUAL(std::string::npos, source.find("I2C app bus: starting on controller 1"));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_board_hal_uses_dedicated_i2c_controller_instance);
  RUN_TEST(test_board_hal_does_not_call_default_wire_directly);
  RUN_TEST(test_board_hal_logs_i2c_app_bus_startup);
  return UNITY_END();
}
