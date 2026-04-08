#include "ForkSpec.h"
#include "OpcodeMapper.h"
#include "mlir/IR/Value.h"
#include <cstdint>

class EVMCostModel {
public:
  explicit EVMCostModel(const ForkSpec &spec) : spec(spec){};

  std::optional<uint32_t>
  recomputeCost(mlir::Value v, const mlir::DenseSet<mlir::Value> &currentStack,
                uint32_t depth = 0, uint32_t accumulatedCost = 0) const;
  uint32_t spillMemoryCost() const;

private:
  const ForkSpec &spec;
  static constexpr uint32_t MAX_DEPTH = 5;

  uint32_t opcodeCost(mlir::Operation *op) const;
  bool canRecompute(mlir::Operation *op) const;
};