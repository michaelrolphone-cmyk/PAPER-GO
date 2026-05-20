#include "SpringboardLogic.h"

static const int kCols = 2;
static const int kCellW = 250;
static const int kCellH = 200;
static const int kStartX = 20;
static const int kStartY = 90;
static const int kTileW = 230;
static const int kTileH = 180;

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
  if (x < tileX || y < tileY) return -1;
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


bool springboardOnlineRequiredUnavailable(bool appSupportsOffline, bool wifiConnected) {
  return !appSupportsOffline && !wifiConnected;
}


bool springboardCanOpenApp(bool appSupportsOffline, bool wifiConnected) {
  return !springboardOnlineRequiredUnavailable(appSupportsOffline, wifiConnected);
}

bool springboardHandleHomePress(size_t& page) {
  if (page == 0) return false;
  page = 0;
  return true;
}


bool springboardAdvancePage(size_t& page, size_t pageCount) {
  if (pageCount <= 1) return false;
  size_t next = page + 1;
  if (next >= pageCount) next = 0;
  if (next == page) return false;
  page = next;
  return true;
}

bool springboardRetreatPage(size_t& page, size_t pageCount) {
  if (pageCount <= 1) return false;
  size_t next = (page == 0) ? (pageCount - 1) : (page - 1);
  if (next == page) return false;
  page = next;
  return true;
}
