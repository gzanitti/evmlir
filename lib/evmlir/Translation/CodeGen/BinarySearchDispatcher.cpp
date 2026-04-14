
#include "BinarySearchDispatcher.h"

void BinarySearchDispatcher::emit(
    llvm::ArrayRef<std::tuple<uint32_t, LabelID, mlir::func::FuncOp>> entries,
    BytecodeStream &stream) {
  // Sort entries by selector for correctness.
  llvm::SmallVector<std::tuple<uint32_t, LabelID, mlir::func::FuncOp>> sorted(
      entries.begin(), entries.end());
  llvm::sort(sorted,
             [](auto &a, auto &b) { return std::get<0>(a) < std::get<0>(b); });
  emitBinarySearch(sorted, stream);
}

void BinarySearchDispatcher::emitBinarySearch(
    llvm::ArrayRef<std::tuple<uint32_t, LabelID, mlir::func::FuncOp>> entries,
    BytecodeStream &stream) {

  if (entries.empty()) {
    stream.emitPush(uint32_t(0));
    stream.emitPush(uint32_t(0));
    stream.emit(Opcode::REVERT);
    return;
  }

  if (entries.size() == 1) {
    auto func = std::get<2>(entries[0]);
    auto args = func.getArguments();

    stream.emit(Opcode::DUP1);
    stream.emitPush(std::get<0>(entries[0]));
    stream.emit(Opcode::EQ);

    if (args.empty()) {
      // Direct jump - No ABI decoding
      stream.emitJumpTarget(std::get<1>(entries[0]));
      stream.emit(Opcode::JUMPI);
    } else {
      LabelID matchLabel = stream.createLabel();
      stream.emitJumpTarget(matchLabel);
      stream.emit(Opcode::JUMPI);

      stream.emitPush(uint32_t(0));
      stream.emitPush(uint32_t(0));
      stream.emit(Opcode::REVERT);

      stream.defineLabel(matchLabel);
      stream.emit(Opcode::JUMPDEST);

      for (int i = args.size() - 1; i >= 0; --i) {
        uint32_t offset = 4 + i * 32;
        stream.emitPush(offset);
        stream.emit(Opcode::CALLDATALOAD);
      }

      stream.emitJumpTarget(std::get<1>(entries[0]));
      stream.emit(Opcode::JUMP);
    }

    stream.emitPush(uint32_t(0));
    stream.emitPush(uint32_t(0));
    stream.emit(Opcode::REVERT);
    return;
  }

  uint32_t mid = entries.size() / 2;
  LabelID rightLabel = stream.createLabel();

  stream.emit(Opcode::DUP1);
  stream.emitPush(std::get<0>(entries[mid]));
  stream.emit(Opcode::LT);
  stream.emitJumpTarget(rightLabel);
  stream.emit(Opcode::JUMPI);

  emitBinarySearch(entries.slice(0, mid), stream);

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