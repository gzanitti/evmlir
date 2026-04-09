#include "StackAllocation/StackAllocator.h"
#include "CodeGen/ForkSpec.h"
#include "TestUtils.h"

using StackAllocatorTest = TranslationTest;

// Helper: run the full pipeline (liveness → interference → allocation) on fn.
static mlir::DenseMap<mlir::Value, ValueLocation>
runAllocation(mlir::func::FuncOp fn, StackAllocator **allocOut = nullptr) {
  auto *liveness = new LivenessInfo(fn);
  auto *graph = new InterferenceGraph(*liveness, fn);
  auto *alloc = new StackAllocator(*graph, MemoryAllocator{}, OsakaSpec);
  auto assignment = alloc->run();
  if (allocOut)
    *allocOut = alloc;
  else
    delete alloc;
  delete graph;
  delete liveness;
  return assignment;
}

static bool isStackLoc(const ValueLocation &loc) {
  return std::holds_alternative<StackLoc>(loc);
}
static uint8_t stackPos(const ValueLocation &loc) {
  return std::get<StackLoc>(loc).position;
}

// Non-interfering sequential chain: values can share a color.
// a is dead before c is computed, so a and c can share a stack slot.
TEST_F(StackAllocatorTest, NonInterfering_CanShareColor) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = add(a, a);
  mlir::Value c = add(b, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph graph(liveness, fn);
  StackAllocator alloc(graph, MemoryAllocator{}, OsakaSpec);
  auto assignment = alloc.run();

  ASSERT_TRUE(isStackLoc(assignment[a]));
  ASSERT_TRUE(isStackLoc(assignment[b]));
  ASSERT_TRUE(isStackLoc(assignment[c]));
  EXPECT_NE(stackPos(assignment[a]), stackPos(assignment[b]));
  EXPECT_NE(stackPos(assignment[b]), stackPos(assignment[c]));
  EXPECT_EQ(stackPos(assignment[a]), stackPos(assignment[c]));
}

// Interfering pair: must get different colors.
TEST_F(StackAllocatorTest, Interfering_DifferentColors) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = add(a, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph graph(liveness, fn);
  StackAllocator alloc(graph, MemoryAllocator{}, OsakaSpec);
  auto assignment = alloc.run();

  ASSERT_TRUE(assignment.count(a));
  ASSERT_TRUE(assignment.count(b));
  ASSERT_TRUE(isStackLoc(assignment[a]));
  ASSERT_TRUE(isStackLoc(assignment[b]));
  EXPECT_NE(stackPos(assignment[a]), stackPos(assignment[b]));
}

// Three-clique: all three values need distinct colors.
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
  StackAllocator alloc(graph, MemoryAllocator{}, OsakaSpec);
  auto assignment = alloc.run();

  ASSERT_TRUE(assignment.count(a));
  ASSERT_TRUE(assignment.count(b));
  ASSERT_TRUE(assignment.count(c));
  ASSERT_TRUE(isStackLoc(assignment[a]));
  ASSERT_TRUE(isStackLoc(assignment[b]));
  ASSERT_TRUE(isStackLoc(assignment[c]));
  EXPECT_NE(stackPos(assignment[a]), stackPos(assignment[b]));
  EXPECT_NE(stackPos(assignment[a]), stackPos(assignment[c]));
  EXPECT_NE(stackPos(assignment[b]), stackPos(assignment[c]));
}

// All assigned stack colors must be in range [0, 15].
TEST_F(StackAllocatorTest, ColorRespectsBounds) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = constant(3);
  mlir::Value d = add(a, b);
  mlir::Value e = add(d, c);
  ret({e});

  auto assignment = runAllocation(fn);
  for (auto &[val, loc] : assignment) {
    if (isStackLoc(loc))
      EXPECT_LT(stackPos(loc), 16u) << "Color out of EVM stack bounds";
  }
}

