#ifndef EVM_OPS_TD
#define EVM_OPS_TD

include "mlir/IR/DialectBase.td"
include "mlir/IR/OpBase.td"
include "mlir/IR/BuiltinTypes.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "evmlir/Dialect/EVM/EVMAttributes.td"

def I256 : I<256>;

def EVM_Dialect : Dialect {
  let name = "evm";
  let summary = "Dialect for EVM-specific operations.";
  let description = [{
    The `evm` dialect models constructs that are specific to the Ethereum
    Virtual Machine execution model and cannot be expressed by standard MLIR
    dialects (arith, cf, scf, func) without losing semantic information.

    Standard dialects are used for everything they can express correctly.
    This dialect covers only the remainder: persistent and transient storage,
    the EVM memory region, calldata access, and the execution environment
    (block context, transaction context, etc).
  }];
  let cppNamespace = "::evmlir::evm";
  let hasOperationAttrVerify = 1;
}

class EVM_Op<string mnemonic, list<Trait> traits = []>
    : Op<EVM_Dialect, mnemonic, traits>;

def EVM_InlineBytecodeOp : EVM_Op<"inline_bytecode", 
    [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
    let summary = "Emit raw bytecode directly into the output stream.";
    let description = [{
        Emits the given bytes directly into the bytecode output without any
        stack scheduling or validation. The caller is responsible for ensuring
        the bytecode is valid and that the stack state is consistent before
        and after this op. This is an escape hatch for cases where the SSA
        model is insufficient.
    }];
    let arguments = (ins DenseI8ArrayAttr:$bytecode);
    let results = (outs); 
}

def EVM_StopOp : EVM_Op<"stop", [Terminator]> {
  let summary = "Halt execution successfully.";
  let description = [{
    Halts execution of the current call and marks it as successful.
    No return data is provided to the caller.
  }];

  let assemblyFormat = "attr-dict";
}

def EVM_AddmodOp : EVM_Op<"addmod", [Pure, AlwaysSpeculatable]> {
  let summary = "Compute (x + y) mod k.";
  let description = [{
    Computes (x + y) mod k, where x, y, and k are treated as unsigned 256-bit integers.
    If k is zero, the result is defined to be zero.
  }];

  let arguments = (ins I256:$x, I256:$y, I256:$k);
  let results = (outs I256:$result);

  let assemblyFormat = "$x `,` $y `,` $k attr-dict `:` type($x) `,` type($y) `,` type($k) `->` type($result)";
}

def EVM_MulmodOp : EVM_Op<"mulmod", [Pure, AlwaysSpeculatable]> {
  let summary = "Compute (x * y) mod k.";
  let description = [{
    Computes (x * y) mod k, where x, y, and k are treated as unsigned 256-bit integers.
    If k is zero, the result is defined to be zero.
  }];

  let arguments = (ins I256:$x, I256:$y, I256:$k);
  let results = (outs I256:$result);

  let assemblyFormat = "$x `,` $y `,` $k attr-dict `:` type($x) `,` type($y) `,` type($k) `->` type($result)";
}

def EVM_ExpOp : EVM_Op<"exp", [Pure, AlwaysSpeculatable]> {
  let summary = "Exponentiation.";
  let description = [{
    Computes base raised to the power of exponent, where both are treated as unsigned 
    256-bit integers. The result is computed modulo 2^256. If exponent is zero, the 
    result is 1 regardless of the value of base, including when base is zero.
  }];

  let arguments = (ins I256:$x, I256:$y);
  let results = (outs I256:$result);

  let assemblyFormat = "$x `,` $y attr-dict `:` type($x) `,` type($y) `->` type($result)";
}

