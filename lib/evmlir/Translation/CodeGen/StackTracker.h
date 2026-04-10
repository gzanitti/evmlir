#include "mlir/IR/Value.h"
#include <llvm-18/llvm/ADT/SmallVector.h>
#include <optional>

class StackTracker {
public:
  void push(mlir::Value v);
  void pop();
  std::optional<uint8_t> findDepth(mlir::Value v) const;
  void dup(uint8_t depth);
  void swap(uint8_t depth);
  uint8_t size() const;

private:
  llvm::SmallVector<mlir::Value> stack;
};
