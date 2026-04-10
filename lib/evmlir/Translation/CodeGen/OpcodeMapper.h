#pragma once

#include "Opcode.h"
#include "evmlir/Dialect/EVM/EVMOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/IR/Operation.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/TypeSwitch.h"
#include <optional>

namespace mlir { class Operation; }

std::optional<Opcode> getOpcode(mlir::Operation *op);
