#include "evmlir/Dialect/EVM/EVMOps.h"
#include "evmlir/Dialect/EVM/EVMResources.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

using namespace mlir;
using namespace evmlir::evm;

#define GET_OP_CLASSES
#include "evmlir/Dialect/EVM/EVMOps.cpp.inc"

void SloadOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {

  effects.emplace_back(MemoryEffects::Read::get(), getSlot(),
                       mlir::evm::StorageResource::get());
}

void SstoreOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Write::get(), getSlot(),
                       mlir::evm::StorageResource::get());
}

void TloadOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {

  effects.emplace_back(MemoryEffects::Read::get(), getSlot(),
                       mlir::evm::TransientStorageResource::get());
}

void TstoreOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Write::get(), getSlot(),
                       mlir::evm::TransientStorageResource::get());
}

void MloadOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {

  effects.emplace_back(MemoryEffects::Read::get(), getOffset(),
                       mlir::evm::MemoryResource::get());
}

void MstoreOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Write::get(), getOffset(),
                       mlir::evm::MemoryResource::get());
}

void Mstore8Op::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Write::get(), getOffset(),
                       mlir::evm::MemoryResource::get());
}

void MsizeOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
}

void McopyOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::MemoryResource::get());
}

void Keccak256Op::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
}

void CalldataCopyOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::MemoryResource::get());
}

void SelfbalanceOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       SideEffects::DefaultResource::get());
}

void CodecopyOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::MemoryResource::get());
}

void BalanceOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(), getAddress(),
                       SideEffects::DefaultResource::get());
}

void ExtcodesizeOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(), getAddress(),
                       SideEffects::DefaultResource::get());
}

void ExtcodehashOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(), getAddress(),
                       SideEffects::DefaultResource::get());
}

void ExtcodecopyOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       SideEffects::DefaultResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::MemoryResource::get());
}

void ReturndatasizeOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::ReturnDataResource::get());
}

void ReturndatacopyOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::ReturnDataResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::MemoryResource::get());
}

void Log0Op::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(), getDataOffset(),
                       mlir::evm::LogResource::get());
}

void Log1Op::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(), getDataOffset(),
                       mlir::evm::LogResource::get());
}

void Log2Op::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(), getDataOffset(),
                       mlir::evm::LogResource::get());
}

void Log3Op ::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(), getDataOffset(),
                       mlir::evm::LogResource::get());
}

void Log4Op::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(), getDataOffset(),
                       mlir::evm::LogResource::get());
}

void CallOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::ReturnDataResource::get());
  effects.emplace_back(MemoryEffects::Read::get(),
                       SideEffects::DefaultResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       SideEffects::DefaultResource::get());
}

void DelegatecallOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::ReturnDataResource::get());
  effects.emplace_back(MemoryEffects::Read::get(),
                       SideEffects::DefaultResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       SideEffects::DefaultResource::get());
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::StorageResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::StorageResource::get());
}

void StaticcallOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::ReturnDataResource::get());
  effects.emplace_back(MemoryEffects::Read::get(),
                       SideEffects::DefaultResource::get());
}

void CallcodeOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::ReturnDataResource::get());
  effects.emplace_back(MemoryEffects::Read::get(),
                       SideEffects::DefaultResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       SideEffects::DefaultResource::get());
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::StorageResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       mlir::evm::StorageResource::get());
}

void CreateOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       SideEffects::DefaultResource::get());
}

void Create2Op::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       SideEffects::DefaultResource::get());
}

void ReturnOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(), getOffset(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(), getLength(),
                       mlir::evm::ReturnDataResource::get());
}

void RevertOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(), getOffset(),
                       mlir::evm::MemoryResource::get());
  effects.emplace_back(MemoryEffects::Write::get(), getLength(),
                       mlir::evm::ReturnDataResource::get());
}

void SelfdestructOp::getEffects(
    SmallVectorImpl<SideEffects::EffectInstance<MemoryEffects::Effect>>
        &effects) {
  effects.emplace_back(MemoryEffects::Read::get(),
                       SideEffects::DefaultResource::get());
  effects.emplace_back(MemoryEffects::Write::get(),
                       SideEffects::DefaultResource::get());
}