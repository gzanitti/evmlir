
#include "EVMEmitter.h"
#include "BytecodeStream.h"
#include "evmlir/Dialect/EVM/EVMDialect.h"
#include <cstdint>

static mlir::LogicalResult verifyModule(mlir::ModuleOp module) {
  mlir::func::FuncOp constructor;
  mlir::func::FuncOp fallback;
  bool failed = false;

  module.walk([&](mlir::func::FuncOp func) {
    auto kindAttr = func->getAttrOfType<mlir::IntegerAttr>("evm.kind");
    if (!kindAttr)
      return;

    auto kind = static_cast<evmlir::evm::FunctionKind>(kindAttr.getInt());

    if (kind == evmlir::evm::FunctionKind::Constructor) {
      if (constructor) {
        auto diag =
            module.emitError("module contains more than one constructor");
        diag.attachNote(constructor.getLoc())
            << "first constructor '" << constructor.getName()
            << "' defined here";
        diag.attachNote(func.getLoc())
            << "second constructor '" << func.getName() << "' defined here";
        failed = true;
      }
      constructor = func;
    }

    if (kind == evmlir::evm::FunctionKind::Fallback) {
      if (fallback) {
        auto diag = module.emitError("module contains more than one fallback");
        diag.attachNote(fallback.getLoc())
            << "first fallback '" << fallback.getName() << "' defined here";
        diag.attachNote(func.getLoc())
            << "second fallback '" << func.getName() << "' defined here";
        failed = true;
      }
      fallback = func;
    }
  });

  return mlir::failure(failed);
}
mlir::FailureOr<EmittedContract>
EVMEmitter::emitModule(mlir::ModuleOp module, DispatcherStrategy &dispatcher) {

  if (mlir::failed(verifyModule(module)))
    return mlir::failure();

  BytecodeStream runtimeStream;
  BytecodeStream deployStream;
  emitRuntime(module, runtimeStream, dispatcher);
  auto runtimeBytecode = runtimeStream.finalize();
  emitDeploy(module, deployStream, runtimeBytecode.size());
  auto deployBytecode = deployStream.finalize();

  return EmittedContract{deployBytecode, runtimeBytecode};
}

void EVMEmitter::emitDeploy(mlir::ModuleOp module, BytecodeStream &stream,
                            uint32_t runtimeSize) {
  auto constructor = findConstructor(module);
  if (constructor)
    emitConstructor(constructor, stream);

  LabelID runtimeStart = stream.createLabel();

  // CODECOPY(destOffset=TOS, codeOffset, length=deepest)
  stream.emitPush(uint32_t(runtimeSize));
  stream.emitJumpTarget(runtimeStart);
  stream.emitPush(uint32_t(0));
  stream.emit(Opcode::CODECOPY);

  // RETURN(offset=TOS, size=deepest)
  stream.emitPush(uint32_t(runtimeSize));
  stream.emitPush(uint32_t(0));
  stream.emit(Opcode::RETURN);

  stream.defineLabel(runtimeStart);
}

mlir::func::FuncOp EVMEmitter::findConstructor(mlir::ModuleOp module) {
  for (auto func : module.getOps<mlir::func::FuncOp>()) {
    auto kindAttr = func->getAttrOfType<mlir::IntegerAttr>("evm.kind");
    if (!kindAttr)
      continue;
    if (static_cast<evmlir::evm::FunctionKind>(kindAttr.getInt()) ==
        evmlir::evm::FunctionKind::Constructor)
      return func;
  }
  return nullptr;
}

mlir::func::FuncOp EVMEmitter::findFallback(mlir::ModuleOp module) {
  for (auto func : module.getOps<mlir::func::FuncOp>()) {
    auto kindAttr = func->getAttrOfType<mlir::IntegerAttr>("evm.kind");
    if (!kindAttr)
      continue;
    if (static_cast<evmlir::evm::FunctionKind>(kindAttr.getInt()) ==
        evmlir::evm::FunctionKind::Fallback)
      return func;
  }
  return nullptr;
}

