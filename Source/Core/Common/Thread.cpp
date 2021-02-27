// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
size_t Thread::MainThreadId = 0;

bool Thread::IsMainThread()
{
  return GetCurrentThreadId() == MainThreadId;
}
} // namespace Plasma
