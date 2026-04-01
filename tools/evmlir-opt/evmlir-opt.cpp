#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"
#include "mlir/Transforms/Passes.h"

#include "evmlir/Dialect/EVM/EVMDialect.h"
#include "evmlir/Transforms/Passes.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;

  registry.insert<evmlir::evm::EVMDialect>();
  registry.insert<mlir::func::FuncDialect>();
  registry.insert<mlir::arith::ArithDialect>();

  mlir::registerCanonicalizerPass();
  mlir::registerCSEPass();

  evmlir::evm::registerEVMTransformsPasses();

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "evmlir optimizer driver\n", registry));
}