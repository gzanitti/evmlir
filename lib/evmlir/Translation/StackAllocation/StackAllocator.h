#pragma once

#include "../Analysis/InterferenceGraph.h"
#include "../CodeGen/EVMCostModel.h"
#include "../CodeGen/ForkSpec.h"
#include "MemoryAllocator.h"
#include "mlir/IR/Value.h"
#include "mlir/Support/LLVM.h"
#include <cassert>

struct StackLoc {
  uint8_t color;
  bool operator==(const StackLoc &other) const { return color == other.color; }
  bool operator!=(const StackLoc &other) const {
    return !(color == other.color);
  }
};

struct SpilledLoc {
  uint32_t memOffset;
  bool operator==(const SpilledLoc &other) const {
    return memOffset == other.memOffset;
  }
  bool operator!=(const SpilledLoc &other) const {
    return !(memOffset == other.memOffset);
  }
};

struct RecomputeLoc {
  mlir::Operation *op;
  bool operator==(const RecomputeLoc &other) const { return op == other.op; }
  bool operator!=(const RecomputeLoc &other) const { return !(op == other.op); }
};

using ValueLocation = std::variant<StackLoc, SpilledLoc, RecomputeLoc>;
class StackAllocator {

public:
  StackAllocator(InterferenceGraph &graph, MemoryAllocator mem, ForkSpec spec)
      : graph(graph), memAllocator(mem), costModel(EVMCostModel(spec)) {}
  mlir::SmallVector<mlir::Value> simplificationStack;
  mlir::DenseMap<mlir::Value, mlir::DenseSet<mlir::Value>> neighborSnapshot;
  mlir::DenseMap<mlir::Value, ValueLocation> assignment;

  mlir::DenseMap<mlir::Value, ValueLocation> run();
  mlir::Value lower_spill_cost();

private:
  InterferenceGraph &graph;
  MemoryAllocator memAllocator;
  EVMCostModel costModel;
  unsigned spill_cost(mlir::Value v) const;
};