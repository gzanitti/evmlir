#include "evmlir/Dialect/EVM/EVMDialect.h"
#include "evmlir/Dialect/EVM/EVMOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
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
  auto mutAttr = op->getAttrOfType<mlir::IntegerAttr>("evm.mutability");

  if (!visAttr || !kindAttr || !mutAttr)
    return op->emitOpError("EVM function must have evm.visibility, "
                           "evm.kind and evm.mutability attributes");

  auto vis = static_cast<Visibility>(visAttr.getInt());
  auto kind = static_cast<FunctionKind>(kindAttr.getInt());

  if (vis == Visibility::Internal) {
    if (op->getAttr("evm.selector"))
      return op->emitOpError("internal function cannot have a selector");
    if (kind != FunctionKind::Function)
      return op->emitOpError("internal function must have kind 'function'");
  }

  if (kind == FunctionKind::Constructor || kind == FunctionKind::Fallback ||
      kind == FunctionKind::Receive) {
    if (vis != Visibility::External)
      return op->emitOpError("constructor/fallback/receive must be external");
  }

  if (kind == FunctionKind::Fallback || kind == FunctionKind::Receive) {
    if (func.getNumArguments() > 0)
      return op->emitOpError("fallback/receive cannot have parameters");
  }

  if (kind == FunctionKind::Receive) {
    if (func.getNumResults() > 0)
      return op->emitOpError("receive cannot return values");
  }

  return mlir::success();
}