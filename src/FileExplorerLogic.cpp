#include "FileExplorerLogic.h"
#include <algorithm>

String fileBaseName(const String& pathOrName) {
  int slash = pathOrName.lastIndexOf('/');
  if (slash < 0) return pathOrName;
  return pathOrName.substring(slash + 1);
}

void sortFileEntries(std::vector<FileEntryView>& entries) {
  std::sort(entries.begin(), entries.end(), [](const FileEntryView& a, const FileEntryView& b){
    if (a.isDir != b.isDir) return a.isDir > b.isDir;
    return a.name < b.name;
  });
}

String formatFileEntry(const FileEntryView& e) {
  return String(e.isDir ? "[D] " : "[F] ") + e.name + "  " + String(e.size) + " bytes";
}

String parentPath(const String& currentPath) {
  if (currentPath.length() == 0 || currentPath == "/") return "/";
  int lastSlash = currentPath.lastIndexOf('/');
  if (lastSlash <= 0) return "/";
  return currentPath.substring(0, lastSlash);
}

String joinPath(const String& base, const String& name) {
  if (name.length() == 0) return base.length() ? base : String("/");
  if (base.length() == 0 || base == "/") return String("/") + name;
  return base + "/" + name;
}
