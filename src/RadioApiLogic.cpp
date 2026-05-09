#include "RadioApiLogic.h"

String buildRadioScanListJson(const std::vector<String>& fileNames) {
  String out = "{\"files\":[";
  for (size_t i=0;i<fileNames.size();++i) {
    if (i) out += ",";
    out += "\"" + fileNames[i] + "\"";
  }
  out += "]}";
  return out;
}
