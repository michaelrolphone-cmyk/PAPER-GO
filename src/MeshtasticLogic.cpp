#include "MeshtasticLogic.h"

String formatMeshtasticStorageStatus(size_t messageFiles, size_t nodeFiles) {
  return String("messages=") + String(messageFiles) + ", nodes=" + String(nodeFiles);
}
