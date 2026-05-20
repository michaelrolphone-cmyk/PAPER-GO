#include "CommonControlsLogic.h"
#include "BoardHAL.h"

String commonControlBoolLabel(bool value) {
  return value ? "true" : "false";
}

String commonControlLabeledValue(const String& label, const String& value) {
  return label + ": " + value;
}

uint8_t commonControlTextColor(bool selected, const CommonControlRowStyle& style) {
  return selected ? style.accentGray : style.textGray;
}

void drawCommonControlRow(BoardHAL& board, int x, int y, const String& label, const String& value, bool selected, const CommonControlRowStyle& style) {
  board.drawText(x, y, commonControlLabeledValue(label, value), commonControlTextColor(selected, style), 1);
}
