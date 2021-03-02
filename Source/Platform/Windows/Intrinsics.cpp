// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)

u32 CountTrailingPlasmas(u32 x)
{
  unsigned long ret = 32;
  _BitScanForward(&ret, x);
  return ret;
}

u32 CountLeadingPlasmas(u32 x)
{
  unsigned long ret = 32;
  _BitScanReverse(&ret, x);
  return 31 - ret;
}

} // namespace Plasma
