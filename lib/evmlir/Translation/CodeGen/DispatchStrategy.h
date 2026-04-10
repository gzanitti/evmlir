#include "BytecodeStream.h"
#include <cstdint>
#include <llvm-18/llvm/ADT/ArrayRef.h>
class DispatcherStrategy {
public:
  virtual ~DispatcherStrategy() = default;

  /// Emits code to read the 4-byte function selector from calldata
  /// and leave it on top of the stack.
  virtual void emitSelectorLoad(BytecodeStream &stream) = 0;

  /// Emits the dispatch logic. Assumes the selector is already on top
  /// of the stack.
  virtual void emit(llvm::ArrayRef<std::pair<uint32_t, LabelID>> entries,
                    BytecodeStream &stream) = 0;
};