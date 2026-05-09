#include "MeshtasticApiLogic.h"

String buildMeshtasticStatsJson(size_t messageFiles, size_t nodeFiles) {
  return String("{\"messageFiles\":") + String(messageFiles) + ",\"nodeFiles\":" + String(nodeFiles) + "}";
}
