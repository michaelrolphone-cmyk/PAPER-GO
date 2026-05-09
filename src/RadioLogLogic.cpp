#include "RadioLogLogic.h"

String formatRadioScanLogLine(const RadioSignal& signal, uint32_t timestampMs, const GpsFix* fix) {
  String out = "ts_ms=" + String(timestampMs);
  out += ",kind=" + signal.kind;
  out += ",name=" + signal.name;
  out += ",addr=" + signal.address;
  out += ",rssi=" + String(signal.rssi);
  out += ",channel=" + String(signal.channel);
  out += ",protocol=" + signal.protocol;
  if (fix && fix->valid) {
    out += ",lat=" + String(fix->lat, 6);
    out += ",lon=" + String(fix->lon, 6);
  }
  return out;
}

String buildRadioScanLog(const std::vector<RadioSignal>& signals, uint32_t timestampMs, const GpsFix* fix) {
  String body;
  for (size_t i=0;i<signals.size();++i) {
    body += formatRadioScanLogLine(signals[i], timestampMs, fix);
    body += "\n";
  }
  return body;
}
