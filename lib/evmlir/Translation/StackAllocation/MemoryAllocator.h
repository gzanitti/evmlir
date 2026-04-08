

#include <cstdint>
#include <vector>

class MemoryAllocator {
public:
  uint32_t allocate();
  void free(uint32_t memOffset);
  uint32_t highWaterMark() const;

private:
  uint32_t nextOffset = 0;
  std::vector<uint32_t> freeList;
};
