#pragma once

#include <Arduino.h>

struct CommonControlRowStyle {
  uint8_t textGray = 0;
  uint8_t accentGray = 2;
};

String commonControlBoolLabel(bool value);
String commonControlLabeledValue(const String& label, const String& value);
uint8_t commonControlTextColor(bool selected, const CommonControlRowStyle& style = CommonControlRowStyle{});
void drawCommonControlRow(class BoardHAL& board, int x, int y, const String& label, const String& value, bool selected, const CommonControlRowStyle& style = CommonControlRowStyle{});
