
#include "../Analysis/InterferenceGraph.h"
#include "mlir/IR/Value.h"
#include "mlir/Support/LLVM.h"

class StackAllocator {

public:
  StackAllocator(InterferenceGraph &graph) : graph(graph) {}
  InterferenceGraph &graph;
  mlir::SmallVector<mlir::Value> simplificationStack;
  mlir::DenseMap<mlir::Value, mlir::DenseSet<mlir::Value>> neighborSnapshot;
  mlir::DenseSet<mlir::Value> spilledValues;
  mlir::DenseMap<mlir::Value, unsigned> assignment;

  mlir::DenseMap<mlir::Value, unsigned> run();
  mlir::Value lower_spill_cost() const;

private:
  unsigned spill_cost(mlir::Value v) const;
};