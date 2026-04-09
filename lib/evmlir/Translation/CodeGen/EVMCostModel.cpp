

#include "EVMCostModel.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

std::optional<uint32_t> EVMCostModel::recomputeCost(
    mlir::Value v, const llvm::DenseSet<mlir::Value> &currentStack,
    bool nextSlotIsWarm, uint32_t memExpansionCost) const {
  uint32_t threshold = spillMemoryCost(nextSlotIsWarm, memExpansionCost);
  return recomputeCostImpl(v, currentStack, threshold, 0, 0);
}

std::optional<uint32_t> EVMCostModel::recomputeCostImpl(
    mlir::Value v, const llvm::DenseSet<mlir::Value> &currentStack,
    uint32_t threshold, uint32_t depth, uint32_t accumulatedCost) const {
  if (currentStack.count(v))
    return 0;
  if (depth >= MAX_DEPTH)
    return std::nullopt;
  if (accumulatedCost >= threshold)
    return std::nullopt;
  mlir::Operation *defOp = v.getDefiningOp();
  if (!defOp)
    return std::nullopt;
  if (!canRecompute(defOp))
    return std::nullopt;
  uint32_t cost = opcodeCost(defOp);
  if (cost == UINT32_MAX)
    return std::nullopt;
  uint32_t budget = accumulatedCost + cost;
  if (budget >= threshold)
    return std::nullopt;
  for (mlir::Value operand : defOp->getOperands()) {
    auto operandCost =
        recomputeCostImpl(operand, currentStack, threshold, depth + 1, budget);
    if (!operandCost)
      return std::nullopt;
    cost += *operandCost;
    budget += *operandCost;
    if (budget >= threshold)
      return std::nullopt;
  }
  return cost;
}

bool EVMCostModel::canRecompute(mlir::Operation *op) const {
  auto iface = llvm::dyn_cast<mlir::MemoryEffectOpInterface>(op);
  if (!iface)
    return false;
  return iface.hasNoEffect();
}

uint32_t EVMCostModel::spillMemoryCost(bool nextSlotIsWarm,
                                       uint32_t memExpansionCost) const {
  uint32_t expandCost = nextSlotIsWarm ? 0 : memExpansionCost;
  return spec.opcodes[static_cast<uint8_t>(Opcode::MSTORE)].staticGas +
         spec.opcodes[static_cast<uint8_t>(Opcode::MLOAD)].staticGas +
         2 * expandCost; // once for MSTORE, once for MLOAD
}

uint32_t EVMCostModel::opcodeCost(mlir::Operation *op) const {
  auto opcodeOpt = getOpcode(op);
  if (!opcodeOpt)
    return UINT32_MAX;
  return spec.opcodes[static_cast<uint8_t>(*opcodeOpt)].staticGas;
}