#include "BleAdvertLogic.h"

String buildBleAdvertSummary(const String& name, int rssi, int serviceUuidCount, int manufacturerDataBytes) {
  String summary = "BLE";
  if (name.length()) summary += " name=" + name;
  summary += " rssi=" + String(rssi);
  summary += " svc=" + String(serviceUuidCount);
  summary += " mfgBytes=" + String(manufacturerDataBytes);
  return summary;
}
