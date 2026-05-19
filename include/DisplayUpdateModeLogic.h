#pragma once

#include <stdint.h>

inline int selectDisplayUpdateMode(bool fullRefreshRequested, int fullMode, int partialMode) {
  (void)partialMode;
  (void)fullRefreshRequested;
  // During display bring-up, prefer a guaranteed full grayscale update mode.
  return fullMode;
}
