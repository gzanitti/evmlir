
#include "InterferenceGraph.h"

InterferenceGraph::InterferenceGraph(LivenessInfo &livenessInfo,
                                     mlir::func::FuncOp func)
    : livenessInfo(livenessInfo) {

  for (auto &block : llvm::reverse(func.getBlocks())) {
    auto liveNow = livenessInfo.getLiveOut(&block);
    for (auto &op : llvm::reverse(block.getOperations())) {
      for (auto operand : op.getOperands()) {
        liveNow.insert(operand);
      }
      for (auto result : op.getResults()) {
        for (auto live : liveNow) {
          if (live == result)
            continue;
          adjacency[result].insert(live);
          adjacency[live].insert(result);
        }
      }
      for (auto result : op.getResults()) {
        liveNow.erase(result);
      }
    }

    for (auto arg : block.getArguments()) {
      for (auto live : liveNow) {
        adjacency[arg].insert(live);
        adjacency[live].insert(arg);
      }
      liveNow.erase(arg);
    }
  }
}

const mlir::DenseSet<mlir::Value> *
InterferenceGraph::neighbors(mlir::Value v) const {
  auto it = adjacency.find(v);
  if (it != adjacency.end()) {
    return &it->second;
  }
  return nullptr;
}

void InterferenceGraph::removeNode(mlir::Value v) {
  auto it = adjacency.find(v);
  if (it == adjacency.end())
    return;

  for (auto neighbor : it->second)
    adjacency[neighbor].erase(v);

  adjacency.erase(it);
}

unsigned InterferenceGraph::useCount(mlir::Value v) const {
  return livenessInfo.getUseCount(v);
}

ValueList InterferenceGraph::getValues() const {
  return llvm::make_first_range(adjacency);
}
