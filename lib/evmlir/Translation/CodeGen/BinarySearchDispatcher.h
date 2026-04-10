#pragma once
#include "DispatchStrategy.h"

class BinarySearchDispatcher : public DispatcherStrategy {
public:
  /// Emits a binary search dispatch tree.
  void emit(llvm::ArrayRef<std::pair<uint32_t, LabelID>> entries,
            BytecodeStream &stream) override;

  void emitSelectorLoad(BytecodeStream &stream) override;

private:
  void emitBinarySearch(llvm::ArrayRef<std::pair<uint32_t, LabelID>> entries,
                        BytecodeStream &stream);
};