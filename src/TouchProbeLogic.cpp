#include "TouchProbeLogic.h"

uint8_t selectGt911Address(bool probe5dOk, bool probe14Ok) {
  if (probe5dOk) return 0x5D;
  if (probe14Ok) return 0x14;
  return 0;
}
