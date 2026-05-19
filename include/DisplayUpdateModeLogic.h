#pragma once

#include <stdint.h>

inline int selectDisplayUpdateMode(bool fullRefreshRequested, int fullMode, int partialMode) {
  return fullRefreshRequested ? fullMode : partialMode;
}
