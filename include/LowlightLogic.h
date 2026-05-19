#pragma once

struct LowlightState {
  bool enabled = false;
  bool backlightOn = false;
};

inline bool shouldBacklightBeOn(const LowlightState& state) {
  return !state.enabled || state.backlightOn;
}

inline void toggleLowlightBacklight(LowlightState& state) {
  if (!state.enabled) return;
  state.backlightOn = !state.backlightOn;
}

inline void setLowlightMode(LowlightState& state, bool enabled) {
  state.enabled = enabled;
}
