#include "NetworkConnectLogic.h"
#include "WifiConfigLogic.h"

bool shouldAttemptSavedWifiConnect(const String& rawWifiConfigJson, bool hasCache) {
  if (!hasCache) return false;
  WifiConfig cfg = parseWifiConfig(rawWifiConfigJson);
  return cfg.valid;
}
