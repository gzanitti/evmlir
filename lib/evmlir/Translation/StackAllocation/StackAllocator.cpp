
#include "StackAllocator.h"

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
      llvm::DenseSet<mlir::Value> currentStack;
      for (auto &[value, loc] : assignment)
        if (std::holds_alternative<StackLoc>(loc))
          currentStack.insert(value);

      bool isWarm = memAllocator.hasFreeSlots();
      uint32_t expandCost = memAllocator.expansionCost();
      auto recomputed =
          costModel.recomputeCost(toRemove, currentStack, isWarm, expandCost);
      if (recomputed &&
          *recomputed < costModel.spillMemoryCost(isWarm, expandCost))
        assignment[toRemove] = RecomputeLoc{toRemove.getDefiningOp()};
      else
        assignment[toRemove] = SpilledLoc{memAllocator.allocate()};
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

    if (assignment.count(value))
      continue;

    mlir::DenseSet<unsigned> usedColors;
    for (auto neighbor : neighborSnapshot[value]) {
      auto it = assignment.find(neighbor);
      if (it == assignment.end())
        continue;

      if (auto *stackLoc = std::get_if<StackLoc>(&it->second))
        usedColors.insert(stackLoc->color);
    }

    auto *op = value.getDefiningOp();

    for (auto &[spilledValue, loc] : assignment) {
      if (!std::holds_alternative<SpilledLoc>(loc))
        continue;

      bool isDead = false;
      if (op) {
        isDead = graph.getLivenessInfo().isDeadAfter(spilledValue, op);
      } else {
        auto blockArg = mlir::cast<mlir::BlockArgument>(value);
        auto &liveOut = graph.getLivenessInfo().getLiveOut(blockArg.getOwner());
        isDead = !liveOut.count(spilledValue);
      }

      if (isDead)
        memAllocator.free(std::get<SpilledLoc>(loc).memOffset);
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

  return lowestCostValue;
}