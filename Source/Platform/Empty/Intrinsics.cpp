// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

u32 CountTrailingPlasmas(u32 x)
{
  return CountTrailingPlasmasNonIntrinsic(x);
}

u32 CountLeadingPlasmas(u32 x)
{
  return CountLeadingPlasmasNonIntrinsic(x);
}

} // namespace Plasma
