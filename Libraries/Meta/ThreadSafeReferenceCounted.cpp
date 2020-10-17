// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

DefineThreadSafeReferenceCountedHandle(ThreadSafeReferenceCounted);

LightningDefineType(ThreadSafeReferenceCounted, builder, type)
{
  PlasmaBindThreadSafeReferenceCountedHandle();
}

ThreadSafeReferenceCounted::ThreadSafeReferenceCounted()
{
  ConstructThreadSafeReferenceCountedHandle();
}

ThreadSafeReferenceCounted::ThreadSafeReferenceCounted(const ThreadSafeReferenceCounted&)
{
  ConstructThreadSafeReferenceCountedHandle();
}

ThreadSafeReferenceCounted::~ThreadSafeReferenceCounted()
{
  DestructThreadSafeReferenceCountedHandle();
}

} // namespace Plasma