def EVM_SignextendOp : EVM_Op<"signextend", [Pure, AlwaysSpeculatable]> {
  let summary = "Sign-extend a byte to a 256-bit integer.";
  let description = [{
    Sign-extends x treating it as a signed integer whose sign bit is at byte position 
    byte counting from the LSB, where byte=0 means x is treated as an 8-bit signed 
    integer, byte=1 as a 16-bit signed integer, and so on up to byte=30 for a 248-bit 
    signed integer. If byte >= 31, x is returned unchanged. The upper bits of the 
    result are filled with the sign bit at position byte.
    The result is a signed 256-bit integer.
  }];

  let arguments = (ins I256:$byte, I256:$x);
  let results = (outs I256:$result);

  let assemblyFormat = "$byte `,` $x attr-dict `:` type($byte) `,` type($x) `->` type($result)";
}	

def EVM_ByteOp : EVM_Op<"byte", [Pure, AlwaysSpeculatable]> {
  let summary = "Extract a byte from a 256-bit integer.";
  let description = [{
    Extracts the byte at position `index` from `x`, where `index`=0 means the least
    significant byte and `index`=31 means the most significant byte. If `index` >= 32,
    the result is zero. The result is an i256 containing the extracted byte in its least
    significant 8 bits, with the upper bits set to zero.
  }];

  let arguments = (ins I256:$index, I256:$x);
  let results = (outs I256:$result);

  let assemblyFormat = "$index `,` $x attr-dict `:` type($index) `,` type($x) `->` type($result)";
}

def EVM_SloadOp : EVM_Op<"sload",
    [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Read a value from persistent contract storage.";
  let description = [{
    Reads from the contract's persistent storage at the given slot key.
    Both the slot and the result are i256.
  }];

  let arguments = (ins I256:$slot);
  let results  = (outs I256:$result);

  let assemblyFormat = "$slot attr-dict `:` type($slot) `->` type($result)";
}

def EVM_SstoreOp : EVM_Op<"sstore",
    [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Write a value to persistent contract storage.";
  let description = [{
    Writes to the contract's persistent storage at the given slot key.
    Both the slot and the value are i256.
  }];

  let arguments = (ins I256:$slot, I256:$value);

  let assemblyFormat = "$slot `,` $value attr-dict `:` type($slot) `,` type($value)";
}

def EVM_TloadOp : EVM_Op<"tload",
    [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Read a value from transient storage.";
  let description = [{
    Reads from the contract's transient storage (EIP-1153) at the given slot.
    Both the slot and the result are i256
  }];

  let arguments = (ins I256:$slot);
  let results  = (outs I256:$result);

  let assemblyFormat = "$slot attr-dict `:` type($slot) `->` type($result)";
}

def EVM_TstoreOp : EVM_Op<"tstore",
    [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Write a value to transient storage.";
  let description = [{
    Writes to the contract's transient storage (EIP-1153) at the given slot.
    Both the slot and the value are i256.
    
  }];

  let arguments = (ins I256:$slot, I256:$value);

  let assemblyFormat = "$slot `,` $value attr-dict `:` type($slot) `,` type($value)";
}

def EVM_MloadOp : EVM_Op<"mload",
    [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Read a value from the EVM memory region.";
  let description = [{
    Reads from the EVM memory region at the given byte offset.
    Both the offset and the result are i256.
  }];

  let arguments = (ins I256:$offset);
  let results  = (outs I256:$result);

  let assemblyFormat = "$offset attr-dict `:` type($offset) `->` type($result)";
}

def EVM_MstoreOp : EVM_Op<"mstore",
    [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Write a value to the EVM memory region.";
  let description = [{
    Writes to the EVM memory region at the given byte offset.
    Both the offset and the value are i256.
  }];

  let arguments = (ins I256:$offset, I256:$value);

  let assemblyFormat = "$offset `,` $value attr-dict `:` type($offset) `,` type($value)";
}

def EVM_Mstore8Op : EVM_Op<"mstore8",
    [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Write a byte to the EVM memory region.";
  let description = [{
    Writes the least significant byte of `value` to the EVM memory region at
    the given byte offset. The offset is i256, but the value is truncated to
    8 bits before being written.
  }];

  let arguments = (ins I256:$offset, I256:$value);

  let assemblyFormat = "$offset `,` $value attr-dict `:` type($offset) `,` type($value)";
}

