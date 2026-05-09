#pragma once
#include <vector>
#include "Types.h"

GpsFix computeBestFitFix(const std::vector<GpsFix>& fixes, double maxDeltaDeg);
