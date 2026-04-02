#include "StackAllocation/StackAllocator.h"
#include "TestUtils.h"

using StackAllocatorTest = TranslationTest;

// Helper: run the full pipeline (liveness → interference → allocation) on fn.
static mlir::DenseMap<mlir::Value, unsigned>
runAllocation(mlir::func::FuncOp fn, LivenessInfo **livenessOut = nullptr,
              InterferenceGraph **graphOut = nullptr,
              StackAllocator **allocOut = nullptr) {
  // Objects are heap-allocated so callers can inspect them after the call.
  auto *liveness = new LivenessInfo(fn);
  auto *graph = new InterferenceGraph(*liveness, fn);
  auto *alloc = new StackAllocator(*graph);
  auto assignment = alloc->run();
  if (livenessOut)
    *livenessOut = liveness;
  if (graphOut)
    *graphOut = graph;
  if (allocOut)
    *allocOut = alloc;
  if (!livenessOut)
    delete liveness;
  if (!graphOut)
    delete graph;
  if (!allocOut)
    delete alloc;
  return assignment;
}

// Non-interfering sequential chain: values can share a color
TEST_F(StackAllocatorTest, NonInterfering_CanShareColor) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = add(a, a);
  mlir::Value c = add(b, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph graph(liveness, fn);
  StackAllocator alloc(graph);
  auto assignment = alloc.run();

  EXPECT_NE(assignment[a], assignment[b]);
  EXPECT_NE(assignment[b], assignment[c]);
  EXPECT_EQ(assignment[a], assignment[c]);
}

// Interfering pair: must get different colors
TEST_F(StackAllocatorTest, Interfering_DifferentColors) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = add(a, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph graph(liveness, fn);
  StackAllocator alloc(graph);
  auto assignment = alloc.run();

  ASSERT_TRUE(assignment.count(a));
  ASSERT_TRUE(assignment.count(b));
  EXPECT_NE(assignment[a], assignment[b]);
}

// Three-clique: all three values need distinct colors
TEST_F(StackAllocatorTest, Triangle_ThreeDistinctColors) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = constant(3);
  mlir::Value d = add(a, b);
  mlir::Value e = add(d, c);
  ret({e});

  LivenessInfo liveness(fn);
  InterferenceGraph graph(liveness, fn);
  StackAllocator alloc(graph);
  auto assignment = alloc.run();

  ASSERT_TRUE(assignment.count(a));
  ASSERT_TRUE(assignment.count(b));
  ASSERT_TRUE(assignment.count(c));
  EXPECT_NE(assignment[a], assignment[b]);
  EXPECT_NE(assignment[a], assignment[c]);
  EXPECT_NE(assignment[b], assignment[c]);
}

// All assigned colors must be in range [0, 15].
TEST_F(StackAllocatorTest, ColorRespectsBounds) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = constant(3);
  mlir::Value d = add(a, b);
  mlir::Value e = add(d, c);
  ret({e});

  auto assignment = runAllocation(fn);
  for (auto &[val, color] : assignment)
    EXPECT_LT(color, 16u) << "Color out of EVM stack bounds";
}

// Linear chain with shared colors
TEST_F(StackAllocatorTest, LinearChainWithShared_ThreeColors) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value t1 = add(a, b);
  mlir::Value c = constant(3);
  mlir::Value t2 = add(t1, c);
  ret({t2});

  auto assignment = runAllocation(fn);

  // Collect the set of distinct colors used.
  llvm::DenseSet<unsigned> colors;
  for (auto &[val, color] : assignment)
    colors.insert(color);

  EXPECT_LE(colors.size(), 3);

  EXPECT_NE(assignment[a], assignment[b]);
  EXPECT_NE(assignment[a], assignment[t1]);
  EXPECT_NE(assignment[b], assignment[t1]);
  EXPECT_NE(assignment[t1], assignment[c]);
  EXPECT_NE(assignment[t1], assignment[t2]);
  EXPECT_NE(assignment[c], assignment[t2]);
}

// Every value with an edge in the interference graph must appear in the
// assignment map after run().
TEST_F(StackAllocatorTest, AllInterferingValuesAssigned) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = constant(3);
  mlir::Value d = add(a, b);
  mlir::Value e = add(d, c);
  ret({e});

  LivenessInfo liveness(fn);
  InterferenceGraph graph(liveness, fn);

  llvm::SmallVector<mlir::Value> graphValues;
  for (auto v : graph.getValues())
    graphValues.push_back(v);

  StackAllocator alloc(graph);
  auto assignment = alloc.run();

  for (auto v : graphValues)
    EXPECT_TRUE(assignment.count(v)) << "Value has no color assignment";
}

