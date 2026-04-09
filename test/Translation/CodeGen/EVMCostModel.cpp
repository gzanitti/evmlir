

#include "CodeGen/EVMCostModel.h"
#include "CodeGen/ForkSpec.h"

#include "evmlir/Dialect/EVM/EVMDialect.h"
#include "evmlir/Dialect/EVM/EVMOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/MLIRContext.h"
#include "llvm/ADT/DenseSet.h"
#include <gtest/gtest.h>

struct EVMCostModelTest : public ::testing::Test {
  mlir::MLIRContext ctx;
  mlir::OpBuilder builder{&ctx};

  void SetUp() override {
    ctx.loadDialect<mlir::func::FuncDialect>();
    ctx.loadDialect<mlir::arith::ArithDialect>();
    ctx.loadDialect<evmlir::evm::EVMDialect>();
  }

  mlir::Type i256() { return mlir::IntegerType::get(&ctx, 256); }
  mlir::Location loc() { return mlir::UnknownLoc::get(&ctx); }

  mlir::func::FuncOp makeFunc(llvm::StringRef name,
                              mlir::TypeRange inputTypes = {}) {
    auto fnType = mlir::FunctionType::get(&ctx, inputTypes, {});
    auto fn = mlir::func::FuncOp::create(loc(), name, fnType);
    fn.addEntryBlock();
    builder.setInsertionPointToEnd(&fn.getBody().front());
    return fn;
  }

  mlir::Value constant256(int64_t val) {
    return builder.create<mlir::arith::ConstantIntOp>(loc(), val, 256)
        .getResult();
  }

  mlir::Value add256(mlir::Value a, mlir::Value b) {
    return builder.create<mlir::arith::AddIOp>(loc(), a, b).getResult();
  }

  mlir::Value evmCaller() {
    return builder.create<evmlir::evm::CallerOp>(loc(), i256()).getResult();
  }

  mlir::Value evmSload(mlir::Value slot) {
    return builder.create<evmlir::evm::SloadOp>(loc(), i256(), slot)
        .getResult();
  }

  mlir::Value evmBlockhash(mlir::Value blockNum) {
    return builder.create<evmlir::evm::BlockhashOp>(loc(), i256(), blockNum)
        .getResult();
  }

  llvm::DenseSet<mlir::Value>
  makeStack(std::initializer_list<mlir::Value> vals) {
    llvm::DenseSet<mlir::Value> s;
    for (auto v : vals)
      s.insert(v);
    return s;
  }
};

TEST_F(EVMCostModelTest, Reject_BlockArgument) {
  auto fn = makeFunc("test", {i256()});
  mlir::Value arg = fn.getArgument(0);

  EVMCostModel model(OsakaSpec);
  auto cost = model.recomputeCost(arg, {}, /*nextSlotIsWarm=*/true,
                                  /*memExpansionCost=*/0);

  EXPECT_FALSE(cost.has_value());
}

TEST_F(EVMCostModelTest, Reject_SideEffects_Sload) {
  makeFunc("test");
  mlir::Value slot = constant256(0);
  mlir::Value result = evmSload(slot);

  EVMCostModel model(OsakaSpec);
  auto cost = model.recomputeCost(result, {}, /*nextSlotIsWarm=*/true,
                                  /*memExpansionCost=*/0);

  EXPECT_FALSE(cost.has_value());
}

TEST_F(EVMCostModelTest, Reject_MaxDepth) {
  makeFunc("test");
  mlir::Value a = constant256(1), b = constant256(2);
  mlir::Value c = constant256(3), d = constant256(4);
  mlir::Value e = constant256(5), f = constant256(6);
  mlir::Value g = constant256(7);

  mlir::Value add1 = add256(a, b);
  mlir::Value add2 = add256(add1, c);
  mlir::Value add3 = add256(add2, d);
  mlir::Value add4 = add256(add3, e);
  mlir::Value add5 = add256(add4, f);
  mlir::Value add6 = add256(add5, g);

  auto stack = makeStack({a, b, c, d, e, f, g});

  EVMCostModel model(OsakaSpec);
  auto cost = model.recomputeCost(add6, stack, /*nextSlotIsWarm=*/false,
                                  /*memExpansionCost=*/100);

  EXPECT_FALSE(cost.has_value());
}

TEST_F(EVMCostModelTest, Reject_EconomicPruning_ExpensivePureOp) {
  makeFunc("test");
  mlir::Value blockNum = constant256(100);
  mlir::Value result = evmBlockhash(blockNum);

  auto stack = makeStack({blockNum});

  EVMCostModel model(OsakaSpec);
  auto cost = model.recomputeCost(result, stack, /*nextSlotIsWarm=*/true,
                                  /*memExpansionCost=*/0);

  EXPECT_FALSE(cost.has_value());
}

TEST_F(EVMCostModelTest, Reject_TwoChainAdd_WarmThreshold) {
  makeFunc("test");
  mlir::Value a = constant256(1), b = constant256(2), c = constant256(3);
  mlir::Value inner = add256(a, b);
  mlir::Value outer = add256(inner, c);

  auto stack = makeStack({a, b, c});

  EVMCostModel model(OsakaSpec);
  auto cost = model.recomputeCost(outer, stack, /*nextSlotIsWarm=*/true,
                                  /*memExpansionCost=*/0);

  EXPECT_FALSE(cost.has_value());
}

TEST_F(EVMCostModelTest, Cost_ValueAlreadyOnStack_IsZero) {
  makeFunc("test");
  mlir::Value v = evmCaller();

  auto stack = makeStack({v});

  EVMCostModel model(OsakaSpec);
  auto cost = model.recomputeCost(v, stack, /*nextSlotIsWarm=*/true,
                                  /*memExpansionCost=*/0);

  ASSERT_TRUE(cost.has_value());
  EXPECT_EQ(*cost, 0u);
}

TEST_F(EVMCostModelTest, Cost_SimpleOpNoOperands_Caller) {
  makeFunc("test");
  mlir::Value v = evmCaller();

  EVMCostModel model(OsakaSpec);
  auto cost = model.recomputeCost(v, {}, /*nextSlotIsWarm=*/true,
                                  /*memExpansionCost=*/0);

  ASSERT_TRUE(cost.has_value());
  EXPECT_EQ(*cost, 2u); // staticGas(CALLER) = 2
}

TEST_F(EVMCostModelTest, Cost_OperandsInStack_SingleAdd) {
  makeFunc("test");
  mlir::Value a = constant256(1), b = constant256(2);
  mlir::Value result = add256(a, b);

  auto stack = makeStack({a, b});

  EVMCostModel model(OsakaSpec);
  auto cost = model.recomputeCost(result, stack, /*nextSlotIsWarm=*/true,
                                  /*memExpansionCost=*/0);

  ASSERT_TRUE(cost.has_value());
  EXPECT_EQ(*cost, 3u); // staticGas(ADD) = 3
}

TEST_F(EVMCostModelTest, Cost_TwoLevelChain) {
  makeFunc("test");
  mlir::Value a = constant256(1), b = constant256(2), c = constant256(3);
  mlir::Value inner = add256(a, b);
  mlir::Value outer = add256(inner, c);

  auto stack = makeStack({a, b, c});

  EVMCostModel model(OsakaSpec);
  auto cost = model.recomputeCost(outer, stack, /*nextSlotIsWarm=*/false,
                                  /*memExpansionCost=*/1);

  ASSERT_TRUE(cost.has_value());
  EXPECT_EQ(*cost, 6u); // staticGas(ADD) + staticGas(ADD) = 3 + 3
}
