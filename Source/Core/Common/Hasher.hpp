#pragma once

namespace Plasma
{
    class Hasher
    {
    public:
        void U32(uint32_t value);
        void U64(uint64_t value);
        template <typename T>
        void operator()(const T& data)
        {
            size_t hash = Zero::HashPolicy<T>()(data);
            U64(hash);
        }

        size_t mHash = 0;
    };

}