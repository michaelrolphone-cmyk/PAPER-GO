#pragma once
#include <Arduino.h>
#include <vector>

size_t springboardPageCount(size_t itemCount, size_t pageSize);
size_t springboardPageStart(size_t pageIndex, size_t pageSize);
int springboardTappedIndexForPage(int16_t x, int16_t y, size_t pageIndex, size_t pageSize);
bool springboardMoveAppToFront(std::vector<String>& orderedIds, size_t selectedIndex);
bool springboardOnlineRequiredUnavailable(bool appSupportsOffline, bool wifiConnected);
bool springboardCanOpenApp(bool appSupportsOffline, bool wifiConnected);
bool springboardHandleHomePress(size_t& page);
bool springboardAdvancePage(size_t& page, size_t pageCount);
bool springboardRetreatPage(size_t& page, size_t pageCount);
