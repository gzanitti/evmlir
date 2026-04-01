#ifndef EVM_PASSES_TD
#define EVM_PASSES_TD

include "mlir/Pass/PassBase.td"

def EVMCanonicalize : Pass<"evm-canonicalize", "mlir::func::FuncOp"> {
  let summary = "Canonicalize EVM dialect operations";
  let description = [{
    Applies canonicalization patterns specific to the EVM dialect.
    This includes algebraic simplifications, constant folding, and
    EVM-specific peephole optimizations.
  }];

  let constructor = "evmlir::evm::createEVMCanonicalizePass()";

  let dependentDialects = ["evmlir::evm::EVMDialect"];
}

#endif // EVM_PASSES_TD