// Spilled values must still receive a color assignment.
TEST_F(StackAllocatorTest, SpilledValues_StillGetColor) {
  // Build a function that forces at least one spill (17 simultaneously-live
  // constants so that one of them has 16 real neighbors).
  auto fn = makeFunc("test", {}, {i32()});
  const int N = 17;
  llvm::SmallVector<mlir::Value> consts;
  for (int i = 0; i < N; ++i)
    consts.push_back(constant(i));

  mlir::Value acc = add(consts[0], consts[1]);
  for (int i = 2; i < N; ++i)
    acc = add(acc, consts[i]);
  ret({acc});

  LivenessInfo liveness(fn);
  InterferenceGraph graph(liveness, fn);
  StackAllocator alloc(graph);
  auto assignment = alloc.run();

  EXPECT_FALSE(alloc.spilledValues.empty());

  for (auto v : alloc.spilledValues)
    EXPECT_TRUE(assignment.count(v)) << "Spilled value has no color";
}

// The simplification stack is populated during run() (then fully drained).
TEST_F(StackAllocatorTest, SimplificationStack_DrainedAfterRun) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = add(a, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph graph(liveness, fn);
  StackAllocator alloc(graph);
  alloc.run();

  // run() pops everything off the simplification stack during the coloring
  // phase; it should be empty when run() returns.
  EXPECT_TRUE(alloc.simplificationStack.empty());
}

// neighborSnapshot captures each value's neighbors at the moment of removal,
// before those edges are deleted. After run(), the snapshot for an interfering
// pair must contain the peer.
TEST_F(StackAllocatorTest, NeighborSnapshot_CapturedBeforeRemoval) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = add(a, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph graph(liveness, fn);
  StackAllocator alloc(graph);
  alloc.run();

  // %a and %b interfere. Whichever was simplified first, its snapshot must
  // contain the other (since at that moment they were still connected).
  bool snapshotAHasB =
      alloc.neighborSnapshot.count(a) && alloc.neighborSnapshot[a].count(b);
  bool snapshotBHasA =
      alloc.neighborSnapshot.count(b) && alloc.neighborSnapshot[b].count(a);
  // At least one of them was simplified while the other was still present.
  EXPECT_TRUE(snapshotAHasB || snapshotBHasA);
}

// Spill cost heuristic: lower useCount/neighbors ratio wins.
// We craft two candidate values where one has clearly lower cost.
// useCount=1, N neighbors → cost 0 (int division)
// useCount=N+1, N neighbors → cost 1 (int division for N ≥ 2)
// The low-cost value should be the one selected for spill.
//
// To trigger the spill path both values need ≥ 16 neighbors. We achieve this
// with 17 constants where one is used an extra time.
TEST_F(StackAllocatorTest, SpillCostHeuristic_LowUsageSpilledFirst) {
  auto fn = makeFunc("test", {}, {i32()});
  const int N = 17; // ensures ≥ 16 simultaneous live values

  llvm::SmallVector<mlir::Value> consts;
  for (int i = 0; i < N; ++i)
    consts.push_back(constant(i));

  // consts[0] is used once in the chain.
  // consts[1] will be reused at the end to get a higher use count.
  mlir::Value acc = add(consts[0], consts[1]);
  for (int i = 2; i < N; ++i)
    acc = add(acc, consts[i]);

  // Extra use of consts[1]: raises its useCount, increasing its spill cost.
  acc = add(acc, consts[1]);
  ret({acc});

  LivenessInfo liveness(fn);
  InterferenceGraph graph(liveness, fn);
  StackAllocator alloc(graph);
  alloc.run();

  // consts[0] has the lowest use count → lowest spill cost → should be
  // preferred for spill over consts[1] (which is used more times).
  if (!alloc.spilledValues.empty() && alloc.spilledValues.count(consts[0]) &&
      alloc.spilledValues.count(consts[1])) {
    // Both got spilled (acceptable if there are 2+ spillees); just verify
    // neither was the only one wrongly chosen.
    SUCCEED();
  } else if (alloc.spilledValues.count(consts[1]) &&
             !alloc.spilledValues.count(consts[0])) {
    FAIL() << "Higher-usage value was spilled instead of lower-usage one";
  }
}
