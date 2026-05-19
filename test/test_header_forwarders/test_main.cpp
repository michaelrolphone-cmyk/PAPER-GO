#include <Arduino.h>
#include <unity.h>

#include "BootLogLogic.h"
#include "AppRenderDecisionLogic.h"
#include "DisplayFramebufferClipLogic.h"
#include "DisplayPixelPackingLogic.h"
#include "DisplayUpdateModeLogic.h"
#include "ScaffoldNoticeLogic.h"
#include "ChessLogic.h"
#include "DisplayRenderLogic.h"
#include "DisplayRefreshLogic.h"
#include "GoLogic.h"
#include "GpsConfigLogic.h"
#include "GpsLogLogic.h"
#include "GpsTrackApiLogic.h"
#include "LowlightLogic.h"
#include "PowerApiLogic.h"
#include "PowerManagementLogic.h"
#include "MinesweeperLogic.h"
#include "MapTileLogic.h"
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
