
#include "BinarySearchDispatcher.h"

void BinarySearchDispatcher::emit(
    llvm::ArrayRef<std::pair<uint32_t, LabelID>> entries,
    BytecodeStream &stream) {
  // Sort entries by selector for correctness.
  llvm::SmallVector<std::pair<uint32_t, LabelID>> sorted(entries.begin(),
                                                         entries.end());
  llvm::sort(sorted, [](auto &a, auto &b) { return a.first < b.first; });
  emitBinarySearch(sorted, stream);
}

void BinarySearchDispatcher::emitBinarySearch(
    llvm::ArrayRef<std::pair<uint32_t, LabelID>> entries,
    BytecodeStream &stream) {
  if (entries.empty()) {
    stream.emitPush(uint32_t(0));
    stream.emitPush(uint32_t(0));
    stream.emit(Opcode::REVERT);
    return;
  }

  if (entries.size() == 1) {
    // Base case: one entry left, compare and jump or revert.
    LabelID noMatch = stream.createLabel();

    stream.emit(Opcode::DUP1);
    stream.emitPush(entries[0].first);
    stream.emit(Opcode::EQ);
    stream.emitJumpTarget(entries[0].second);
    stream.emit(Opcode::JUMPI);

    stream.defineLabel(noMatch);
    stream.emit(Opcode::JUMPDEST);
    stream.emitPush(uint32_t(0));
    stream.emitPush(uint32_t(0));
    stream.emit(Opcode::REVERT);
    return;
  }

  // Recursive case: split at midpoint.
  uint32_t mid = entries.size() / 2;
  LabelID rightLabel = stream.createLabel();

  // Compare selector with mid entry's selector.
  stream.emit(Opcode::DUP1);
  stream.emitPush(entries[mid].first);
  stream.emit(Opcode::LT); // selector < mid.selector ?
  stream.emitJumpTarget(rightLabel);
  stream.emit(Opcode::JUMPI); // if selector >= mid, go right

  // Left half: selectors < mid.
  emitBinarySearch(entries.slice(0, mid), stream);

  // Right half: selectors >= mid.
  stream.defineLabel(rightLabel);
  stream.emit(Opcode::JUMPDEST);
  emitBinarySearch(entries.slice(mid), stream);
}

void BinarySearchDispatcher::emitSelectorLoad(BytecodeStream &stream) {
  stream.emitPush(uint32_t(0));
  stream.emit(Opcode::CALLDATALOAD);
  stream.emitPush(uint32_t(0xE0));
  stream.emit(Opcode::SHR);
}