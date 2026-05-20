#include "MapCacheOpsLogic.h"

uint32_t removeFilesRecursively(fs::FS& fs, const String& rootPath) {
  File dir = fs.open(rootPath);
  if (!dir) return 0;
  uint32_t removed = 0;
  File f = dir.openNextFile();
  while (f) {
    String path = String(f.name());
    if (f.isDirectory()) {
      removed += removeFilesRecursively(fs, path);
    } else {
      if (path.length() && fs.remove(path)) removed++;
    }
    f = dir.openNextFile();
  }
  dir.close();
  return removed;
}
