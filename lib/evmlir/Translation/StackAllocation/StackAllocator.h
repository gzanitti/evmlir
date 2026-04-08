#pragma once

#include "../Analysis/InterferenceGraph.h"
#include "../CodeGen/EVMCostModel.h"
#include "../CodeGen/ForkSpec.h"
#include "MemoryAllocator.h"
#include "mlir/IR/Value.h"
#include "mlir/Support/LLVM.h"

struct StackLoc {
  uint8_t position;
};
struct SpilledLoc {
  uint32_t memOffset;
};
struct RecomputeLoc {
  mlir::Operation *op;
};

using ValueLocation = std::variant<StackLoc, SpilledLoc, RecomputeLoc>;
class StackAllocator {

public:
  StackAllocator(InterferenceGraph &graph, MemoryAllocator mem, ForkSpec spec)
      : graph(graph), memAllocator(mem), costModel(EVMCostModel(spec)) {}
  InterferenceGraph &graph;
  mlir::SmallVector<mlir::Value> simplificationStack;
  mlir::DenseMap<mlir::Value, mlir::DenseSet<mlir::Value>> neighborSnapshot;
  mlir::DenseMap<mlir::Value, ValueLocation> assignment;

  mlir::DenseMap<mlir::Value, ValueLocation> run();
  mlir::Value lower_spill_cost();

private:
  MemoryAllocator memAllocator;
  EVMCostModel costModel;
  unsigned spill_cost(mlir::Value v) const;
};