def EVM_MsizeOp : EVM_Op<"msize", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Return the current size of the EVM memory region in bytes.";
  let description = [{
    Returns the current size of the EVM memory region in bytes.
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_McopyOp : EVM_Op<"mcopy", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Copy a region of memory.";
  let description = [{
    Copies `length` bytes from the EVM memory region starting at `source_offset`
    to the region starting at `dest_offset`. This op never traps; if the source
    or destination range is out of bounds, it is treated as zero-padded.
  }];

  let arguments = (ins I256:$dest_offset, I256:$source_offset, I256:$length);

  let assemblyFormat = "$dest_offset `,` $source_offset `,` $length attr-dict `:` type($dest_offset) `,` type($source_offset) `,` type($length)";
}

def EVM_CalldataloadOp : EVM_Op<"calldataload", [Pure, AlwaysSpeculatable]> {
  let summary = "Read a 32-byte word from calldata at the given byte offset.";
  let description = [{
    Reads from the calldata region starting at `offset`.
    If the offset is out of bounds, the result is zero-padded — the op never
    traps. Both the offset and the result are i256.
  }];

  let arguments = (ins I256:$offset);
  let results  = (outs I256:$result);

  let assemblyFormat = "$offset attr-dict `:` type($offset) `->` type($result)";
}

def EVM_CalldatasizeOp : EVM_Op<"calldatasize",
    [Pure, AlwaysSpeculatable]> {
  let summary = "Return the size of the calldata in bytes.";
  let description = [{
    Returns the byte length of the calldata for the current call.
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_CalldatacopyOp : EVM_Op<"calldatacopy", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Copy a region of calldata into memory.";
  let description = [{
    Copies `length` bytes from calldata starting at `calldata_offset` into the
    EVM memory region starting at `memory_offset`. If the source range is
    out of bounds, it is treated as zero-padded; this op never traps.
  }];

  let arguments = (ins I256:$memory_offset, I256:$calldata_offset, I256:$length);

  let assemblyFormat = "$memory_offset `,` $calldata_offset `,` $length attr-dict `:` type($memory_offset) `,` type($calldata_offset) `,` type($length)";
}

def EVM_Keccak256Op : EVM_Op<"keccak256", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Compute the Keccak-256 hash of a region of memory.";
  let description = [{
    Computes the Keccak-256 hash of `length` bytes from the EVM memory region
    starting at `offset`. The result is an i256 containing the hash value.
  }];

  let arguments = (ins I256:$offset, I256:$length);
  let results  = (outs I256:$result);

  let assemblyFormat = "$offset `,` $length attr-dict `:` type($offset) `,` type($length) `->` type($result)";
}

def EVM_CallerOp : EVM_Op<"caller", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the caller's address.";
  let description = [{
    Returns the address of the caller (the account that initiated the current call).
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_CallvalueOp : EVM_Op<"callvalue", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the amount of Ether sent with the call.";
  let description = [{
    Returns the amount of Ether (in wei) sent with the current call.
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_OriginOp : EVM_Op<"origin", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the transaction origin address.";
  let description = [{
    Returns the address of the transaction origin (the account that initiated
    the top-level transaction, which may be different from the caller in the
    case of internal calls). The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_GaspriceOp : EVM_Op<"gasprice", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the gas price of the transaction.";
  let description = [{
    Returns the gas price (in wei) of the current transaction.
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_GasOp : EVM_Op<"gas", []> {
  let summary = "Return the remaining gas.";
  let description = [{
    Returns the amount of gas remaining in the current execution context.
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_AddressOp : EVM_Op<"address", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the current contract's address.";
  let description = [{
    Returns the address of the current contract.
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_CodesizeOp : EVM_Op<"codesize", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the size of the current contract's code in bytes.";
  let description = [{
    Returns the byte length of the current contract's code.
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_SelfbalanceOp : EVM_Op<"selfbalance", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Return the current contract's balance.";
  let description = [{
    Returns the balance (in wei) of the current contract.
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_CodecopyOp : EVM_Op<"codecopy", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Copy a region of the current contract's code into memory.";
  let description = [{
    Copies `length` bytes from the current contract's code starting at `code_offset`
    into the EVM memory region starting at `memory_offset`. If the source range is
    out of bounds, it is treated as zero-padded; this op never traps.
  }];

  let arguments = (ins I256:$memory_offset, I256:$code_offset, I256:$length);

  let assemblyFormat = "$memory_offset `,` $code_offset `,` $length attr-dict `:` type($memory_offset) `,` type($code_offset) `,` type($length)";
}

def EVM_PcOp : EVM_Op<"pc", []> {
  let summary = "Return the current program counter.";
  let description = [{
    Returns the current program counter (the byte offset of the currently executing instruction in the contract's code).
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_NumberOp : EVM_Op<"number", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the current block number.";
  let description = [{
    Returns the number of the current block.
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_TimestampOp : EVM_Op<"timestamp", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the current block timestamp.";
  let description = [{
    Returns the timestamp of the current block as a Unix timestamp (seconds since the epoch).
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_CoinbaseOp : EVM_Op<"coinbase", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the current block's coinbase address.";
  let description = [{
    Returns the coinbase address of the current block (the address of the miner or validator that produced the block).
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_GaslimitOp : EVM_Op<"gaslimit", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the current block's gas limit.";
  let description = [{
    Returns the gas limit of the current block.
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_BasefeeOp : EVM_Op<"basefee", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the current block's base fee.";
  let description = [{
    Returns the base fee of the current block (EIP-1559).
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_PrevrandaoOp : EVM_Op<"prevrandao", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the current block's prevRandao value.";
  let description = [{
    Returns the prevRandao value of the current block (EIP-4399).
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_ChainidOp : EVM_Op<"chainid", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the current chain ID.";
  let description = [{
    Returns the chain ID of the current execution environment.
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_BlobbasefeeOp : EVM_Op<"blobbasefee", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the current block's bloob base fee.";
  let description = [{
    Returns the bloob base fee of the current block.
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_BlockhashOp : EVM_Op<"blockhash", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the hash of a recent block.";
  let description = [{
    Returns the hash of the block with the given block number, or zero if the
    block number is not one of the 256 most recent blocks excluding the current one.
    Both the block number and the result are i256.
  }];

  let arguments = (ins I256:$block_number);
  let results = (outs I256:$result);

  let assemblyFormat = "$block_number attr-dict `:` type($block_number) `->` type($result)";
}

def EVM_BlobhashOp : EVM_Op<"blobhash", [Pure, AlwaysSpeculatable]> {
  let summary = "Return the hash of a blob.";
  let description = [{
    Returns the hash of the blob with the given blob ID, or zero if the blob
    does not exist. Both the blob ID and the result are i256.
  }];

  let arguments = (ins I256:$blob_id);
  let results = (outs I256:$result);

  let assemblyFormat = "$blob_id attr-dict `:` type($blob_id) `->` type($result)";
}

def EVM_BalanceOp : EVM_Op<"balance", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Return the balance of an arbitrary address.";
  let description = [{
    Returns the balance (in wei) of the account with the given address.
    Both the address and the result are i256.
  }];

  let arguments = (ins I256:$address);
  let results = (outs I256:$result);

  let assemblyFormat = "$address attr-dict `:` type($address) `->` type($result)";
}

def EVM_ExtcodesizeOp : EVM_Op<"extcodesize", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Return the size of an arbitrary account's code in bytes.";
  let description = [{
    Returns the byte length of the code of the account with the given address.
    Both the address and the result are i256.
  }];

  let arguments = (ins I256:$address);
  let results = (outs I256:$result);

  let assemblyFormat = "$address attr-dict `:` type($address) `->` type($result)";
}

def EVM_ExtcodehashOp : EVM_Op<"extcodehash", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Return the hash of an arbitrary account's code.";
  let description = [{
    Returns the hash of the code of the account with the given address, or zero
    if the account does not exist. Both the address and the result are i256.
  }];

  let arguments = (ins I256:$address);
  let results = (outs I256:$result);

  let assemblyFormat = "$address attr-dict `:` type($address) `->` type($result)";
}

def EVM_ExtcodecopyOp : EVM_Op<"extcodecopy", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Copy a region of an arbitrary account's code into memory.";
  let description = [{
    Copies `length` bytes from the code of the account with the given address
    starting at `code_offset` into the EVM memory region starting at `memory_offset`.
    If the source range is out of bounds, it is treated as zero-padded; this op never traps.
  }];

  let arguments = (ins I256:$address, I256:$memory_offset, I256:$code_offset, I256:$length);

  let assemblyFormat = "$address `,` $memory_offset `,` $code_offset `,` $length attr-dict `:` type($address) `,` type($memory_offset) `,` type($code_offset) `,` type($length)";
}

def EVM_ReturndatasizeOp : EVM_Op<"returndatasize", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Return the size of the return data buffer in bytes.";
  let description = [{
    Returns the byte length of the return data buffer from the previous call.
    The result type is always i256.
  }];

  let results = (outs I256:$result);

  let assemblyFormat = "attr-dict `->` type($result)";
}

def EVM_ReturndatacopyOp : EVM_Op<"returndatacopy", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Copy a region of the return data buffer into memory.";
  let description = [{
    Copies `length` bytes from the return data buffer starting at `data_offset`
    into the EVM memory region starting at `memory_offset`. If the source range is
    out of bounds, it is treated as zero-padded; this op never traps.
  }];

  let arguments = (ins I256:$memory_offset, I256:$data_offset, I256:$length);

  let assemblyFormat = "$memory_offset `,` $data_offset `,` $length attr-dict `:` type($memory_offset) `,` type($data_offset) `,` type($length)";
}


def EVM_Log0Op : EVM_Op<"log0", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Emit a log entry with zero topics.";
  let description = [{
    Emits a log entry with the given data and zero topics.
  }];

  let arguments = (ins I256:$data_offset, I256:$data_length);

  let assemblyFormat = "$data_offset `,` $data_length attr-dict `:` type($data_offset) `,` type($data_length)";
}

def EVM_Log1Op : EVM_Op<"log1", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Emit a log entry with one topic.";
  let description = [{
    Emits a log entry with the given data and one topic.
  }];

  let arguments = (ins I256:$data_offset, I256:$data_length, I256:$topic1);

  let assemblyFormat = "$data_offset `,` $data_length `,` $topic1 attr-dict `:` type($data_offset) `,` type($data_length) `,` type($topic1)";
}

def EVM_Log2Op : EVM_Op<"log2", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Emit a log entry with two topics.";
  let description = [{
    Emits a log entry with the given data and two topics.
  }];

  let arguments = (ins I256:$data_offset, I256:$data_length, I256:$topic1, I256:$topic2);

  let assemblyFormat = "$data_offset `,` $data_length `,` $topic1 `,` $topic2 attr-dict `:` type($data_offset) `,` type($data_length) `,` type($topic1) `,` type($topic2)";
}

def EVM_Log3Op : EVM_Op<"log3", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Emit a log entry with three topics.";
  let description = [{
    Emits a log entry with the given data and three topics.
  }];

  let arguments = (ins I256:$data_offset, I256:$data_length, I256:$topic1, I256:$topic2, I256:$topic3);

  let assemblyFormat = "$data_offset `,` $data_length `,` $topic1 `,` $topic2 `,` $topic3 attr-dict `:` type($data_offset) `,` type($data_length) `,` type($topic1) `,` type($topic2) `,` type($topic3)";
}

def EVM_Log4Op : EVM_Op<"log4", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Emit a log entry with four topics.";
  let description = [{
    Emits a log entry with the given data and four topics.
  }];

  let arguments = (ins I256:$data_offset, I256:$data_length, I256:$topic1, I256:$topic2, I256:$topic3, I256:$topic4);

  let assemblyFormat = "$data_offset `,` $data_length `,` $topic1 `,` $topic2 `,` $topic3 `,` $topic4 attr-dict `:` type($data_offset) `,` type($data_length) `,` type($topic1) `,` type($topic2) `,` type($topic3) `,` type($topic4)";
}

def EVM_CallOp : EVM_Op<"call", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Execute a call to another account.";
  let description = [{
    Opens a new sub-context and runs the bytecode associated with the target account,
    returning control to the caller upon completion. Accounts with no deployed code
    are treated as successful calls.
  }];

  let arguments = (ins I256:$gas, I256:$to, I256:$value, I256:$args_offset, I256:$args_length, I256:$ret_offset, I256:$ret_length);
  let results = (outs I256:$result);

  let assemblyFormat = "$gas `,` $to `,` $value `,` $args_offset `,` $args_length `,` $ret_offset `,` $ret_length attr-dict `:` type($gas) `,` type($to) `,` type($value) `,` type($args_offset) `,` type($args_length) `,` type($ret_offset) `,` type($ret_length) `->` type($result)";
}

def EVM_DelegatecallOp : EVM_Op<"delegatecall", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Execute a delegate call to another account.";
  let description = [{
    Executes the code of the target account using the caller's own storage,
    msg.sender, and msg.value. Accounts with no deployed code are treated as
    successful calls.
  }];

  let arguments = (ins I256:$gas, I256:$to, I256:$args_offset, I256:$args_length, I256:$ret_offset, I256:$ret_length);
  let results = (outs I256:$result);

  let assemblyFormat = "$gas `,` $to `,` $args_offset `,` $args_length `,` $ret_offset `,` $ret_length attr-dict `:` type($gas) `,` type($to) `,` type($args_offset) `,` type($args_length) `,` type($ret_offset) `,` type($ret_length) `->` type($result)";
}

def EVM_StaticcallOp : EVM_Op<"staticcall", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Execute a static call to another account.";
  let description = [{
    Opens a new read-only sub-context and runs the bytecode of the target account,
    then resumes the current one. State modifications and ETH transfers are not
    permitted within the sub-context. Accounts with no deployed code are treated
    as successful calls (returning 1).

    The following instructions are disallowed within the sub-context: CREATE,
    CREATE2, LOG0, LOG1, LOG2, LOG3, LOG4, SSTORE, SELFDESTRUCT, EOFCREATE,
    and CALL or EXTCALL if the transferred value is non-zero.
  }];

  let arguments = (ins I256:$gas, I256:$to, I256:$args_offset, I256:$args_length, I256:$ret_offset, I256:$ret_length);
  let results = (outs I256:$result);

  let assemblyFormat = "$gas `,` $to `,` $args_offset `,` $args_length `,` $ret_offset `,` $ret_length attr-dict `:` type($gas) `,` type($to) `,` type($args_offset) `,` type($args_length) `,` type($ret_offset) `,` type($ret_length) `->` type($result)";
}

def EVM_CallcodeOp : EVM_Op<"callcode", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Execute a callcode call to another account.";
  let description = [{
    Opens a new sub-context and runs the bytecode of the target account, but using
    the caller's own storage rather than the target's. Accounts with no deployed
    code are treated as successful calls.
  }];

  let arguments = (ins I256:$gas, I256:$to, I256:$value, I256:$args_offset, I256:$args_length, I256:$ret_offset, I256:$ret_length);
  let results = (outs I256:$result);

  let assemblyFormat = "$gas `,` $to `,` $value `,` $args_offset `,` $args_length `,` $ret_offset `,` $ret_length attr-dict `:` type($gas) `,` type($to) `,` type($value) `,` type($args_offset) `,` type($args_length) `,` type($ret_offset) `,` type($ret_length) `->` type($result)";
}

def EVM_CreateOp : EVM_Op<"create", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Create a new account with the given initialization code.";
  let description = [{
    Computes a destination address from the sender's address and nonce, opens a
    sub-context at that address, and executes the initialization code. On success,
    the new account's bytecode is set to the data returned by the initialization
    code and the destination address is returned. Returns zero on failure.

    The destination address is derived as:
      address = keccak256(rlp([sender_address, sender_nonce]))[12:]
  }];

  let arguments = (ins I256:$value, I256:$init_code_offset, I256:$init_code_length);
  let results = (outs I256:$result);

  let assemblyFormat = "$value `,` $init_code_offset `,` $init_code_length attr-dict `:` type($value) `,` type($init_code_offset) `,` type($init_code_length) `->` type($result)";
}

def EVM_Create2Op : EVM_Op<"create2", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  let summary = "Create a new account with the given initialization code and salt.";
  let description = [{
    Behaves like CREATE but derives the destination address from the sender, a
    caller-supplied salt, and the hash of the initialization code, making the
    address predictable before deployment. On success, the new account's bytecode
    is set to the data returned by the initialization code and the destination
    address is returned. Returns zero on failure.

    The destination address is derived as:
      address = keccak256(0xFF ++ sender_address ++ salt ++ keccak256(init_code))[12:]
  }];

  let arguments = (ins I256:$value, I256:$init_code_offset, I256:$init_code_length, I256:$salt);
  let results = (outs I256:$result);

  let assemblyFormat = "$value `,` $init_code_offset `,` $init_code_length `,` $salt attr-dict `:` type($value) `,` type($init_code_offset) `,` type($init_code_length) `,` type($salt) `->` type($result)";
}

def EVM_ReturnOp : EVM_Op<"return", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>, Terminator]> {
  let summary = "Return from the current call with the given return data.";
  let description = [{
    Returns from the current call, providing `length` bytes of return data from
    the EVM memory region starting at `offset`. This op never traps; if the
    return data range is out of bounds, it is treated as zero-padded.
  }];

  let arguments = (ins I256:$offset, I256:$length);

  let assemblyFormat = "$offset `,` $length attr-dict `:` type($offset) `,` type($length)";
}

def EVM_RevertOp : EVM_Op<"revert", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>, Terminator]> {
  let summary = "Revert the current call with the given revert data.";
  let description = [{
    Reverts the current call, providing `length` bytes of revert data from
    the EVM memory region starting at `offset`. This op never traps; if the
    revert data range is out of bounds, it is treated as zero-padded.
  }];

  let arguments = (ins I256:$offset, I256:$length);

  let assemblyFormat = "$offset `,` $length attr-dict `:` type($offset) `,` type($length)";
}

def EVM_InvalidOp : EVM_Op<"invalid", [Terminator]> {
  let summary = "Indicates an invalid instruction.";
  let description = [{
    This op is used to represent invalid instructions. It always traps when executed.
  }];

  let assemblyFormat = "attr-dict";
}

def EVM_SelfdestructOp : EVM_Op<"selfdestruct", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>, Terminator]> {
  let summary = "Transfer the current account's balance to a recipient, and destroy the account if created in the same transaction.";
  let description = [{
    Transfers the entire balance of the current account to the given recipient.
    If the current account was created within the same transaction, it is also
    destroyed (code and storage cleared); otherwise only the balance transfer
    takes place and the account remains intact (EIP-6780). If the recipient does
    not exist, it is created to receive the transferred balance.
  }];

  let arguments = (ins I256:$recipient);

  let assemblyFormat = "$recipient attr-dict `:` type($recipient)";
}

#endif // EVM_OPS_TD