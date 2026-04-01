#ifndef EVMLIR_TRANSFORMS_PASSES_H
#define EVMLIR_TRANSFORMS_PASSES_H

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Pass/Pass.h"

namespace evmlir::evm {
class EVMDialect;
} // namespace evmlir::evm

namespace evmlir::evm {

std::unique_ptr<mlir::Pass> createEVMCanonicalizePass();

#define GEN_PASS_DECL
#include "evmlir/Transforms/Passes.h.inc"

#define GEN_PASS_REGISTRATION
#include "evmlir/Transforms/Passes.h.inc"

} // namespace evmlir::evm

#endif // EVMLIR_TRANSFORMS_PASSES_H