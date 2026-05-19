#include <Arduino.h>
#include <unity.h>

#include "BootLogLogic.h"
#include "ChessLogic.h"
#include "DisplayRenderLogic.h"
#include "DisplayRefreshLogic.h"
#include "GoLogic.h"
#include "MinesweeperLogic.h"
#include "TicTacToeLogic.h"
#include "WifiConfigLogic.h"

void test_forwarding_headers_compile_in_flat_include_mode() {
  TEST_ASSERT_TRUE(true);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_forwarding_headers_compile_in_flat_include_mode);
  UNITY_END();
}

void loop() {}
