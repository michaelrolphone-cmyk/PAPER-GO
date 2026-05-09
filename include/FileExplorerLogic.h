#pragma once
#include <Arduino.h>
#include <vector>

struct FileEntryView {
  String name;
  bool isDir = false;
  size_t size = 0;
};

void sortFileEntries(std::vector<FileEntryView>& entries);
String formatFileEntry(const FileEntryView& e);
