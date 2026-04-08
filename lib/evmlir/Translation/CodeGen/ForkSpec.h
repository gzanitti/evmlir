#pragma once

#include <cstdint>
#include <llvm-18/llvm/ADT/StringRef.h>

struct OpcodeInfo {
  uint32_t staticGas;
  uint32_t minGas;
  uint32_t maxGas;
  bool hasDynamicComponent;
};

struct ForkSpec {
  llvm::StringRef name;
  OpcodeInfo opcodes[256];
};

extern const ForkSpec OsakaSpec;