#include "MarkdownLogic.h"

static String simplifyInlineMarkdown(String line) {
  while (true) {
    int imageStart = line.indexOf("![");
    if (imageStart < 0) break;
    int altEnd = line.indexOf(']', imageStart + 2);
    int urlStart = altEnd >= 0 ? line.indexOf('(', altEnd) : -1;
    int urlEnd = urlStart >= 0 ? line.indexOf(')', urlStart) : -1;
    if (altEnd < 0 || urlStart < 0 || urlEnd < 0) break;
    String alt = line.substring(imageStart + 2, altEnd);
    line = line.substring(0, imageStart) + "[Image: " + alt + "]" + line.substring(urlEnd + 1);
  }
  while (true) {
    int linkStart = line.indexOf('[');
    if (linkStart < 0) break;
    int textEnd = line.indexOf(']', linkStart + 1);
    int urlStart = textEnd >= 0 ? line.indexOf('(', textEnd) : -1;
    int urlEnd = urlStart >= 0 ? line.indexOf(')', urlStart) : -1;
    if (textEnd < 0 || urlStart < 0 || urlEnd < 0) break;
    String text = line.substring(linkStart + 1, textEnd);
    line = line.substring(0, linkStart) + text + line.substring(urlEnd + 1);
  }
  line.replace("**", "");
  line.replace("__", "");
  line.replace("*", "");
  line.replace("_", "");
  line.replace("`", "");
  return line;
}

static void appendWrapped(String& out, const String& line, size_t& lines, size_t maxLines, size_t maxChars, size_t wrapWidth) {
  int start = 0;
  while (start < (int)line.length() && lines < maxLines && out.length() < maxChars) {
    int remaining = line.length() - start;
    int width = remaining > (int)wrapWidth ? (int)wrapWidth : remaining;
    String chunk = line.substring(start, start + width);
    if (width == (int)wrapWidth && start + width < (int)line.length()) {
      int split = chunk.lastIndexOf(' ');
      if (split > 8) {
        chunk = chunk.substring(0, split);
        width = split + 1;
      }
    }
    chunk.trim();
    out += chunk + "\n";
    lines++;
    start += width;
  }
}

size_t markdownLineCount(const String& text) {
  if (text.length() == 0) return 0;
  size_t count = 0;
  for (size_t i = 0; i < text.length(); ++i) {
    if (text[i] == '\n') count++;
  }
  if (text[text.length() - 1] != '\n') count++;
  return count;
}

int markdownClampStartLine(const String& rendered, int requestedStartLine, size_t windowLines) {
  size_t totalLines = markdownLineCount(rendered);
  if (totalLines <= windowLines) return 0;
  int maxStart = (int)(totalLines - windowLines);
  if (requestedStartLine < 0) return 0;
  if (requestedStartLine > maxStart) return maxStart;
  return requestedStartLine;
}

String markdownWindow(const String& rendered, int startLine, size_t windowLines) {
  if (rendered.length() == 0 || windowLines == 0) return "";
  int currentLine = 0;
  int startIdx = 0;
  while (startIdx < (int)rendered.length() && currentLine < startLine) {
    if (rendered[startIdx] == '\n') currentLine++;
    startIdx++;
  }

  int endIdx = startIdx;
  size_t emittedLines = 0;
  while (endIdx < (int)rendered.length() && emittedLines < windowLines) {
    if (rendered[endIdx] == '\n') emittedLines++;
    endIdx++;
  }
  return rendered.substring(startIdx, endIdx);
}

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

String markdownRenderPreview(const String& markdown, size_t maxChars, size_t maxLines, size_t wrapWidth) {
  String out;
  size_t lines = 0;
  int start = 0;
  bool inCodeBlock = false;
  while (start <= (int)markdown.length() && lines < maxLines && out.length() < maxChars) {
    int end = markdown.indexOf('\n', start);
    if (end < 0) end = markdown.length();
    String line = markdown.substring(start, end);
    start = end + 1;
    String trimmed = line;
    trimmed.trim();

    if (trimmed.startsWith("```")) {
      inCodeBlock = !inCodeBlock;
      continue;
    }
    if (trimmed.length() == 0) {
      out += "\n";
      lines++;
      continue;
    }

    if (inCodeBlock) {
      appendWrapped(out, "| " + line, lines, maxLines, maxChars, wrapWidth);
      continue;
    }

    if (trimmed.startsWith("#")) {
      while (trimmed.startsWith("#")) trimmed.remove(0, 1);
      trimmed.trim();
      appendWrapped(out, String("[H] ") + simplifyInlineMarkdown(trimmed), lines, maxLines, maxChars, wrapWidth);
      continue;
    }
    if (trimmed.startsWith(">")) {
      trimmed.remove(0, 1);
      trimmed.trim();
      appendWrapped(out, String("> ") + simplifyInlineMarkdown(trimmed), lines, maxLines, maxChars, wrapWidth);
      continue;
    }
    if (trimmed == "---" || trimmed == "***" || trimmed == "___") {
      appendWrapped(out, "----------------", lines, maxLines, maxChars, wrapWidth);
      continue;
    }
    if (trimmed.startsWith("- ") || trimmed.startsWith("* ")) {
      String bulletBody = trimmed.substring(2);
      if (bulletBody.startsWith("[ ] ")) {
        appendWrapped(out, String("☐ ") + simplifyInlineMarkdown(bulletBody.substring(4)), lines, maxLines, maxChars, wrapWidth);
      } else if (bulletBody.startsWith("[x] ") || bulletBody.startsWith("[X] ")) {
        appendWrapped(out, String("☑ ") + simplifyInlineMarkdown(bulletBody.substring(4)), lines, maxLines, maxChars, wrapWidth);
      } else {
        appendWrapped(out, String("• ") + simplifyInlineMarkdown(bulletBody), lines, maxLines, maxChars, wrapWidth);
      }
      continue;
    }
    int dot = trimmed.indexOf('.');
    if (dot > 0) {
      bool ordered = true;
      for (int i = 0; i < dot; ++i) {
        if (!isDigit(trimmed[i])) { ordered = false; break; }
      }
      if (ordered && dot + 1 < (int)trimmed.length() && trimmed[dot + 1] == ' ') {
        appendWrapped(out, trimmed.substring(0, dot + 1) + " " + simplifyInlineMarkdown(trimmed.substring(dot + 2)), lines, maxLines, maxChars, wrapWidth);
        continue;
      }
    }
    if (trimmed.indexOf('|') >= 0) {
      appendWrapped(out, String("[T] ") + simplifyInlineMarkdown(trimmed), lines, maxLines, maxChars, wrapWidth);
      continue;
    }

    appendWrapped(out, simplifyInlineMarkdown(trimmed), lines, maxLines, maxChars, wrapWidth);
  }
  return out;
}
