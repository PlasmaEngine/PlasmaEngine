// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
namespace Memory
{

Stack::Stack(cstr name, Graph* parent, size_t stackSize, size_t maxEntries) : Graph(name, parent)
{
  mStackSize = stackSize;
  mMaxEntries = maxEntries;

  if (StackDebug)
  {
    mEntries.Reserve(maxEntries);
  }

  mStackHeader = (::byte*)plAllocate(mStackSize);
  mStackIndex = 0;
  mMaxSizeReached = 0;
}

Stack::~Stack()
{
  CleanUp();
}

void Stack::Print(size_t tabs, size_t flags)
{
  PrintHelper(tabs, flags, "Stack");
}

MemPtr Stack::Allocate(size_t numberOfBytes)
{
  AddAllocation(numberOfBytes);

  if (StackDebug)
  {
    ErrorIf(mEntries.Size() == mMaxEntries,
            "Maximum number of stack entries reached."
            "Expand the max entries.");

    ErrorIf(mStackIndex + numberOfBytes > mStackSize,
            "All memory used in stack."
            "Expand the starting size.");
  }

  ::byte* curHead = mStackHeader + mStackIndex;
  mStackIndex += numberOfBytes;

  if (StackDebug)
  {
    if (mMaxSizeReached < mStackIndex)
      mMaxSizeReached = mStackIndex;

    mEntries.PushBack(Entry(curHead, numberOfBytes));
  }

  //TracyAlloc(curHead, numberOfBytes);
	
  return curHead;
}

void Stack::Deallocate(MemPtr ptr, size_t numberOfBytes)
{
  if (StackDebug)
  {
    Entry& entry = mEntries.Back();
    ErrorIf(entry.Ptr != ptr,
            "Stack deallocation out of order. Stack items"
            " must be deleted in proper stack order, first in last out.");
    ErrorIf(entry.Size != numberOfBytes, "Bad sized passed to deallocate.");
    mEntries.PopBack();
  }

  //TracyFree(ptr);
  mStackIndex -= numberOfBytes;
}

void Stack::CleanUp()
{
  plDeallocate(mStackHeader);
}

} // namespace Memory
} // namespace Plasma
