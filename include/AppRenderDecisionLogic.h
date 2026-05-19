#pragma once

#include "Types.h"

inline bool shouldRenderAfterInputEvent(TouchType touchType, bool homePressed, bool powerPressed) {
  return homePressed || powerPressed || touchType != TouchType::None;
}
