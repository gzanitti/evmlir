
#include "evmlir/Dialect/EVM/EVMDialect.h"
#include "evmlir/Dialect/EVM/EVMOps.h"
#include "evmlir/Transforms/Passes.h"

#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;
using namespace evmlir::evm;

namespace evmlir::evm {

#define GEN_PASS_DEF_EVMCANONICALIZE
#include "evmlir/Transforms/Passes.h.inc"

struct EVMCanonicalizePass
    : public impl::EVMCanonicalizeBase<EVMCanonicalizePass> {

  void runOnOperation() override {
    mlir::func::FuncOp func = getOperation();

    RewritePatternSet patterns(&getContext());

    getContext().getLoadedDialect<EVMDialect>()->getCanonicalizationPatterns(
        patterns);

    if (failed(applyPatternsAndFoldGreedily(func, std::move(patterns))))
      signalPassFailure();
  }
};

std::unique_ptr<mlir::Pass> createEVMCanonicalizePass() {
  return std::make_unique<EVMCanonicalizePass>();
}

} // namespace evmlir::evm