#include "DisplayRefreshLogic.h"

bool shouldForceFullRefresh(const DisplayRefreshState& state, uint8_t maxPartialsBeforeFull) {
  if (maxPartialsBeforeFull == 0) return true;
  return state.partialSinceFull >= maxPartialsBeforeFull;
}

void recordDisplayRefresh(DisplayRefreshState& state, bool usedFullRefresh) {
  if (usedFullRefresh) {
    state.partialSinceFull = 0;
    return;
  }
  if (state.partialSinceFull < 255) state.partialSinceFull++;
}


bool shouldRenderFrame(const RenderDecisionInput& input) {
  return input.forceFullRefresh || input.statusBarChanged || input.appRequestedRender;
}
