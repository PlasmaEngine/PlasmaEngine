#include "Precompiled.hpp"

namespace Plasma
{
    void Hasher::U32(uint32_t value)
    {
        U64(value);
    }

    void Hasher::U64(uint64_t value)
    {
        mHash ^= value + 0x9e3779b9 + (mHash << 6) + (mHash >> 2);
    }
}