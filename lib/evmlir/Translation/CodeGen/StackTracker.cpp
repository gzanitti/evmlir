

#include "StackTracker.h"

void StackTracker::push(mlir::Value v) { stack.push_back(v); }

void StackTracker::pop() { stack.pop_back(); }

std::optional<uint8_t> StackTracker::findDepth(mlir::Value v) const {
  uint8_t limit = std::min((uint8_t)stack.size(), (uint8_t)16);
  for (int i = stack.size() - 1; i >= (int)(stack.size() - limit); --i) {
    if (stack[i] == v)
      return stack.size() - 1 - i;
  }
  return std::nullopt;
}

void StackTracker::dup(uint8_t depth) {
  if (depth >= stack.size())
    return; // TODO: error handling
  stack.push_back(stack[stack.size() - 1 - depth]);
}

void StackTracker::swap(uint8_t depth) {
  if (depth >= stack.size())
    return; // TODO: error handling
  std::swap(stack[stack.size() - 1], stack[stack.size() - 1 - depth]);
}

uint8_t StackTracker::size() const { return stack.size(); }

void StackTracker::reset() { stack.clear(); }