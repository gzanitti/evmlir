#pragma once
#include "DispatchStrategy.h"
#include <tuple>

class BinarySearchDispatcher : public DispatcherStrategy {
public:
  /// Emits a binary search dispatch tree.
  void emit(
      llvm::ArrayRef<std::tuple<uint32_t, LabelID, mlir::func::FuncOp>> entries,
      BytecodeStream &stream, std::optional<LabelID> fallback) override;

  void emitSelectorLoad(BytecodeStream &stream) override;

private:
  void emitBinarySearch(
      llvm::ArrayRef<std::tuple<uint32_t, LabelID, mlir::func::FuncOp>> entries,
      BytecodeStream &stream, std::optional<LabelID> fallback);
};