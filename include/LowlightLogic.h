#pragma once

struct LowlightState {
  bool enabled = false;
  bool backlightOn = false;
};

inline bool shouldBacklightBeOn(const LowlightState& state) {
  return state.backlightOn;
}

inline void toggleLowlightBacklight(LowlightState& state) {
  state.backlightOn = !state.backlightOn;
}

inline void setLowlightMode(LowlightState& state, bool enabled) {
  state.enabled = enabled;
}
