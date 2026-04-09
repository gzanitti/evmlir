
#include "MemoryAllocator.h"

uint32_t MemoryAllocator::allocate() {
  if (!freeList.empty()) {
    uint32_t offset = freeList.back();
    freeList.pop_back();
    return offset;
  }
  uint32_t offset = nextOffset;
  nextOffset += 32; // EVM word size
  return offset;
}

void MemoryAllocator::free(uint32_t memOffset) {
  freeList.push_back(memOffset);
}

uint32_t MemoryAllocator::highWaterMark() const { return nextOffset; }

bool MemoryAllocator::hasFreeSlots() const { return !freeList.empty(); }

uint32_t MemoryAllocator::expansionCost() const {
  uint32_t words = nextOffset / 32;
  return 3 + (2 * words + 1) / 512;
}