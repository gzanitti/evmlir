
#include "Opcode.h"
#include <cstdint>
#include <variant>
#include <vector>

struct LabelID {
  uint16_t value;
  bool operator==(LabelID other) const { return value == other.value; }
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

using Instruction = std::variant<OpcodeInstr, LabelRefInstr, LabelDefInstr>;

class BytecodeStream {
public:
  void emit(Opcode opcode);

  void emitPush(uint8_t value[32]);

  LabelID createLabel();

  void emitJumpTarget(LabelID label);

  void defineLabel(LabelID label);

  std::vector<uint8_t> finalize();
};