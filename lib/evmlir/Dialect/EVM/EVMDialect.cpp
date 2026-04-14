#include "evmlir/Dialect/EVM/EVMDialect.h"
#include "evmlir/Dialect/EVM/EVMOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/DialectImplementation.h"

using namespace mlir;
using namespace evmlir::evm;

#include "evmlir/Dialect/EVM/EVMAttributes.cpp.inc"
#include "evmlir/Dialect/EVM/EVMDialect.cpp.inc"

void EVMDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "evmlir/Dialect/EVM/EVMOps.cpp.inc"
      >();
}

mlir::LogicalResult
EVMDialect::verifyOperationAttribute(mlir::Operation *op,
                                     mlir::NamedAttribute attr) {
  auto func = mlir::dyn_cast<mlir::func::FuncOp>(op);
  if (!func)
    return mlir::success();

  auto visAttr = op->getAttrOfType<mlir::IntegerAttr>("evm.visibility");
  auto kindAttr = op->getAttrOfType<mlir::IntegerAttr>("evm.kind");

  if (!visAttr || !kindAttr)
    return op->emitOpError("EVM function must have evm.visibility and "
                           "evm.kind attributes");

  auto vis = static_cast<Visibility>(visAttr.getInt());
  auto kind = static_cast<FunctionKind>(kindAttr.getInt());

  if (vis == Visibility::Internal) {
    if (op->getAttr("evm.selector"))
      return op->emitOpError("internal function cannot have a selector");
    if (kind != FunctionKind::Function)
      return op->emitOpError("internal function must have kind 'function'");
  }

  return mlir::success();
}