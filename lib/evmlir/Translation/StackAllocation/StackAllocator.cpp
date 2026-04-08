
#include "StackAllocator.h"
#include "mlir/IR/Value.h"
#include "mlir/Support/LLVM.h"
#include <cassert>
mlir::DenseMap<mlir::Value, ValueLocation> StackAllocator::run() {
  while (!graph.empty()) {
    mlir::Value toRemove;
    for (auto &value : graph.getValues()) {
      auto neighbors = graph.neighbors(value);
      if (neighbors->size() < 16) {
        toRemove = value;
        break;
      }
    }

    if (!toRemove) {
      toRemove = lower_spill_cost();
      assert(toRemove &&
             "lower_spill_cost returned empty value on non-empty graph");
      assignment[toRemove] = SpilledLoc{memAllocator.allocate()};
      ;
    }

    // Snapshot current neighbors before removal: these are exactly the
    // neighbors that will be already colored when we pop `toRemove` during
    // simplificationStack loop.
    neighborSnapshot[toRemove] = *graph.neighbors(toRemove);
    simplificationStack.push_back(toRemove);
    graph.removeNode(toRemove);
  }

  while (!simplificationStack.empty()) {
    auto value = simplificationStack.pop_back_val();
    mlir::DenseSet<unsigned> usedColors;
    for (auto neighbor : neighborSnapshot[value]) {
      auto it = assignment.find(neighbor);
      if (it == assignment.end())
        continue;

      if (auto *stackLoc = std::get_if<StackLoc>(&it->second))
        usedColors.insert(stackLoc->position);
    }

    uint8_t assignedColor = 0;
    while (usedColors.count(assignedColor))
      assignedColor++;
    assignment[value] = StackLoc{assignedColor};
  }

  return assignment;
};

unsigned StackAllocator::spill_cost(mlir::Value v) const {
  return graph.useCount(v) / graph.neighbors(v)->size();
}

mlir::Value StackAllocator::lower_spill_cost() {
  llvm::DenseSet<mlir::Value> currentStack;
  for (auto &[value, loc] : assignment)
    if (std::holds_alternative<StackLoc>(loc))
      currentStack.insert(value);

  mlir::Value lowestCostValue;
  unsigned lowestCost = std::numeric_limits<unsigned>::max();
  for (auto &value : graph.getValues()) {
    if (assignment.count(value))
      continue;
    unsigned cost = spill_cost(value);
    if (cost < lowestCost) {
      lowestCost = cost;
      lowestCostValue = value;
    }
  }

  auto recomputed = costModel.recomputeCost(lowestCostValue, currentStack);
  if (recomputed && *recomputed < costModel.spillMemoryCost()) {
    assignment[lowestCostValue] = RecomputeLoc{lowestCostValue.getDefiningOp()};
  } else {
    assignment[lowestCostValue] = SpilledLoc{memAllocator.allocate()};
  }

  return lowestCostValue;
}