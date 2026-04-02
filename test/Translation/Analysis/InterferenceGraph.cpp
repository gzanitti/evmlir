#include "TestUtils.h"
#include <gtest/gtest.h>
#include <iostream>

using InterferenceGraphTest = TranslationTest;

// Helper: collect all values that appear in the graph.
static llvm::SmallVector<mlir::Value>
collectValues(const InterferenceGraph &g) {
  llvm::SmallVector<mlir::Value> vals;
  for (auto v : g.getValues())
    vals.push_back(v);
  return vals;
}

// --- Two constants used together: they must interfere ---
//
// IR:  %a = const 1
//      %b = const 2
//      %c = addi %a, %b
//      return %c
TEST_F(InterferenceGraphTest, Interference_ConcurrentValues) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = add(a, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph g(liveness, fn);

  EXPECT_TRUE(interferes(g, a, b));
}

// Interference edges are bidirectional.
TEST_F(InterferenceGraphTest, Interference_IsSymmetric) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = add(a, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph g(liveness, fn);

  EXPECT_EQ(interferes(g, a, b), interferes(g, b, a));
}

// IR:  %a = const 1
//      %b = const 2
//      %c = const 3
//      %d = addi %a, %b
//      %e = addi %d, %c
//      return %e
TEST_F(InterferenceGraphTest, Triangle_ThreeMutuallyInterfering) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = constant(3);
  mlir::Value d = add(a, b);
  mlir::Value e = add(d, c);
  ret({e});

  LivenessInfo liveness(fn);
  InterferenceGraph g(liveness, fn);

  // %a: {%b, %c, %d}
  // %b: {%a, %c, %d}
  // %c: {%a, %b, %d, %e}
  // %d: {%a, %b, %c, %e}
  // %e: {%c, %d}
  std::cout << "Neighbors of a: "
            << (g.neighbors(a) ? g.neighbors(a)->size() : 0) << "\n";
  EXPECT_TRUE(interferes(g, a, b));
  EXPECT_TRUE(interferes(g, a, c));
  EXPECT_TRUE(interferes(g, a, d));
  EXPECT_FALSE(interferes(g, a, e));

  std::cout << "Neighbors of b: "
            << (g.neighbors(b) ? g.neighbors(b)->size() : 0) << "\n";
  EXPECT_TRUE(interferes(g, b, a));
  EXPECT_TRUE(interferes(g, b, c));
  EXPECT_TRUE(interferes(g, b, d));
  EXPECT_FALSE(interferes(g, b, e));

  std::cout << "Neighbors of c: "
            << (g.neighbors(c) ? g.neighbors(c)->size() : 0) << "\n";
  EXPECT_TRUE(interferes(g, c, a));
  EXPECT_TRUE(interferes(g, c, b));
  EXPECT_TRUE(interferes(g, c, d));
  EXPECT_TRUE(interferes(g, c, e));

  std::cout << "Neighbors of d: "
            << (g.neighbors(d) ? g.neighbors(d)->size() : 0) << "\n";
  EXPECT_TRUE(interferes(g, d, a));
  EXPECT_TRUE(interferes(g, d, b));
  EXPECT_TRUE(interferes(g, d, c));
  EXPECT_TRUE(interferes(g, d, e));

  std::cout << "Neighbors of e: "
            << (g.neighbors(e) ? g.neighbors(e)->size() : 0) << "\n";
  EXPECT_FALSE(interferes(g, e, a));
  EXPECT_FALSE(interferes(g, e, b));
  EXPECT_TRUE(interferes(g, e, c));
  EXPECT_TRUE(interferes(g, e, d));
}

// No value should be its own neighbor.
TEST_F(InterferenceGraphTest, NoSelfLoop) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = add(a, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph g(liveness, fn);

  for (auto v : collectValues(g)) {
    auto *n = g.neighbors(v);
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->count(v), 0u) << "Value has a self-loop";
  }
}

