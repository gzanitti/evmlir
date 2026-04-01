
#include "mlir/Analysis/Liveness.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Value.h"

class LivenessInfo {
public:
  explicit LivenessInfo(mlir::func::FuncOp func);

  const mlir::Liveness::ValueSetT &getLiveOut(mlir::Block *block) const;
  const mlir::Liveness::ValueSetT &getLiveIn(mlir::Block *block) const;

  unsigned getUseCount(mlir::Value value) const;

private:
  mlir::Liveness liveness;
  mlir::DenseMap<mlir::Value, unsigned> useCounts;
};