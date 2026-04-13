#include "../CodeGen/BinarySearchDispatcher.h"
#include "../StackAllocation/StackAllocator.h"
#include "BytecodeStream.h"
#include "ForkSpec.h"
#include "StackTracker.h"
#include "evmlir/Dialect/EVM/EVMDialect.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/IR/Block.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/RegionGraphTraits.h"
#include "llvm/ADT/PostOrderIterator.h"

struct EmittedContract {
  std::vector<uint8_t> deployBytecode;
  std::vector<uint8_t> runtimeBytecode;
};

class EVMEmitter {
public:
  EVMEmitter(const mlir::DenseMap<mlir::Value, ValueLocation> &layout,
             const ForkSpec &spec, BytecodeStream &stream,
             LivenessInfo &liveness)
      : layout(layout), stream(stream), liveness(liveness), spec(spec){};

  EmittedContract emitModule(mlir::ModuleOp module,
                             DispatcherStrategy &dispatcher);

private:
  void emitRuntime(mlir::ModuleOp module, BytecodeStream &stream,
                   DispatcherStrategy &dispatcher);
  void emitDeploy(mlir::ModuleOp module, BytecodeStream &stream,
                  uint32_t runtimeSize);
  void emitFunction(mlir::func::FuncOp func, BytecodeStream &stream);
  void emitConstructor(mlir::func::FuncOp func, BytecodeStream &stream);
  void emitBlock(mlir::Block &block, BytecodeStream &stream);
  void emitOp(mlir::Operation &op, BytecodeStream &stream);
  void emitCall(mlir::func::CallOp &call, BytecodeStream &stream);
  void emitBranch(mlir::cf::BranchOp &br, BytecodeStream &stream);
  void emitCondBranch(mlir::cf::CondBranchOp &condBr, BytecodeStream &stream);
  void emitReturn(mlir::func::ReturnOp &ret, BytecodeStream &stream);
  void emitConstant(mlir::arith::ConstantOp &constOp, BytecodeStream &stream);
  void emitValue(mlir::Value v, mlir::Operation *currentOp,
                 BytecodeStream &stream);
  void emitRecompute(mlir::Operation *op, BytecodeStream &stream);
  void scheduleOperands(mlir::Operation &op, BytecodeStream &stream);
  mlir::func::FuncOp findConstructor(mlir::ModuleOp module);

  const mlir::DenseMap<mlir::Value, ValueLocation> &layout;
  mlir::DenseMap<mlir::Block *, LabelID> blockLabels;
  BytecodeStream &stream;
  StackTracker stackTracker;
  LivenessInfo &liveness;
  bool hasCallingConvention = false;
  const ForkSpec &spec;
};
