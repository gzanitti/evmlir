
#include "BytecodeStream.h"

void BytecodeStream::emit(Opcode opcode) {
  instructions.push_back(OpcodeInstr{opcode});
}

void BytecodeStream::emitPush(uint8_t value[32]) {
  int leadingZeros = 0;
  while (leadingZeros < 32 && value[leadingZeros] == 0)
    ++leadingZeros;

  int numBytes = 32 - leadingZeros;
  Opcode opcode =
      (numBytes == 0)
          ? Opcode::PUSH0
          : static_cast<Opcode>(static_cast<uint8_t>(Opcode::PUSH1) +
                                (numBytes - 1));
  PushInstr instr;
  instr.opcode = opcode;
  std::copy(value, value + 32, instr.value);
  instructions.push_back(instr);
}

void BytecodeStream::emitPush(uint32_t value) {
  uint8_t bytes[32] = {};
  bytes[28] = (value >> 24) & 0xFF;
  bytes[29] = (value >> 16) & 0xFF;
  bytes[30] = (value >> 8) & 0xFF;
  bytes[31] = value & 0xFF;
  emitPush(bytes);
}

void BytecodeStream::emitPush(const llvm::APInt &value) {
  uint8_t bytes[32] = {};
  for (int i = 0; i < 4; i++) {
    uint64_t chunk = value.extractBitsAsZExtValue(64, i * 64);
    int base = 24 - i * 8;
    bytes[base] = (chunk >> 56) & 0xFF;
    bytes[base + 1] = (chunk >> 48) & 0xFF;
    bytes[base + 2] = (chunk >> 40) & 0xFF;
    bytes[base + 3] = (chunk >> 32) & 0xFF;
    bytes[base + 4] = (chunk >> 24) & 0xFF;
    bytes[base + 5] = (chunk >> 16) & 0xFF;
    bytes[base + 6] = (chunk >> 8) & 0xFF;
    bytes[base + 7] = chunk & 0xFF;
  }
  emitPush(bytes);
}

LabelID BytecodeStream::createLabel() { return LabelID{nextLabelID++}; }

void BytecodeStream::emitJumpTarget(LabelID label) {
  instructions.push_back(LabelRefInstr{label});
}

void BytecodeStream::defineLabel(LabelID label) {
  instructions.push_back(LabelDefInstr{label});
}

std::vector<uint8_t> BytecodeStream::finalize() {
  std::vector<uint8_t> bytecode;
  std::unordered_map<LabelID, size_t> labelOffsets;
  // TODO: Always emit PUSH2 for label refs (enough for contracts.
  // Could be optimized to use the smallest PUSH that fits.
  std::vector<std::pair<size_t, LabelID>> patchSites;

  // First pass: emit instructions, record label offsets, and reserve space for
  // label references using PUSH2 placeholders.
  for (const auto &instr : instructions) {
    if (auto *opcodeInstr = std::get_if<OpcodeInstr>(&instr)) {
      bytecode.push_back(static_cast<uint8_t>(opcodeInstr->opcode));
    } else if (auto *pushInstr = std::get_if<PushInstr>(&instr)) {
      bytecode.push_back(static_cast<uint8_t>(pushInstr->opcode));
      if (pushInstr->opcode != Opcode::PUSH0) {
        int numBytes = static_cast<uint8_t>(pushInstr->opcode) -
                       static_cast<uint8_t>(Opcode::PUSH1) + 1;
        bytecode.insert(bytecode.end(), pushInstr->value + (32 - numBytes),
                        pushInstr->value + 32);
      }
    } else if (auto *labelRef = std::get_if<LabelRefInstr>(&instr)) {
      bytecode.push_back(static_cast<uint8_t>(Opcode::PUSH2));
      patchSites.push_back({bytecode.size(), labelRef->label});
      bytecode.push_back(0x00);
      bytecode.push_back(0x00);
    } else if (auto *labelDef = std::get_if<LabelDefInstr>(&instr)) {
      labelOffsets[labelDef->label] = bytecode.size();
    }
  }

  // Second pass: patch PUSH2 operands with the actual label offsets.
  for (const auto &[site, label] : patchSites) {
    size_t offset = labelOffsets.at(label);
    bytecode[site] = (offset >> 8) & 0xFF;
    bytecode[site + 1] = offset & 0xFF;
  }

  return bytecode;
}

uint32_t BytecodeStream::size() const { return instructions.size(); }