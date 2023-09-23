// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace PL
{
static u32 gLexicographicMask = 0xffffffff;
static u64 gLexicographicUpperMask = static_cast<u64>(gLexicographicMask) << 32;
static u64 gLexicographicLowerMask = static_cast<u64>(gLexicographicMask);
} // namespace PL

const TimeType cTimeMax = LONG_MAX;

u64 GetLexicographicId(u32 id1, u32 id2)
{
  u64 id = 0;

  // put the smaller number in the top 32 bits and the larger in the bottom 16
  if (id1 < id2)
  {
    id |= (static_cast<u64>(id1) & PL::gLexicographicMask) << 32;
    id |= (static_cast<u64>(id2) & PL::gLexicographicMask);
  }
  else
  {
    id |= (static_cast<u64>(id2) & PL::gLexicographicMask) << 32;
    id |= (static_cast<u64>(id1) & PL::gLexicographicMask);
  }

  /*could also do
  u32* start = reinterpret_cast<u32*>(&id);
  if(id1 < id2)
  {
    start[0] = id2;
    start[1] = id1;
  }
  else
  {
    start[0] = id1;
    start[1] = id2;
  }
  although endianness matters, which would only screw up if
  sending a pair id from one machine to another
  */

  return id;
}

void UnPackLexicographicId(u32& id1, u32& id2, u64 pairId)
{
  id1 = static_cast<u32>(pairId & PL::gLexicographicLowerMask);
  id2 = static_cast<u32>((pairId & PL::gLexicographicUpperMask) >> 32);

  /*could also do
  u32* start = reinterpret_cast<u32*>(&pairId);
  id1 = *start;
  id2 = *(start + 1);*/
}

bool IsBigEndian()
{
  int i = 1;
  ::byte* lowByte = (::byte*)&i;

  return (*lowByte == 0);
}

u32 NextPowerOfTwo(u32 x)
{
  u32 leadingPlasmas = CountLeadingPlasmas(x);
  return 1 << (32 - leadingPlasmas);
}

} // namespace Plasma

// Used for counting printf statement lengths
char gDiscardBuffer[2] = {0};
