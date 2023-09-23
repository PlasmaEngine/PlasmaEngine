// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
namespace Memory
{

Pool::Pool(cstr name, Graph* parent, size_t blockSize, size_t blocksPerPage, bool podStackPool) : Graph(name, parent)
{
  mPodStackPool = podStackPool;

  ErrorIf(mPodStackPool == false && parent == nullptr,
          "Memory pool needs a parent node "
          "otherwise it will not be deallocated.");
  mNextFreeBlock = nullptr;
  mBlockSize = blockSize;
  mBlocksPerPage = blocksPerPage;
  mPageSize = blockSize * blocksPerPage;
}

void Pool::AllocatePage()
{
  // Allocate a new page of memory and
  // divide it into blocks that are
  // each placed on the free list.
  DeltaDedicated(mPageSize);
  ::byte* memoryPage = (::byte*)plAllocate(mPageSize);
  mPages.PushBack(memoryPage);
  for (unsigned block = 0; block < mBlocksPerPage; ++block)
    PushOnFreeList(memoryPage + mBlockSize * block);
}

void Pool::CleanUp()
{
  ErrorIf(
      mPodStackPool == false && mData.BytesAllocated != 0, "Failed to release all memory from pool %s", Name.c_str());
  // Deallocate each page
  for (unsigned i = 0; i < mPages.Size(); ++i)
    plDeallocate(mPages[i]); // mPageSize
  mPages.Deallocate();
}

Pool::~Pool()
{
  CleanUp();
}

MemPtr Pool::Allocate(size_t numberOfBytes)
{
  // Allocate memory by pop a block off the free list.
  ErrorIf(numberOfBytes > mBlockSize, "Allocation is large than block size.");
  AddAllocation(mBlockSize);
  auto ptr = PopOnFreeList();
  //TracyAlloc(ptr, numberOfBytes);
  return ptr;
}

void Pool::Deallocate(MemPtr ptr, size_t /*numberOfBytes*/)
{
  // It should be safe to delete null pointers
  if (ptr == nullptr)
    return;

#ifdef PlasmaDebug
  // 0xFAFAFAFA is our own byte pattern used to show that we deallocated the
  // memory, but have not yet released it to the os
  memset(ptr, 0xFA, mBlockSize);
#endif

  // Deallocate memory by push a block on the free list.
  RemoveAllocation(mBlockSize);
  PushOnFreeList(ptr);

  //TracyFree(ptr);
}

MemPtr Pool::PopOnFreeList()
{
  // No blocks left allocate a new page.
  if (mNextFreeBlock == nullptr)
    AllocatePage();

  // Pop the block
  FreeBlock* block = (FreeBlock*)mNextFreeBlock;
  mNextFreeBlock = block->NextBlock;
  return block;
}

void Pool::PushOnFreeList(MemPtr ptr)
{
  // Push the block
  FreeBlock* block = (FreeBlock*)ptr;
  block->NextBlock = mNextFreeBlock;
  mNextFreeBlock = block;
}

void Pool::Print(size_t tabs, size_t flags)
{
  PrintHelper(tabs, flags, "Pool");
}

} // namespace Memory
} // namespace Plasma
