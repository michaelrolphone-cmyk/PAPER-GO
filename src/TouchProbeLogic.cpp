#include "TouchProbeLogic.h"

uint8_t selectGt911Address(bool probe14Ok, bool probe5dOk) {
  if (probe14Ok) return 0x14;
  if (probe5dOk) return 0x5D;
  return 0;
}

uint8_t probeGt911Address(bool (*probeFn)(uint8_t addr, void* ctx), void* ctx) {
  if (!probeFn) return 0;
  if (probeFn(0x14, ctx)) return 0x14;
  if (probeFn(0x5D, ctx)) return 0x5D;
  return 0;
}
