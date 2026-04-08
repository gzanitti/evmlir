#ifndef EVMLIR_DIALECT_EVM_EVMOPS_H
#define EVMLIR_DIALECT_EVM_EVMOPS_H

#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#define GET_OP_CLASSES
#include "evmlir/Dialect/EVM/EVMOps.h.inc"

#endif // EVMLIR_DIALECT_EVM_EVMOPS_H