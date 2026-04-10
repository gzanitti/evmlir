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

class EVMEmitter {
public:
  EVMEmitter(const mlir::DenseMap<mlir::Value, ValueLocation> &layout,
             const ForkSpec &spec, BytecodeStream &stream,
             LivenessInfo &liveness)
      : layout(layout), stream(stream), liveness(liveness), spec(spec){};

  std::vector<uint8_t> emitModule(mlir::ModuleOp module,
                                  DispatcherStrategy &dispatcher);

private:
  void emitFunction(mlir::func::FuncOp func);
  void emitBlock(mlir::Block &block);
  void emitOp(mlir::Operation &op);
  void emitCall(mlir::func::CallOp &call);
  void emitBranch(mlir::cf::BranchOp &br);
  void emitCondBranch(mlir::cf::CondBranchOp &condBr);
  void emitReturn(mlir::func::ReturnOp &ret);
  void emitConstant(mlir::arith::ConstantOp &constOp);
  void emitValue(mlir::Value v, mlir::Operation *currentOp);
  void emitRecompute(mlir::Operation *op);
  void scheduleOperands(mlir::Operation &op);

  const mlir::DenseMap<mlir::Value, ValueLocation> &layout;
  mlir::DenseMap<mlir::Block *, LabelID> blockLabels;
  BytecodeStream &stream;
  StackTracker stackTracker;
  LivenessInfo &liveness;
  bool hasCallingConvention = false;
  const ForkSpec &spec;
};
