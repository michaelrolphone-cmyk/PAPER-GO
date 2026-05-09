#pragma once
#include <Arduino.h>
#include <vector>

class NavigationStack {
public:
  void clear();
  void onOpen(const String& currentId, const String& nextId);
  String popBackTarget();
  bool hasBack() const { return !_stack.empty(); }
private:
  std::vector<String> _stack;
};
