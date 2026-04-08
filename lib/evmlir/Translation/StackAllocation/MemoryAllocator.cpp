
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