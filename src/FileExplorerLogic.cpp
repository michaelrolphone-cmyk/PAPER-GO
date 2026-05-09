#include "FileExplorerLogic.h"
#include <algorithm>

void sortFileEntries(std::vector<FileEntryView>& entries) {
  std::sort(entries.begin(), entries.end(), [](const FileEntryView& a, const FileEntryView& b){
    if (a.isDir != b.isDir) return a.isDir > b.isDir;
    return a.name < b.name;
  });
}

String formatFileEntry(const FileEntryView& e) {
  return String(e.isDir ? "[D] " : "[F] ") + e.name + "  " + String(e.size) + " bytes";
}
