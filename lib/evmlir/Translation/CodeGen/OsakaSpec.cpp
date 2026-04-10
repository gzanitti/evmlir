// OsakaSpec.cpp
// Author: Claude Code - Sonnet 4.6
//
//
// Gas costs for the Osaka hard fork (execution layer of Fusaka).
//
// Sources:
//   EIP-7607  (Osaka meta)         https://eips.ethereum.org/EIPS/eip-7607
//   EIP-7939  (CLZ opcode)         https://eips.ethereum.org/EIPS/eip-7939
//   EIP-2929  (Berlin, cold/warm)  https://eips.ethereum.org/EIPS/eip-2929
//   EIP-2200  (Istanbul, SSTORE)   https://eips.ethereum.org/EIPS/eip-2200
//   EIP-3529  (London, refund cut) https://eips.ethereum.org/EIPS/eip-3529
//   EIP-3860  (Shanghai, initcode) https://eips.ethereum.org/EIPS/eip-3860
//   EIP-1153  (Cancun, TLOAD/TSTORE)
//   EIP-5656  (Cancun, MCOPY)
//   EIP-4844  (Cancun, BLOBHASH)
//   EIP-7516  (Cancun, BLOBBASEFEE)
//
// The only new opcode added in Osaka (vs Cancun) is CLZ (0x1e, EIP-7939).
// All other costs carry forward from Cancun unchanged.
//
// OpcodeInfo fields: {staticGas, minGas, maxGas, hasDynamicComponent}
//   staticGas           : base cost always charged (warm path for access
//   opcodes) minGas              : minimum total gas (warm path, smallest
//   inputs) maxGas              : maximum bounded total gas; UINT32_MAX if
//   theoretically unbounded
//                         (memory expansion grows quadratically so can be
//                         arbitrarily large)
//   hasDynamicComponent : true when additional gas depends on inputs, state, or
//   memory
//
// --- Memory expansion formula (applies to any opcode that grows active memory)
// ---
//   mem_words(x) = ceil(x / 32)
//   C_mem(n)     = (n*n / 512) + (3 * n)    where n = mem_words
//   expansion_cost = C_mem(new_words) - C_mem(old_words)
//   This is quadratic beyond ~23 KB, so maxGas is unbounded for memory-touching
//   opcodes.
//
// --- EIP-2929 access cost constants ---
//   COLD_ACCOUNT_ACCESS = 2600   (first access to an address in this tx)
//   WARM_ACCOUNT_ACCESS =  100   (subsequent accesses)
//   COLD_SLOAD          = 2100   (first access to a storage slot in this tx)
//   WARM_SLOAD          =  100   (subsequent accesses)

#include "ForkSpec.h"

