// MIT Licensed (see LICENSE.md).
#pragma once
#include "Standard.hpp"

namespace Plasma
{

typedef size_t MemCounterType;
typedef void* MemPtr;

MemPtr plAllocate(size_t numberOfBytes);
void plDeallocate(MemPtr);
MemPtr plStaticAllocate(size_t size);

#define UseStaticMemory()                                                                                              \
  static void* operator new(size_t size)                                                                               \
  {                                                                                                                    \
    return plStaticAllocate(size);                                                                                      \
  }                                                                                                                    \
  static void operator delete(void* /*pMem*/, size_t /*size*/)                                                         \
  {                                                                                                                    \
  }

#define OverloadedNew()                                                                                                \
  static void* operator new(size_t size);                                                                              \
  static void operator delete(void* pMem, size_t size);                                                                \
  static void* operator new(size_t size, void* ptr)                                                                    \
  {                                                                                                                    \
    return ptr;                                                                                                        \
  };                                                                                                                   \
  static void operator delete(void* mem, void* ptr){};

#define ImplementOverloadedNewWithAllocator(ClassName, AllocatorObj)                                                   \
  void* ClassName::operator new(size_t size)                                                                           \
  {                                                                                                                    \
    return AllocatorObj->Allocate(size);                                                                               \
  };                                                                                                                   \
  void ClassName::operator delete(void* pMem, size_t size)                                                             \
  {                                                                                                                    \
    AllocatorObj->Deallocate(pMem, size);                                                                              \
  }

} // namespace Plasma
