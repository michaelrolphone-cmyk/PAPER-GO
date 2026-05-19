#pragma once
#ifndef PAPER_GO_DISPLAY_REFRESH_LOGIC_H
#define PAPER_GO_DISPLAY_REFRESH_LOGIC_H

#include <stdint.h>

struct DisplayRefreshState {
  uint8_t partialSinceFull = 0;
};

struct RenderDecisionInput {
  bool forceFullRefresh = false;
  bool statusBarChanged = false;
  bool appRequestedRender = false;
};

bool shouldForceFullRefresh(const DisplayRefreshState& state, uint8_t maxPartialsBeforeFull);
void recordDisplayRefresh(DisplayRefreshState& state, bool usedFullRefresh);
bool shouldRenderFrame(const RenderDecisionInput& input);

#endif // PAPER_GO_DISPLAY_REFRESH_LOGIC_H