// Linear chain with shared colors.
TEST_F(StackAllocatorTest, LinearChainWithShared_ThreeColors) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value t1 = add(a, b);
  mlir::Value c = constant(3);
  mlir::Value t2 = add(t1, c);
  ret({t2});

  auto assignment = runAllocation(fn);

  llvm::DenseSet<uint8_t> colors;
  for (auto &[val, loc] : assignment)
    if (isStackLoc(loc))
      colors.insert(stackPos(loc));

  EXPECT_LE(colors.size(), 3u);

  ASSERT_TRUE(isStackLoc(assignment[a]));
  ASSERT_TRUE(isStackLoc(assignment[b]));
  ASSERT_TRUE(isStackLoc(assignment[t1]));
  ASSERT_TRUE(isStackLoc(assignment[c]));
  ASSERT_TRUE(isStackLoc(assignment[t2]));
  EXPECT_NE(stackPos(assignment[a]), stackPos(assignment[b]));
  EXPECT_NE(stackPos(assignment[a]), stackPos(assignment[t1]));
  EXPECT_NE(stackPos(assignment[b]), stackPos(assignment[t1]));
  EXPECT_NE(stackPos(assignment[t1]), stackPos(assignment[c]));
  EXPECT_NE(stackPos(assignment[t1]), stackPos(assignment[t2]));
  EXPECT_NE(stackPos(assignment[c]), stackPos(assignment[t2]));
}

// Every value in the interference graph must appear in the assignment map.
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

  StackAllocator alloc(graph, MemoryAllocator{}, OsakaSpec);
  auto assignment = alloc.run();

  for (auto v : graphValues)
    EXPECT_TRUE(assignment.count(v)) << "Value has no location assignment";
}

// When 17 values are simultaneously live, the forced-spill path is triggered.
// All values must still appear in the assignment map (SpilledLoc, RecomputeLoc,
// or StackLoc).
TEST_F(StackAllocatorTest, ForcedSpill_AllValuesAssigned) {
  const int N = 17;
  llvm::SmallVector<mlir::Type> argTypes(N, i32());
  auto fn = makeFunc("test", argTypes, {i32()});
  auto &entry = fn.getBody().front();
  llvm::SmallVector<mlir::Value> args;
  for (int i = 0; i < N; ++i)
    args.push_back(entry.getArgument(i));

  mlir::Value acc = add(args[0], args[1]);
  for (int i = 2; i < N; ++i)
    acc = add(acc, args[i]);
  ret({acc});

  LivenessInfo liveness(fn);
  InterferenceGraph graph(liveness, fn);

  llvm::SmallVector<mlir::Value> graphValues;
  for (auto v : graph.getValues())
    graphValues.push_back(v);

  StackAllocator alloc(graph, MemoryAllocator{}, OsakaSpec);
  auto assignment = alloc.run();

  for (auto v : graphValues)
    EXPECT_TRUE(assignment.count(v)) << "Value missing from assignment";
}

// The simplification stack is fully drained after run().
TEST_F(StackAllocatorTest, SimplificationStack_DrainedAfterRun) {
  auto fn = makeFunc("test", {}, {i32()});
  mlir::Value a = constant(1);
  mlir::Value b = constant(2);
  mlir::Value c = add(a, b);
  ret({c});

  LivenessInfo liveness(fn);
  InterferenceGraph graph(liveness, fn);
  StackAllocator alloc(graph, MemoryAllocator{}, OsakaSpec);
  alloc.run();

  EXPECT_TRUE(alloc.simplificationStack.empty());
}

// Spill cost heuristic: lower useCount/neighbors ratio is selected first.
// consts[0] has useCount=1; consts[1] has useCount=2 (used an extra time).
// With N=17, all interfere → 16 neighbors each → spill_cost(consts[0])=0,
// spill_cost(consts[1])=0 too (1/16 and 2/16 both truncate to 0 in integer
// division with 16 neighbors). However, as N grows, consts[1] eventually gets
// cost=1 while consts[0] stays at 0. We use N=17 where the ratio difference
// is detectable: consts[1] should NOT be the sole non-stack value while
// consts[0] gets a stack location.
TEST_F(StackAllocatorTest, SpillCostHeuristic_LowUsageSpilledFirst) {
  auto fn = makeFunc("test", {}, {i32()});
  const int N = 17;

  llvm::SmallVector<mlir::Value> consts;
  for (int i = 0; i < N; ++i)
    consts.push_back(constant(i + 1));

  mlir::Value acc = add(consts[0], consts[1]);
  for (int i = 2; i < N; ++i)
    acc = add(acc, consts[i]);

  acc = add(acc, consts[1]);
  ret({acc});

  LivenessInfo liveness(fn);
  InterferenceGraph graph(liveness, fn);
  StackAllocator alloc(graph, MemoryAllocator{}, OsakaSpec);
  alloc.run();

  bool consts0NonStack = !isStackLoc(alloc.assignment[consts[0]]);
  bool consts1NonStack = !isStackLoc(alloc.assignment[consts[1]]);
  if (consts1NonStack && !consts0NonStack)
    FAIL() << "Higher-usage value got non-stack location instead of "
              "lower-usage one";
}

