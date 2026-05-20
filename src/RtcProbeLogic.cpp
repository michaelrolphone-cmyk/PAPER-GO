#include "RtcProbeLogic.h"

bool isRtcI2cProbeSuccess(uint8_t endTransmissionCode) {
  return endTransmissionCode == 0;
}
