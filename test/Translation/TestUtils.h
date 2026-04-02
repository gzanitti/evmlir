#pragma once

#include "Analysis/InterferenceGraph.h"
#include "Analysis/LivenessInfo.h"

#include "gtest/gtest.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"

struct TranslationTest : public ::testing::Test {
  mlir::MLIRContext ctx;
  mlir::OpBuilder builder{&ctx};

  void SetUp() override {
    ctx.loadDialect<mlir::func::FuncDialect>();
    ctx.loadDialect<mlir::arith::ArithDialect>();
    ctx.loadDialect<mlir::cf::ControlFlowDialect>();
  }

  mlir::Type i32() { return mlir::IntegerType::get(&ctx, 32); }
  mlir::Location loc() { return mlir::UnknownLoc::get(&ctx); }

  // Creates a FuncOp with an entry block and positions the builder at its end.
  mlir::func::FuncOp makeFunc(llvm::StringRef name,
                               mlir::TypeRange inputTypes = {},
                               mlir::TypeRange resultTypes = {}) {
    auto fnType = mlir::FunctionType::get(&ctx, inputTypes, resultTypes);
    auto fn = mlir::func::FuncOp::create(loc(), name, fnType);
    fn.addEntryBlock();
    builder.setInsertionPointToEnd(&fn.getBody().front());
    return fn;
  }

  mlir::Value constant(int64_t val) {
    return builder.create<mlir::arith::ConstantIntOp>(loc(), val, 32)
                 .getResult();
  }

  mlir::Value add(mlir::Value a, mlir::Value b) {
    return builder.create<mlir::arith::AddIOp>(loc(), a, b).getResult();
  }

  void ret(mlir::ValueRange vals = {}) {
    builder.create<mlir::func::ReturnOp>(loc(), vals);
  }

  mlir::Block *addBlock(mlir::func::FuncOp fn, mlir::TypeRange argTypes = {}) {
    auto *block = new mlir::Block();
    for (auto t : argTypes)
      block->addArgument(t, loc());
    fn.getBody().push_back(block);
    return block;
  }

  void setInsertionPointToEnd(mlir::Block *block) {
    builder.setInsertionPointToEnd(block);
  }

  void branch(mlir::Block *dest, mlir::ValueRange args = {}) {
    builder.create<mlir::cf::BranchOp>(loc(), dest, args);
  }

  // Returns true if a and b are neighbors in g, excluding self-loops.
  bool interferes(const InterferenceGraph &g, mlir::Value a, mlir::Value b) {
    if (a == b)
      return false;
    auto *n = g.neighbors(a);
    return n && n->count(b);
  }

  // Counts real (non-self-loop) neighbors of v.
  int realNeighborCount(const InterferenceGraph &g, mlir::Value v) {
    auto *n = g.neighbors(v);
    if (!n)
      return 0;
    int count = 0;
    for (auto nb : *n)
      if (nb != v)
        count++;
    return count;
  }
};
