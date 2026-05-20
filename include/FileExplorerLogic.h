#pragma once
#include <Arduino.h>
#include <vector>

struct FileEntryView {
  String name;
  bool isDir = false;
  size_t size = 0;
};

String fileBaseName(const String& pathOrName);
void sortFileEntries(std::vector<FileEntryView>& entries);
String formatFileEntry(const FileEntryView& e);
String parentPath(const String& currentPath);
String joinPath(const String& base, const String& name);
