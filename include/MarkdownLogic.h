#pragma once
#include <Arduino.h>

String markdownPreview(const String& markdown, size_t maxChars, size_t maxLines);
String markdownTitle(const String& markdown);
