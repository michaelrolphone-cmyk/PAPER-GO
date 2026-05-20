#pragma once
#include <Arduino.h>
#include <vector>

struct FileListEntry {
  String name;
  String path;
  bool dir = false;
  uint32_t size = 0;
};

bool isAllowedFilePath(const String& path);
String normalizeFilePath(const String& path);
String fileApiErrorJson(const String& code, const String& message);
String fileApiOkJson(const String& dataJson);
String fileListJson(const String& path, int page, int pageSize, int total, const std::vector<FileListEntry>& entries);
