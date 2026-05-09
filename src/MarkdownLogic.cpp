#include "MarkdownLogic.h"

String markdownTitle(const String& markdown) {
  int start = 0;
  while (start < (int)markdown.length()) {
    int end = markdown.indexOf('\n', start);
    if (end < 0) end = markdown.length();
    String line = markdown.substring(start, end);
    line.trim();
    if (line.length() == 0) { start = end + 1; continue; }
    if (line.startsWith("#")) {
      while (line.startsWith("#")) line.remove(0, 1);
      line.trim();
    }
    return line;
  }
  return "Untitled";
}

String markdownPreview(const String& markdown, size_t maxChars, size_t maxLines) {
  String out;
  size_t lines = 0;
  for (size_t i = 0; i < markdown.length() && out.length() < maxChars; ++i) {
    char c = markdown[i];
    out += c;
    if (c == '\n') {
      lines++;
      if (lines >= maxLines) break;
    }
  }
  return out;
}
