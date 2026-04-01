#include "evmlir/Dialect/EVM/EVMDialect.h"
#include "evmlir/Dialect/EVM/EVMOps.h"
#include "mlir/IR/DialectImplementation.h"

using namespace mlir;
using namespace evmlir::evm;

#include "evmlir/Dialect/EVM/EVMDialect.cpp.inc"

void EVMDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "evmlir/Dialect/EVM/EVMOps.cpp.inc"
      >();
}