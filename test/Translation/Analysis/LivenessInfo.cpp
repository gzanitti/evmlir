#include "TestUtils.h"

using LivenessInfoTest = TranslationTest;

// An SSA value defined but never used as an operand should have useCount 0.
TEST_F(LivenessInfoTest, UnusedValue_UseCountIsZero) {
  auto fn = makeFunc("test");
  mlir::Value a = constant(42);
  ret();

  LivenessInfo info(fn);
  EXPECT_EQ(info.getUseCount(a), 0);
}

// A value used exactly once has useCount 1.
TEST_F(LivenessInfoTest, SingleUse_UseCountIsOne) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  ret({a});

  LivenessInfo info(fn);
  EXPECT_EQ(info.getUseCount(a), 1);
}

// When a value appears twice in the operands of one op, useCount is 2.
TEST_F(LivenessInfoTest, DoubleUse_UseCountIsTwo) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(21);
  mlir::Value b = add(a, a);
  ret({b});

  LivenessInfo info(fn);
  EXPECT_EQ(info.getUseCount(a), 2);
}

// Uses accumulate across multiple ops: addi uses a twice, then addi uses a
// once more → total 3.
TEST_F(LivenessInfoTest, MultipleOpsContribute) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = add(a, a);
  mlir::Value c = add(a, b);
  ret({c});

  LivenessInfo info(fn);
  EXPECT_EQ(info.getUseCount(a), 3);
}

// Block arguments are regular Values; their use counts should be tracked.
TEST_F(LivenessInfoTest, BlockArgument_UseCount) {
  auto fn = makeFunc("test", {i32()}, {i32()});
  mlir::Value arg0 = fn.getBody().front().getArgument(0);
  builder.setInsertionPointToEnd(&fn.getBody().front());
  mlir::Value b = add(arg0, arg0);
  ret({b});

  LivenessInfo info(fn);
  EXPECT_EQ(info.getUseCount(arg0), 2);
}

// Each value tracks its own independent count.
TEST_F(LivenessInfoTest, MultipleValuesIndependentCounts) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = add(a, b);
  mlir::Value d = add(b, b);
  ret({add(c, d)});

  LivenessInfo info(fn);
  EXPECT_EQ(info.getUseCount(a), 1);
  EXPECT_EQ(info.getUseCount(b), 3);
}

// func.return counts as a use of its operand.
TEST_F(LivenessInfoTest, ReturnOnlyUsesValue) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(7);
  ret({a}); // only user of a

  LivenessInfo info(fn);
  EXPECT_EQ(info.getUseCount(a), 1);
}

// In a chain of 5 adds, each intermediate result is used exactly once.
TEST_F(LivenessInfoTest, LargeChain_CountsAccurate) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value v = constant(0);
  std::vector<mlir::Value> intermediates;
  for (int i = 1; i <= 5; ++i) {
    mlir::Value next = add(v, constant(i));
    intermediates.push_back(v);
    v = next;
  }
  ret({v});

  LivenessInfo info(fn);
  for (auto val : intermediates)
    EXPECT_EQ(info.getUseCount(val), 1);
}
