#pragma once

#include <Arduino.h>

String bootStepLog(const String& step, bool ok, const String& detail = "");
String boolLabel(bool value);