// Builds a function with a 17-clique: 17 add results all live simultaneously.
// %a and %b are function args; 17 adds of (%a,%b) are all alive at once when
// they're all consumed together in the reduction tree.
static mlir::func::FuncOp buildCliqueFunc(TranslationTest &t) {
  auto fn = t.makeFunc("with_clique", {t.i32(), t.i32()}, {t.i32()});
  mlir::Value a = fn.getBody().front().getArgument(0);
  mlir::Value b = fn.getBody().front().getArgument(1);

  llvm::SmallVector<mlir::Value> c;
  for (int i = 0; i < 17; ++i)
    c.push_back(t.add(a, b));

  mlir::Value t0 = t.add(c[0], c[1]);
  mlir::Value t1 = t.add(c[2], c[3]);
  mlir::Value t2 = t.add(c[4], c[5]);
  mlir::Value t3 = t.add(c[6], c[7]);
  mlir::Value t4 = t.add(c[8], c[9]);
  mlir::Value t5 = t.add(c[10], c[11]);
  mlir::Value t6 = t.add(c[12], c[13]);
  mlir::Value t7 = t.add(c[14], c[15]);
  mlir::Value t8 = t.add(t0, c[16]);

  mlir::Value r0 = t.add(t0, t1);
  mlir::Value r1 = t.add(t2, t3);
  mlir::Value r2 = t.add(t4, t5);
  mlir::Value r3 = t.add(t6, t7);
  mlir::Value r4 = t.add(r0, r1);
  mlir::Value r5 = t.add(r2, r3);
  mlir::Value res = t.add(r4, r5);
  (void)t8;
  t.ret({res});
  return fn;
}

// With 17 simultaneously-live function arguments (all interfering pairwise),
// at least one must receive a non-stack location since EVM stack depth is 16.
TEST_F(StackAllocatorTest, ForcedSpill_HasNonStackLocation) {
  auto fn = buildCliqueFunc(*this);
  auto assignment = runAllocation(fn);

  bool anyNonStack = false;
  for (auto &[val, loc] : assignment)
    if (!isStackLoc(loc))
      anyNonStack = true;
  EXPECT_TRUE(anyNonStack)
      << "Expected at least one non-stack location with 17 live args";
}

// When recompute cost is cheaper than spill, the allocator must choose
// RecomputeLoc. 17 constants returned directly form a pure 17-clique.
TEST_F(StackAllocatorTest, RecomputeLoc_WhenCheaper) {
  const int N = 17;
  llvm::SmallVector<mlir::Type> retTypes(N, i32());
  auto fn = makeFunc("test", {}, retTypes);
  llvm::SmallVector<mlir::Value> consts;
  for (int i = 0; i < N; ++i)
    consts.push_back(constant(i + 1));
  ret(consts);

  auto assignment = runAllocation(fn);

  bool anyRecompute = false;
  for (auto &[val, loc] : assignment)
    if (std::holds_alternative<RecomputeLoc>(loc))
      anyRecompute = true;
  EXPECT_TRUE(anyRecompute) << "Expected RecomputeLoc: constants cost 3 gas to "
                               "recompute vs 12 gas to spill";
}

// SpilledLoc is used when a value cannot be recomputed. Function arguments
// have no defining op so recompute returns nullopt → forced to SpilledLoc.
// Build a clique with function args to guarantee the spill path is hit.
TEST_F(StackAllocatorTest, SpilledValue_GetsSpilledLoc) {
  const int N = 17;
  llvm::SmallVector<mlir::Type> argTypes(N, i32());
  llvm::SmallVector<mlir::Type> retTypes(N, i32());
  auto fn = makeFunc("test", argTypes, retTypes);
  auto &entry = fn.getBody().front();
  llvm::SmallVector<mlir::Value> args;
  for (int i = 0; i < N; ++i)
    args.push_back(entry.getArgument(i));
  ret(args);

  auto assignment = runAllocation(fn);

  bool anySpilled = false;
  for (auto arg : args)
    if (assignment.count(arg) &&
        std::holds_alternative<SpilledLoc>(assignment[arg]))
      anySpilled = true;
  EXPECT_TRUE(anySpilled)
      << "At least one arg should be SpilledLoc (no defining op)";
}
