
#include "StackAllocator.h"
#include "mlir/IR/Value.h"
#include "mlir/Support/LLVM.h"
#include <cassert>
mlir::DenseMap<mlir::Value, unsigned> StackAllocator::run() {
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
      spilledValues.insert(toRemove);
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
      if (it != assignment.end())
        usedColors.insert(it->second);
    }

    unsigned assignedColor = 0;
    while (usedColors.count(assignedColor))
      assignedColor++;
    assignment[value] = assignedColor;
  }

  return assignment;
};

unsigned StackAllocator::spill_cost(mlir::Value v) const {
  return graph.useCount(v) / graph.neighbors(v)->size();
  // TODO: better heuristic
}

mlir::Value StackAllocator::lower_spill_cost() const {
  mlir::Value lowestCostValue;
  unsigned lowestCost = std::numeric_limits<unsigned>::max();

  for (auto &value : graph.getValues()) {
    if (spilledValues.count(value))
      continue;

    unsigned cost = spill_cost(value);
    if (cost < lowestCost) {
      lowestCost = cost;
      lowestCostValue = value;
    }
  }

  return lowestCostValue;
}