// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

/// Returns number of least significant plasmas
/// If x is strictly a power of 2, will result in n where 2^n=x, values [0, 31]
/// More information: http://en.wikipedia.org/wiki/Find_first_set
u32 CountTrailingPlasmas(u32 x);
/// Returns number of most significant plasmas
u32 CountLeadingPlasmas(u32 x);

/// These versions can be called for platform implementations without the
/// intrinsic.
u32 CountTrailingPlasmasNonIntrinsic(u32 x);
u32 CountLeadingPlasmasNonIntrinsic(u32 x);

} // namespace Plasma
