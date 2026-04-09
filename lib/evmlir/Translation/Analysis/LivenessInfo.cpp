

#include "LivenessInfo.h"

LivenessInfo::LivenessInfo(mlir::func::FuncOp func) : liveness(func) {
  func.walk([&](mlir::Operation *op) {
    for (auto operand : op->getOperands()) {
      useCounts[operand]++;
    }
  });
}

const mlir::Liveness::ValueSetT &
LivenessInfo::getLiveIn(mlir::Block *block) const {
  return liveness.getLiveIn(block);
}
const mlir::Liveness::ValueSetT &
LivenessInfo::getLiveOut(mlir::Block *block) const {
  return liveness.getLiveOut(block);
}

unsigned LivenessInfo::getUseCount(mlir::Value value) const {
  auto it = useCounts.find(value);
  return it != useCounts.end() ? it->second : 0;
}

bool LivenessInfo::isDeadAfter(mlir::Value value, mlir::Operation *op) const {
  return liveness.isDeadAfter(value, op);
}