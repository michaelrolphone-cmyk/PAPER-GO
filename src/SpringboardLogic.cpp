#include "SpringboardLogic.h"

static const int kCols = 5;
static const int kCellW = 180;
static const int kCellH = 130;
static const int kStartX = 25;
static const int kStartY = 105;
static const int kTileW = 150;
static const int kTileH = 100;

size_t springboardPageCount(size_t itemCount, size_t pageSize) {
  if (pageSize == 0) return 0;
  return (itemCount + pageSize - 1) / pageSize;
}

size_t springboardPageStart(size_t pageIndex, size_t pageSize) {
  return pageIndex * pageSize;
}

int springboardTappedIndexForPage(int16_t x, int16_t y, size_t pageIndex, size_t pageSize) {
  if (x < kStartX || y < kStartY) return -1;
  int c = (x - kStartX) / kCellW;
  int r = (y - kStartY) / kCellH;
  if (c < 0 || c >= kCols || r < 0) return -1;
  int local = r * kCols + c;
  if (local < 0 || static_cast<size_t>(local) >= pageSize) return -1;

  int tileX = kStartX + c * kCellW;
  int tileY = kStartY + r * kCellH;
  if (x > tileX + kTileW || y > tileY + kTileH) return -1;
  return static_cast<int>(springboardPageStart(pageIndex, pageSize)) + local;
}

bool springboardMoveAppToFront(std::vector<String>& orderedIds, size_t selectedIndex) {
  if (selectedIndex >= orderedIds.size()) return false;
  if (selectedIndex == 0) return false;
  String id = orderedIds[selectedIndex];
  orderedIds.erase(orderedIds.begin() + selectedIndex);
  orderedIds.insert(orderedIds.begin(), id);
  return true;
}
