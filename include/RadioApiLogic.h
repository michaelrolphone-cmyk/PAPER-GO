#pragma once
#include <Arduino.h>
#include <vector>

String buildRadioScanListJson(const std::vector<String>& fileNames);