// After removeNode(%a), none of %a's former neighbors should reference %a.
TEST_F(InterferenceGraphTest, RemoveNode_CleansAllEdges) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = constant(3);
  mlir::Value d = add(a, b);
  mlir::Value e = add(d, c);
  ret({e});

  LivenessInfo liveness(fn);
  InterferenceGraph g(liveness, fn);

  ASSERT_TRUE(interferes(g, a, b));
  ASSERT_TRUE(interferes(g, a, c));

  g.removeNode(a);

  auto *nbB = g.neighbors(b);
  ASSERT_NE(nbB, nullptr);
  EXPECT_EQ(nbB->count(a), 0u);

  auto *nbC = g.neighbors(c);
  ASSERT_NE(nbC, nullptr);
  EXPECT_EQ(nbC->count(a), 0u);

  EXPECT_EQ(g.neighbors(a), nullptr);
}

// Calling removeNode twice on the same value must not crash.
TEST_F(InterferenceGraphTest, RemoveNode_Idempotent) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = add(a, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph g(liveness, fn);

  g.removeNode(a);
  EXPECT_NO_FATAL_FAILURE(g.removeNode(a));
}

// neighbors() returns nullptr for a value that was never added to the graph.
TEST_F(InterferenceGraphTest, Neighbors_NullForUnknownValue) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = add(a, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph g(liveness, fn);

  mlir::Value unrelated = constant(99);
  EXPECT_EQ(g.neighbors(unrelated), nullptr);
}

// Values with no real interference neighbors don't appear in the graph
TEST_F(InterferenceGraphTest, IsolatedValues_NotInGraph) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = add(b, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph g(liveness, fn);

  EXPECT_FALSE(interferes(g, a, b));
  EXPECT_FALSE(interferes(g, a, c));

  EXPECT_TRUE(interferes(g, b, c));
  EXPECT_TRUE(interferes(g, c, b));
}

// getValues() returns all values that have at least one edge.
TEST_F(InterferenceGraphTest, GetValues_ContainsAllInterferingNodes) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = constant(3);
  mlir::Value d = add(a, b);
  mlir::Value e = add(d, c);
  ret({e});

  LivenessInfo liveness(fn);
  InterferenceGraph g(liveness, fn);

  auto vals = collectValues(g);
  EXPECT_NE(std::find(vals.begin(), vals.end(), a), vals.end());
  EXPECT_NE(std::find(vals.begin(), vals.end(), b), vals.end());
  EXPECT_NE(std::find(vals.begin(), vals.end(), c), vals.end());
}

// --- Two values both live-out from entry to a successor block interfere ---
//
// IR:  entry:
//        %a = const 1
//        %b = const 2
//        br successor
//      successor:
//        %c = addi %a, %b
//        return %c
TEST_F(InterferenceGraphTest, MultiBlock_LiveOutPropagatesInterference) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);

  mlir::Block *successor = addBlock(fn);
  branch(successor);

  setInsertionPointToEnd(successor);
  mlir::Value c = add(a, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph g(liveness, fn);

  EXPECT_TRUE(interferes(g, a, b));
}

// --- Block argument interferes with other live-in values ---
//
// IR:  entry:
//        %a = const 1
//        %b = const 2
//        br successor(%a)
//      successor(%x : i32):
//        %c = addi %x, %b
//        return %c
TEST_F(InterferenceGraphTest, MultiBlock_BlockArgInterferesWithLiveInValue) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);

  mlir::Block *successor = addBlock(fn, {i32()});
  branch(successor, {a});

  setInsertionPointToEnd(successor);
  mlir::Value x = successor->getArgument(0);
  mlir::Value c = add(x, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph g(liveness, fn);

  EXPECT_TRUE(interferes(g, a, b));
  EXPECT_FALSE(interferes(g, a, x));
  EXPECT_FALSE(interferes(g, a, c));

  EXPECT_TRUE(interferes(g, b, a));
  EXPECT_TRUE(interferes(g, b, c));
  EXPECT_TRUE(interferes(g, b, x));

  EXPECT_TRUE(interferes(g, x, c));
  EXPECT_TRUE(interferes(g, x, b));
  EXPECT_FALSE(interferes(g, x, a));

  EXPECT_TRUE(interferes(g, c, b));
  EXPECT_TRUE(interferes(g, c, x));
  EXPECT_FALSE(interferes(g, c, a));
}
