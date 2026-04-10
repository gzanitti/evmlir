
#include "EVMEmitter.h"
#include <iostream>

std::vector<uint8_t> EVMEmitter::emitFunction(mlir::func::FuncOp func) {

  for (auto &block : func.getBody())
    blockLabels[&block] = stream.createLabel();

  llvm::ReversePostOrderTraversal<mlir::Region *> rpo(&func.getBody());
  for (auto *block : rpo)
    emitBlock(*block);

  return stream.finalize();
}

void EVMEmitter::emitBlock(mlir::Block &block) {
  if (!block.isEntryBlock()) {
    stream.defineLabel(blockLabels[&block]);
    stream.emit(Opcode::JUMPDEST);
  }
  for (auto &op : block)
    emitOp(op);
}

void EVMEmitter::emitOp(mlir::Operation &op) {
  if (auto brOp = mlir::dyn_cast<mlir::cf::BranchOp>(op))
    return emitBranch(brOp);
  if (auto condBrOp = mlir::dyn_cast<mlir::cf::CondBranchOp>(op))
    return emitCondBranch(condBrOp);
  if (auto returnOp = mlir::dyn_cast<mlir::func::ReturnOp>(op))
    return emitReturn(returnOp);
  if (auto constOp = mlir::dyn_cast<mlir::arith::ConstantOp>(op))
    return emitConstant(constOp);

  scheduleOperands(op);
  auto opcodeOpt = getOpcode(&op);
  if (!opcodeOpt)
    return;

  stream.emit(*opcodeOpt);

  for (unsigned i = 0; i < op.getOperands().size(); ++i)
    stackTracker.pop();

  if (op.getNumResults() > 0) {
    auto result = op.getResult(0);
    auto it = layout.find(result);
    if (it != layout.end() && std::holds_alternative<SpilledLoc>(it->second)) {
      auto offset = std::get<SpilledLoc>(it->second).memOffset;
      stream.emitPush(offset);
      stream.emit(Opcode::SWAP1);
      stream.emit(Opcode::MSTORE);
    } else {
      stackTracker.push(result);
    }
  }
}

void EVMEmitter::scheduleOperands(mlir::Operation &op) {
  for (auto operand : op.getOperands())
    emitValue(operand, &op);
}

void EVMEmitter::emitValue(mlir::Value v, mlir::Operation *currentOp) {
  auto it = layout.find(v);
  if (it == layout.end())
    return; // TODO: error handling
  const ValueLocation &loc = it->second;
  if (std::holds_alternative<StackLoc>(loc)) {
    auto depth = stackTracker.findDepth(v);

    if (!depth)
      return; // TODO: error handling

    if (*depth > 0 && !liveness.isDeadAfter(v, currentOp)) {
      stackTracker.dup(*depth);
      stream.emit(
          static_cast<Opcode>(static_cast<uint8_t>(Opcode::DUP1) + *depth));
    }
  } else if (std::holds_alternative<SpilledLoc>(loc)) {
    uint32_t memOffset = std::get<SpilledLoc>(loc).memOffset;
    stream.emitPush(memOffset);
    stream.emit(Opcode::MLOAD);
    stackTracker.push(v);
  } else if (std::holds_alternative<RecomputeLoc>(loc)) {
    emitRecompute(std::get<RecomputeLoc>(loc).op);
  }
}

void EVMEmitter::emitRecompute(mlir::Operation *op) {
  scheduleOperands(*op);

  auto opcodeOpt = getOpcode(op);
  if (!opcodeOpt)
    return;
  stream.emit(*opcodeOpt);

  for (auto _ : op->getOperands())
    stackTracker.pop();

  if (op->getNumResults() > 0) {
    auto result = op->getResult(0);
    auto it = layout.find(result);
    if (it != layout.end() && std::holds_alternative<SpilledLoc>(it->second)) {
      auto offset = std::get<SpilledLoc>(it->second).memOffset;
      stream.emitPush(offset);
      stream.emit(Opcode::SWAP1);
      stream.emit(Opcode::MSTORE);
    } else {
      stackTracker.push(result);
    }
  }
}

void EVMEmitter::emitConstant(mlir::arith::ConstantOp &constOp) {
  auto intAttr = mlir::dyn_cast<mlir::IntegerAttr>(constOp.getValue());
  if (!intAttr)
    return; // TODO: error handling
  llvm::APInt value = intAttr.getValue();
  stackTracker.push(constOp.getResult());
  stream.emitPush(value);
}

void EVMEmitter::emitBranch(mlir::cf::BranchOp &br) {
  for (auto arg : br.getDestOperands())
    emitValue(arg, br);

  stream.emitJumpTarget(blockLabels[br.getDest()]);
  stream.emit(Opcode::JUMP);
}

void EVMEmitter::emitCondBranch(mlir::cf::CondBranchOp &condBr) {
  emitValue(condBr.getCondition(), condBr);

  for (auto arg : condBr.getTrueDestOperands())
    emitValue(arg, condBr);
  stream.emitJumpTarget(blockLabels[condBr.getTrueDest()]);
  stream.emit(Opcode::JUMPI);

  for (auto arg : condBr.getFalseDestOperands())
    emitValue(arg, condBr);
  stream.emitJumpTarget(blockLabels[condBr.getFalseDest()]);
  stream.emit(Opcode::JUMP);
}

void EVMEmitter::emitReturn(mlir::func::ReturnOp &ret) {
  auto func = ret->getParentOfType<mlir::func::FuncOp>();
  auto visAttr = func->getAttrOfType<mlir::IntegerAttr>("evm.visibility");
  bool isInternal = static_cast<evmlir::evm::Visibility>(visAttr.getInt()) ==
                    evmlir::evm::Visibility::Internal;

  for (auto value : ret.getOperands())
    emitValue(value, ret);

  if (isInternal) {
    uint8_t depth = ret.getNumOperands();
    if (depth > 0)
      stream.emit(
          static_cast<Opcode>(static_cast<uint8_t>(Opcode::SWAP1) + depth - 1));
    stream.emit(Opcode::JUMP);
  } else {
    llvm_unreachable(
        "external function return: ABI encoding not yet implemented");
  }
}