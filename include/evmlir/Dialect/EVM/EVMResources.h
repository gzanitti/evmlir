#ifndef EVMLIR_DIALECT_EVM_IR_EVMRESOURCES_H
#define EVMLIR_DIALECT_EVM_IR_EVMRESOURCES_H

#include "mlir/Interfaces/SideEffectInterfaces.h"

namespace mlir::evm {

/// Persistent contract storage (SLOAD / SSTORE).
/// Survives across transactions.
struct StorageResource
    : public mlir::SideEffects::Resource::Base<StorageResource> {
  llvm::StringRef getName() final { return "StorageResource"; }
};

/// Transient storage (TLOAD / TSTORE, EIP-1153).
/// Same key-value interface as persistent storage, but cleared at end-of-tx.
struct TransientStorageResource
    : public mlir::SideEffects::Resource::Base<TransientStorageResource> {
  llvm::StringRef getName() final { return "TransientStorageResource"; }
};

/// The EVM's byte-addressable volatile memory region (MLOAD / MSTORE /
/// MSTORE8).
struct MemoryResource
    : public mlir::SideEffects::Resource::Base<MemoryResource> {
  llvm::StringRef getName() final { return "MemoryResource"; }
};

/// EVM's logging mechanism (LOG0-LOG4).
struct LogResource : public mlir::SideEffects::Resource::Base<LogResource> {
  llvm::StringRef getName() final { return "LogResource"; }
};

/// The EVM's return data buffer, which is used to store data returned by
/// external calls
struct ReturnDataResource
    : public mlir::SideEffects::Resource::Base<ReturnDataResource> {
  llvm::StringRef getName() final { return "ReturnDataResource"; }
};

// /// The EVM's read-only calldata region, which is only accessible during
// /// contract execution and cannot be modified.
// struct CalldataResource
//     : public mlir::SideEffects::Resource::Base<CalldataResource> {
//   llvm::StringRef getName() final { return "CalldataResource"; }
// };

} // namespace mlir::evm

#endif // EVMLIR_DIALECT_EVM_IR_EVMRESOURCES_H