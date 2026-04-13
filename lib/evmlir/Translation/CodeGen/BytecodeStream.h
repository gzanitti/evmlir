#pragma once

#include "Opcode.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <llvm-18/llvm/ADT/APInt.h>
#include <sys/types.h>
#include <unordered_map>
#include <variant>
#include <vector>

struct LabelID {
  uint16_t value;
  bool operator==(LabelID other) const { return value == other.value; }
};

template <> struct std::hash<LabelID> {
  size_t operator()(LabelID id) const noexcept {
    return std::hash<uint16_t>{}(id.value);
  }
};

struct OpcodeInstr {
  Opcode opcode;
};
struct PushInstr {
  Opcode opcode;
  uint8_t value[32];
};
struct LabelRefInstr {
  LabelID label;
};
struct LabelDefInstr {
  LabelID label;
};

using Instruction =
    std::variant<OpcodeInstr, PushInstr, LabelRefInstr, LabelDefInstr>;

class BytecodeStream {
public:
  void emit(Opcode opcode);
  void emitPush(uint8_t value[32]);
  void emitPush(uint32_t value);
  void emitPush(const llvm::APInt &value);
  LabelID createLabel();
  void emitJumpTarget(LabelID label);
  void defineLabel(LabelID label);
  std::vector<uint8_t> finalize();
  uint32_t size() const;

private:
  std::vector<Instruction> instructions;
  uint16_t nextLabelID = 0;
};