void EVMEmitter::emitRuntime(mlir::ModuleOp module, BytecodeStream &stream,
                             DispatcherStrategy &dispatcher) {
  llvm::SmallVector<std::tuple<uint32_t, LabelID, mlir::func::FuncOp>> entries;
  mlir::func::FuncOp fallbackFn;

  for (auto func : module.getOps<mlir::func::FuncOp>()) {
    // Pre-allocate labels for all blocks.
    for (auto &block : func.getBody())
      blockLabels[&block] = stream.createLabel();

    // Collect external function entries for dispatcher.
    auto visAttr = func->getAttrOfType<mlir::IntegerAttr>("evm.visibility");
    auto kindAttr = func->getAttrOfType<mlir::IntegerAttr>("evm.kind");
    auto selAttr = func->getAttrOfType<mlir::IntegerAttr>("evm.selector");
    if (visAttr && kindAttr && selAttr) {
      auto vis = static_cast<evmlir::evm::Visibility>(visAttr.getInt());
      auto kind = static_cast<evmlir::evm::FunctionKind>(kindAttr.getInt());
      if (kind == evmlir::evm::FunctionKind::Fallback)
        fallbackFn = func;

      if (vis == evmlir::evm::Visibility::External &&
          kind == evmlir::evm::FunctionKind::Function)
        entries.push_back({static_cast<uint32_t>(selAttr.getInt()),
                           blockLabels[&func.getBody().front()], func});
    }
  }

  std::optional<LabelID> fallbackLabel;
  if (fallbackFn)
    fallbackLabel = blockLabels[&fallbackFn.getBody().front()];

  dispatcher.emitSelectorLoad(stream);
  dispatcher.emit(entries, stream, fallbackLabel);

  for (auto func : module.getOps<mlir::func::FuncOp>())
    emitFunction(func, stream);
}

void EVMEmitter::emitConstructor(mlir::func::FuncOp func,
                                 BytecodeStream &stream) {
  stackTracker.reset();
  llvm::ReversePostOrderTraversal<mlir::Region *> rpo(&func.getBody());
  for (auto *block : rpo) {
    if (!block->isEntryBlock()) {
      stream.defineLabel(blockLabels[block]);
      stream.emit(Opcode::JUMPDEST);
    }
    for (auto &op : *block)
      emitOp(op, stream);
  }
}

void EVMEmitter::emitFunction(mlir::func::FuncOp func, BytecodeStream &stream) {
  stackTracker.reset();
  llvm::ReversePostOrderTraversal<mlir::Region *> rpo(&func.getBody());
  for (auto *block : rpo)
    emitBlock(*block, stream);
}

void EVMEmitter::emitBlock(mlir::Block &block, BytecodeStream &stream) {
  stream.defineLabel(blockLabels[&block]);
  stream.emit(Opcode::JUMPDEST);
  for (auto &op : block)
    emitOp(op, stream);
}

void EVMEmitter::emitOp(mlir::Operation &op, BytecodeStream &stream) {
  if (auto brOp = mlir::dyn_cast<mlir::cf::BranchOp>(op))
    return emitBranch(brOp, stream);
  if (auto condBrOp = mlir::dyn_cast<mlir::cf::CondBranchOp>(op))
    return emitCondBranch(condBrOp, stream);
  if (auto returnOp = mlir::dyn_cast<mlir::func::ReturnOp>(op))
    return emitReturn(returnOp, stream);
  if (auto constOp = mlir::dyn_cast<mlir::arith::ConstantOp>(op))
    return emitConstant(constOp, stream);
  if (auto callOp = mlir::dyn_cast<mlir::func::CallOp>(op))
    return emitCall(callOp, stream);

  scheduleOperands(op, stream);
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

void EVMEmitter::scheduleOperands(mlir::Operation &op, BytecodeStream &stream) {
  for (auto operand : llvm::reverse(op.getOperands()))
    emitValue(operand, &op, stream);
}

void EVMEmitter::emitValue(mlir::Value v, mlir::Operation *currentOp,
                           BytecodeStream &stream) {
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
    emitRecompute(std::get<RecomputeLoc>(loc).op, stream);
  }
}

