#pragma once
#include <Arduino.h>
#include <FS.h>

uint32_t removeFilesRecursively(fs::FS& fs, const String& rootPath);
