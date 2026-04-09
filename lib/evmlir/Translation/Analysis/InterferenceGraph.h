#pragma once

#include "LivenessInfo.h"
#include "mlir/Analysis/Liveness.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Support/LLVM.h"
using ValueList = decltype(llvm::make_first_range(
    std::declval<
        const mlir::DenseMap<mlir::Value, mlir::DenseSet<mlir::Value>> &>()));

class InterferenceGraph {
public:
  InterferenceGraph(LivenessInfo &livenessInfo, mlir::func::FuncOp func);

  const mlir::DenseSet<mlir::Value> *neighbors(mlir::Value v) const;
  void removeNode(mlir::Value v);
  unsigned useCount(mlir::Value v) const;
  ValueList getValues() const;
  bool empty() const { return adjacency.empty(); }
  LivenessInfo &getLivenessInfo() const { return livenessInfo; }

private:
  mlir::DenseMap<mlir::Value, mlir::DenseSet<mlir::Value>> adjacency;
  LivenessInfo &livenessInfo;
};