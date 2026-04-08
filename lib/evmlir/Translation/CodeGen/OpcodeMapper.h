#pragma once

#include "Opcode.h"
#include <optional>

namespace mlir { class Operation; }

std::optional<Opcode> getOpcode(mlir::Operation *op);