static ForkSpec buildOsakaSpec() {
  ForkSpec spec;
  spec.name = "osaka";

  // =========================================================================
  // 0x00–0x0b  Arithmetic
  // =========================================================================

  spec.opcodes[static_cast<uint8_t>(Opcode::STOP)] = {0, 0, 0, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::ADD)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::MUL)] = {5, 5, 5, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SUB)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DIV)] = {5, 5, 5, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SDIV)] = {5, 5, 5, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::MOD)] = {5, 5, 5, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SMOD)] = {5, 5, 5, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::ADDMOD)] = {8, 8, 8, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::MULMOD)] = {8, 8, 8, false};

  // EXP: gas = 10 + 50 * byte_length(exponent)
  //   byte_length(0) = 0  → min = 10
  //   exponent is a 256-bit value → max byte_length = 32 → max = 10 + 50*32 =
  //   1610
  spec.opcodes[static_cast<uint8_t>(Opcode::EXP)] = {10, 10, 1610, true};

  spec.opcodes[static_cast<uint8_t>(Opcode::SIGNEXTEND)] = {5, 5, 5, false};

  // =========================================================================
  // 0x10–0x1e  Comparison & Bitwise Logic
  // =========================================================================

  spec.opcodes[static_cast<uint8_t>(Opcode::LT)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::GT)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SLT)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SGT)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::EQ)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::ISZERO)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::AND)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::OR)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::XOR)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::NOT)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::BYTE)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SHL)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SHR)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SAR)] = {3, 3, 3, false};

  // CLZ: COUNT LEADING ZEROS — NEW in Osaka (EIP-7939)
  spec.opcodes[static_cast<uint8_t>(Opcode::CLZ)] = {5, 5, 5, false};

  // =========================================================================
  // 0x20  SHA3 / Keccak
  // =========================================================================

  // KECCAK256: gas = 30 + 6 * ceil(size / 32) + mem_expansion(offset, size)
  //   min: 30 (size=0, no memory growth)
  //   max: unbounded (size and mem_expansion can be arbitrarily large)
  spec.opcodes[static_cast<uint8_t>(Opcode::KECCAK256)] = {30, 30, UINT32_MAX,
                                                           true};

  // =========================================================================
  // 0x30–0x3f  Environmental Information
  // =========================================================================

  spec.opcodes[static_cast<uint8_t>(Opcode::ADDRESS)] = {2, 2, 2, false};

  // BALANCE (EIP-2929): warm=100, cold=2600
  //   staticGas = 100 (warm path); cold adds 2500 surcharge on first access.
  spec.opcodes[static_cast<uint8_t>(Opcode::BALANCE)] = {100, 100, 2600, true};

  spec.opcodes[static_cast<uint8_t>(Opcode::ORIGIN)] = {2, 2, 2, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::CALLER)] = {2, 2, 2, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::CALLVALUE)] = {2, 2, 2, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::CALLDATALOAD)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::CALLDATASIZE)] = {2, 2, 2, false};

  // CALLDATACOPY: gas = 3 + 3 * ceil(size / 32) + mem_expansion(destOffset,
  // size)
  //   min: 3 (size=0)  max: unbounded
  spec.opcodes[static_cast<uint8_t>(Opcode::CALLDATACOPY)] = {3, 3, UINT32_MAX,
                                                              true};

  spec.opcodes[static_cast<uint8_t>(Opcode::CODESIZE)] = {2, 2, 2, false};

  // CODECOPY: gas = 3 + 3 * ceil(size / 32) + mem_expansion(destOffset, size)
  spec.opcodes[static_cast<uint8_t>(Opcode::CODECOPY)] = {3, 3, UINT32_MAX,
                                                          true};

  spec.opcodes[static_cast<uint8_t>(Opcode::GASPRICE)] = {2, 2, 2, false};

  // EXTCODESIZE (EIP-2929): warm=100, cold=2600
  spec.opcodes[static_cast<uint8_t>(Opcode::EXTCODESIZE)] = {100, 100, 2600,
                                                             true};

  // EXTCODECOPY (EIP-2929):
  //   gas = access_cost + 3 * ceil(size / 32) + mem_expansion(destOffset, size)
  //   access_cost: 100 (warm) or 2600 (cold)
  //   min: 100 (warm, size=0, no mem growth)  max: unbounded
  spec.opcodes[static_cast<uint8_t>(Opcode::EXTCODECOPY)] = {100, 100,
                                                             UINT32_MAX, true};

  spec.opcodes[static_cast<uint8_t>(Opcode::RETURNDATASIZE)] = {2, 2, 2, false};

  // RETURNDATACOPY: gas = 3 + 3 * ceil(size / 32) + mem_expansion(destOffset,
  // size)
  spec.opcodes[static_cast<uint8_t>(Opcode::RETURNDATACOPY)] = {
      3, 3, UINT32_MAX, true};

  // EXTCODEHASH (EIP-2929): warm=100, cold=2600
  spec.opcodes[static_cast<uint8_t>(Opcode::EXTCODEHASH)] = {100, 100, 2600,
                                                             true};

  // =========================================================================
  // 0x40–0x4a  Block Information
  // =========================================================================

  spec.opcodes[static_cast<uint8_t>(Opcode::BLOCKHASH)] = {20, 20, 20, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::COINBASE)] = {2, 2, 2, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::TIMESTAMP)] = {2, 2, 2, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::NUMBER)] = {2, 2, 2, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PREVRANDAO)] = {
      2, 2, 2, false}; // formerly DIFFICULTY
  spec.opcodes[static_cast<uint8_t>(Opcode::GASLIMIT)] = {2, 2, 2, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::CHAINID)] = {2, 2, 2, false};

  // SELFBALANCE (EIP-1884): reads the executing contract's own balance directly
  // from
  //   the execution frame. NOT subject to EIP-2929 cold/warm access. Flat 5
  //   gas.
  spec.opcodes[static_cast<uint8_t>(Opcode::SELFBALANCE)] = {5, 5, 5, false};

  spec.opcodes[static_cast<uint8_t>(Opcode::BASEFEE)] = {2, 2, 2, false};

  // BLOBHASH (EIP-4844): returns versioned hash of the i-th blob. Flat 3 gas.
  spec.opcodes[static_cast<uint8_t>(Opcode::BLOBHASH)] = {3, 3, 3, false};

  // BLOBBASEFEE (EIP-7516): returns current blob base fee. Flat 2 gas.
  spec.opcodes[static_cast<uint8_t>(Opcode::BLOBBASEFEE)] = {2, 2, 2, false};

  // =========================================================================
  // 0x50–0x5f  Stack, Memory, Storage and Flow Operations
  // =========================================================================

  spec.opcodes[static_cast<uint8_t>(Opcode::POP)] = {2, 2, 2, false};

  // MLOAD: gas = 3 + mem_expansion(offset, 32)
  //   Loads 32 bytes from memory starting at offset. Expands if offset+32 >
  //   active memory.
  spec.opcodes[static_cast<uint8_t>(Opcode::MLOAD)] = {3, 3, UINT32_MAX, true};

  // MSTORE: gas = 3 + mem_expansion(offset, 32)
  spec.opcodes[static_cast<uint8_t>(Opcode::MSTORE)] = {3, 3, UINT32_MAX, true};

  // MSTORE8: gas = 3 + mem_expansion(offset, 1)
  spec.opcodes[static_cast<uint8_t>(Opcode::MSTORE8)] = {3, 3, UINT32_MAX,
                                                         true};

  // SLOAD (EIP-2929):
  //   If the slot has NOT been accessed this tx: charge 2100 (cold SLOAD cost).
  //   If the slot HAS been accessed this tx:     charge 100  (warm SLOAD cost).
  //   staticGas = 100 (warm); cold surcharge = +2000.
  spec.opcodes[static_cast<uint8_t>(Opcode::SLOAD)] = {100, 100, 2100, true};

  // SSTORE (EIP-2200 + EIP-2929 + EIP-3529):
  //   Complex cost that depends on:
  //     (a) cold/warm state of the storage slot (EIP-2929)
  //     (b) original value (at tx start), current value, and new value
  //
  //   Step 1 — Cold slot surcharge (EIP-2929):
  //     If slot not yet accessed this tx: add 2100 and mark warm.
  //
  //   Step 2 — EIP-2200 dynamic cost (applied after cold surcharge):
  //     current == new (no-op, slot unchanged)            → 100 gas
  //     original == current AND original == 0 AND new!=0  → 20000 gas  (zero →
  //     nonzero) original == current AND original != 0 AND new!=0  →  2900 gas
  //     (nonzero → nonzero) original == current AND original != 0 AND new==0  →
  //     2900 gas  (nonzero → zero, +4800 refund) original != current (dirty
  //     slot, any)             →   100 gas  (±refund adjustments)
  //
  //   Refund rules (EIP-3529): max refund per tx = gas_used / 5.
  //     Clearing (nonzero→0 on clean slot): +4800 refund.
  //     Restoring original (dirty slot):    +19900 (if original==0) or +2800
  //     (if original!=0).
  //
  //   Practical min/max (first access, cold):
  //     min  = 2100 + 100  =  2200  (cold, dirty no-op write)
  //     max  = 2100 + 20000 = 22100  (cold, zero→nonzero)
  //   Warm path:
  //     min  = 100  (warm no-op)
  //     max  = 20000 (warm, zero→nonzero)
  spec.opcodes[static_cast<uint8_t>(Opcode::SSTORE)] = {100, 100, 22100, true};

  spec.opcodes[static_cast<uint8_t>(Opcode::JUMP)] = {8, 8, 8, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::JUMPI)] = {10, 10, 10, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PC)] = {2, 2, 2, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::MSIZE)] = {2, 2, 2, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::GAS)] = {2, 2, 2, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::JUMPDEST)] = {1, 1, 1, false};

  // TLOAD (EIP-1153): transient storage read. Flat 100 gas. No persistence
  // across txs.
  spec.opcodes[static_cast<uint8_t>(Opcode::TLOAD)] = {100, 100, 100, false};

  // TSTORE (EIP-1153): transient storage write. Flat 100 gas. No persistence
  // across txs.
  spec.opcodes[static_cast<uint8_t>(Opcode::TSTORE)] = {100, 100, 100, false};

  // MCOPY (EIP-5656): gas = 3 + 3 * ceil(length / 32) + mem_expansion
  //   Copies memory regions. Both src and dst windows may trigger expansion
  //   independently; the expansion is computed as C_mem(max(dst_end, src_end))
  //   - C_mem(current). min: 3 (length=0, no expansion)  max: unbounded
  spec.opcodes[static_cast<uint8_t>(Opcode::MCOPY)] = {3, 3, UINT32_MAX, true};

  // PUSH0 (EIP-3855): pushes the constant 0 onto the stack. Flat 2 gas.
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH0)] = {2, 2, 2, false};

  // =========================================================================
  // 0x60–0x7f  PUSH1..PUSH32  (all cost 3 gas, no dynamic component)
  // =========================================================================

  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH1)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH2)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH3)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH4)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH5)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH6)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH7)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH8)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH9)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH10)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH11)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH12)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH13)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH14)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH15)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH16)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH17)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH18)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH19)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH20)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH21)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH22)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH23)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH24)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH25)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH26)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH27)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH28)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH29)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH30)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH31)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::PUSH32)] = {3, 3, 3, false};

  // =========================================================================
  // 0x80–0x8f  DUP1..DUP16  (all cost 3 gas)
  // =========================================================================

  spec.opcodes[static_cast<uint8_t>(Opcode::DUP1)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP2)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP3)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP4)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP5)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP6)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP7)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP8)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP9)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP10)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP11)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP12)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP13)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP14)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP15)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::DUP16)] = {3, 3, 3, false};

  // =========================================================================
  // 0x90–0x9f  SWAP1..SWAP16  (all cost 3 gas)
  // =========================================================================

  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP1)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP2)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP3)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP4)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP5)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP6)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP7)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP8)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP9)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP10)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP11)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP12)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP13)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP14)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP15)] = {3, 3, 3, false};
  spec.opcodes[static_cast<uint8_t>(Opcode::SWAP16)] = {3, 3, 3, false};

  // =========================================================================
  // 0xa0–0xa4  Logging Operations
  // =========================================================================

  // LOGn: gas = 375 + (375 * n_topics) + (8 * data_byte_length) + mem_expansion
  //   The topic count is fixed per opcode (0..4), so the base = 375 + 375*n.
  //   The data length and memory expansion are fully dynamic.
  //   staticGas = 375 + 375*n (minimum with 0 bytes of data and no memory
  //   growth).
  //
  //   LOG0: 375 + 0*375 = 375
  //   LOG1: 375 + 1*375 = 750
  //   LOG2: 375 + 2*375 = 1125
  //   LOG3: 375 + 3*375 = 1500
  //   LOG4: 375 + 4*375 = 1875
  spec.opcodes[static_cast<uint8_t>(Opcode::LOG0)] = {375, 375, UINT32_MAX,
                                                      true};
  spec.opcodes[static_cast<uint8_t>(Opcode::LOG1)] = {750, 750, UINT32_MAX,
                                                      true};
  spec.opcodes[static_cast<uint8_t>(Opcode::LOG2)] = {1125, 1125, UINT32_MAX,
                                                      true};
  spec.opcodes[static_cast<uint8_t>(Opcode::LOG3)] = {1500, 1500, UINT32_MAX,
                                                      true};
  spec.opcodes[static_cast<uint8_t>(Opcode::LOG4)] = {1875, 1875, UINT32_MAX,
                                                      true};

  // =========================================================================
  // 0xf0–0xff  System Operations
  // =========================================================================

  // CREATE: gas = 32000 + initcode_word_cost + mem_expansion +
  // code_deposit_cost
  //   initcode_word_cost (EIP-3860): 2 * ceil(initcode_len / 32)
  //   code_deposit_cost (post-execution): 200 * returned_code_size
  //   The code_deposit_cost is charged after the init code runs and can be
  //   large. min: 32000 (empty initcode, no expansion, no returned code) max:
  //   unbounded (arbitrarily large initcode and returned code)
  spec.opcodes[static_cast<uint8_t>(Opcode::CREATE)] = {32000, 32000,
                                                        UINT32_MAX, true};

  // CALL (EIP-2929 + EIP-150):
  //   gas = access_cost + value_cost + new_acct_cost + mem_expansion +
  //   gas_forwarded
  //
  //   access_cost   = 100 (warm) or 2600 (cold) for the target address
  //   value_cost    = 9000 if call_value > 0
  //   new_acct_cost = 25000 if (call_value > 0 AND target account is empty per
  //   EIP-161) mem_expansion = cost to expand for (argsOffset,argsLength) and
  //   (retOffset,retLength) gas_forwarded = min(requested, floor(remaining *
  //   63/64))  [EIP-150, not a cost to caller] stipend       = 2300 added to
  //   callee if call_value > 0 (not deducted from caller)
  //
  //   min: 100 (warm, no value, non-empty target, no memory growth, 0 gas
  //   forwarded) max: 2600 + 9000 + 25000 = 36600 (cold + value + new account)
  //   + mem_exp + forwarded gas
  //        In practice unbounded due to memory expansion and forwarded gas
  //        consumption.
  spec.opcodes[static_cast<uint8_t>(Opcode::CALL)] = {100, 100, UINT32_MAX,
                                                      true};

  // CALLCODE: same formula as CALL. Legacy opcode (delegates code but keeps
  // caller context).
  //   Access cost, value cost, and new account cost rules are identical to
  //   CALL.
  spec.opcodes[static_cast<uint8_t>(Opcode::CALLCODE)] = {100, 100, UINT32_MAX,
                                                          true};

  // RETURN: gas = mem_expansion(offset, size)
  //   No base cost. Only charges if memory must be expanded to satisfy the
  //   output range.
  spec.opcodes[static_cast<uint8_t>(Opcode::RETURN)] = {0, 0, UINT32_MAX, true};

  // DELEGATECALL (EIP-2929 + EIP-150):
  //   gas = access_cost + mem_expansion + gas_forwarded
  //   No value transfer (cannot send ETH), so no value_cost or new_acct_cost.
  //   access_cost: 100 (warm) or 2600 (cold).
  spec.opcodes[static_cast<uint8_t>(Opcode::DELEGATECALL)] = {100, 100,
                                                              UINT32_MAX, true};

  // CREATE2 (EIP-1014 + EIP-3860):
  //   gas = 32000
  //       + 6 * ceil(initcode_len / 32)     // keccak256 hashing for address
  //       derivation
  //       + 2 * ceil(initcode_len / 32)     // EIP-3860 initcode word cost
  //       + mem_expansion
  //       + code_deposit_cost               // 200 * returned_code_size
  //   Total word overhead = 8 gas per 32-byte word of initcode.
  //   min: 32000 (empty initcode)  max: unbounded
  spec.opcodes[static_cast<uint8_t>(Opcode::CREATE2)] = {32000, 32000,
                                                         UINT32_MAX, true};

  // STATICCALL (EIP-214 + EIP-2929 + EIP-150):
  //   Same formula as DELEGATECALL. Read-only execution frame (no state changes
  //   allowed). access_cost: 100 (warm) or 2600 (cold). No value transfer.
  spec.opcodes[static_cast<uint8_t>(Opcode::STATICCALL)] = {100, 100,
                                                            UINT32_MAX, true};

  // REVERT: gas = mem_expansion(offset, size)
  //   No base cost. Reverts state changes, returns remaining gas to caller.
  spec.opcodes[static_cast<uint8_t>(Opcode::REVERT)] = {0, 0, UINT32_MAX, true};

  // INVALID: consumes ALL remaining gas in the current frame. No refund.
  //   Treated as hasDynamicComponent=true because cost = remaining_gas (fully
  //   dynamic). maxGas = UINT32_MAX as a sentinel for "all remaining gas".
  spec.opcodes[static_cast<uint8_t>(Opcode::INVALID)] = {0, 0, UINT32_MAX,
                                                         true};

  // SELFDESTRUCT (EIP-2929 + EIP-3529):
  //   gas = 5000
  //       + 2600  if target address is cold (first access this tx)
  //       + 25000 if (balance > 0 AND target account is empty per EIP-161)
  //   Note: EIP-3529 (London) removed the 24000 gas refund for SELFDESTRUCT
  //   entirely. min: 5000 (warm target, either balance==0 or target non-empty)
  //   max: 5000 + 2600 + 25000 = 32600 (cold + value to empty account)
  spec.opcodes[static_cast<uint8_t>(Opcode::SELFDESTRUCT)] = {5000, 5000, 32600,
                                                              true};

  return spec;
}

const ForkSpec OsakaSpec = buildOsakaSpec();
