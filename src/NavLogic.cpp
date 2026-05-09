#include "NavLogic.h"

void NavigationStack::clear() { _stack.clear(); }

void NavigationStack::onOpen(const String& currentId, const String& nextId) {
  if (!currentId.length()) return;
  if (currentId == nextId) return;
  _stack.push_back(currentId);
}

String NavigationStack::popBackTarget() {
  if (_stack.empty()) return "";
  String id = _stack.back();
  _stack.pop_back();
  return id;
}
