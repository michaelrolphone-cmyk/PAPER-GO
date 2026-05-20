#include <unity.h>
#include "TouchProbeLogic.h"

void test_prefers_0x14_when_both_present() {
  TEST_ASSERT_EQUAL_HEX8(0x14, selectGt911Address(true, true));
}

void test_falls_back_to_0x5d() {
  TEST_ASSERT_EQUAL_HEX8(0x5D, selectGt911Address(false, true));
}

void test_returns_zero_when_not_found() {
  TEST_ASSERT_EQUAL_HEX8(0x00, selectGt911Address(false, false));
}

struct ProbeState {
  bool has14;
  bool has5d;
  uint8_t calls[2];
  uint8_t callCount;
};

bool fakeProbe(uint8_t addr, void* ctx) {
  ProbeState* state = static_cast<ProbeState*>(ctx);
  if (state->callCount < 2) state->calls[state->callCount] = addr;
  state->callCount++;
  if (addr == 0x14) return state->has14;
  if (addr == 0x5D) return state->has5d;
  return false;
}

void test_probe_sequence_checks_0x14_first_then_0x5d() {
  ProbeState state{false, true, {0, 0}, 0};
  uint8_t addr = probeGt911Address(fakeProbe, &state);
  TEST_ASSERT_EQUAL_HEX8(0x5D, addr);
  TEST_ASSERT_EQUAL_UINT8(2, state.callCount);
  TEST_ASSERT_EQUAL_HEX8(0x14, state.calls[0]);
  TEST_ASSERT_EQUAL_HEX8(0x5D, state.calls[1]);
}

void test_probe_stops_after_0x14_hit() {
  ProbeState state{true, true, {0, 0}, 0};
  uint8_t addr = probeGt911Address(fakeProbe, &state);
  TEST_ASSERT_EQUAL_HEX8(0x14, addr);
  TEST_ASSERT_EQUAL_UINT8(1, state.callCount);
  TEST_ASSERT_EQUAL_HEX8(0x14, state.calls[0]);
}

void test_probe_null_function_returns_zero() {
  TEST_ASSERT_EQUAL_HEX8(0x00, probeGt911Address(nullptr, nullptr));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_prefers_0x14_when_both_present);
  RUN_TEST(test_falls_back_to_0x5d);
  RUN_TEST(test_returns_zero_when_not_found);
  RUN_TEST(test_probe_sequence_checks_0x14_first_then_0x5d);
  RUN_TEST(test_probe_stops_after_0x14_hit);
  RUN_TEST(test_probe_null_function_returns_zero);
  return UNITY_END();
}
