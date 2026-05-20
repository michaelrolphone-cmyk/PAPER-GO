#pragma once

#include <Arduino.h>

uint8_t selectGt911Address(bool probe14Ok, bool probe5dOk);
uint8_t probeGt911Address(bool (*probeFn)(uint8_t addr, void* ctx), void* ctx);
