#include "BytecodeStream.h"
#include <cstdint>
#include <tuple>
#include <llvm-18/llvm/ADT/ArrayRef.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
class DispatcherStrategy {
public:
  virtual ~DispatcherStrategy() = default;

  /// Emits code to read the 4-byte function selector from calldata
  /// and leave it on top of the stack.
  virtual void emitSelectorLoad(BytecodeStream &stream) = 0;

  /// Emits the dispatch logic. Assumes the selector is already on top
  /// of the stack.
  virtual void emit(
      llvm::ArrayRef<std::tuple<uint32_t, LabelID, mlir::func::FuncOp>> entries,
      BytecodeStream &stream, std::optional<LabelID> fallback) = 0;
};