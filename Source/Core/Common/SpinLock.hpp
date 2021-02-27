// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{
// The spin lock loops without a wait or sleep until the lock becomes available
// SpinLock does NOT support locking twice on the same thread (deadlock will
// occur)
class PlasmaShared SpinLock
{
public:
  void Lock();
  void Unlock();

private:
  Atomic<bool> mLocked;
};
} // namespace Plasma