void EVMEmitter::emitRecompute(mlir::Operation *op, BytecodeStream &stream) {
  scheduleOperands(*op, stream);

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

void EVMEmitter::emitCall(mlir::func::CallOp &call, BytecodeStream &stream) {
  // Look up the callee function in the module.
  auto module = call->getParentOfType<mlir::ModuleOp>();
  auto callee = mlir::cast<mlir::func::FuncOp>(
      mlir::SymbolTable::lookupNearestSymbolFrom(module, call.getCalleeAttr()));

  auto operands = call.getOperands();
  for (auto operand : llvm::reverse(operands))
    emitValue(operand, call, stream);

  LabelID retLabel = stream.createLabel();
  stream.emitJumpTarget(retLabel);
  stream.emitJumpTarget(blockLabels[&callee.getBody().front()]);
  stream.emit(Opcode::JUMP);

  stream.defineLabel(retLabel);
  stream.emit(Opcode::JUMPDEST);

  // Results are now on top of the stack.
  for (auto result : call.getResults())
    stackTracker.push(result);
}

void EVMEmitter::emitConstant(mlir::arith::ConstantOp &constOp,
                              BytecodeStream &stream) {
  auto intAttr = mlir::dyn_cast<mlir::IntegerAttr>(constOp.getValue());
  if (!intAttr)
    return; // TODO: error handling
  llvm::APInt value = intAttr.getValue();
  stackTracker.push(constOp.getResult());
  stream.emitPush(value);
}

void EVMEmitter::emitBranch(mlir::cf::BranchOp &br, BytecodeStream &stream) {
  for (auto arg : br.getDestOperands())
    emitValue(arg, br, stream);

  stream.emitJumpTarget(blockLabels[br.getDest()]);
  stream.emit(Opcode::JUMP);
}

void EVMEmitter::emitCondBranch(mlir::cf::CondBranchOp &condBr,
                                BytecodeStream &stream) {
  emitValue(condBr.getCondition(), condBr, stream);

  for (auto arg : condBr.getTrueDestOperands())
    emitValue(arg, condBr, stream);
  stream.emitJumpTarget(blockLabels[condBr.getTrueDest()]);
  stream.emit(Opcode::JUMPI);

  for (auto arg : condBr.getFalseDestOperands())
    emitValue(arg, condBr, stream);
  stream.emitJumpTarget(blockLabels[condBr.getFalseDest()]);
  stream.emit(Opcode::JUMP);
}

void EVMEmitter::emitReturn(mlir::func::ReturnOp &ret, BytecodeStream &stream) {
  auto func = ret->getParentOfType<mlir::func::FuncOp>();
  auto kindAttr = func->getAttrOfType<mlir::IntegerAttr>("evm.kind");
  if (kindAttr && static_cast<evmlir::evm::FunctionKind>(kindAttr.getInt()) ==
                      evmlir::evm::FunctionKind::Constructor)
    return;

  auto visAttr = func->getAttrOfType<mlir::IntegerAttr>("evm.visibility");
  bool isInternal = static_cast<evmlir::evm::Visibility>(visAttr.getInt()) ==
                    evmlir::evm::Visibility::Internal;

  for (auto value : ret.getOperands())
    emitValue(value, ret, stream);

  if (isInternal) {
    uint8_t depth = ret.getNumOperands();
    if (depth > 0)
      stream.emit(
          static_cast<Opcode>(static_cast<uint8_t>(Opcode::SWAP1) + depth - 1));
    stream.emit(Opcode::JUMP);
  } else {
    uint32_t numValues = ret.getNumOperands();
    for (uint32_t i = 0; i < numValues; ++i) {
      uint32_t offset = (numValues - 1 - i) * 32;
      stream.emitPush(offset);
      // stackTracker.push(mlir::Value{}); // offset placeholder
      stream.emit(Opcode::MSTORE);
      // stackTracker.pop(); // offset
      stackTracker.pop(); // value
    }
    stream.emitPush(uint32_t(numValues * 32));
    stream.emitPush(uint32_t(0));
    stream.emit(Opcode::RETURN);
  }
}