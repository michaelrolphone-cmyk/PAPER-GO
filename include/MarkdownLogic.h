#pragma once
#include <Arduino.h>

String markdownPreview(const String& markdown, size_t maxChars, size_t maxLines);
String markdownTitle(const String& markdown);
String markdownRenderPreview(const String& markdown, size_t maxChars, size_t maxLines, size_t wrapWidth);
size_t markdownLineCount(const String& text);
int markdownClampStartLine(const String& rendered, int requestedStartLine, size_t windowLines);
String markdownWindow(const String& rendered, int startLine, size_t windowLines);
String markdownBuildProgressState(const String& path, int startLine);
int markdownReadProgressStartLine(const String& stateJson, const String& path, int defaultStartLine);
