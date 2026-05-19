#pragma once

#include <Arduino.h>
#include "PowerManagementLogic.h"

String buildPowerPolicyJson(const PowerPolicy& policy, bool configPresent);
