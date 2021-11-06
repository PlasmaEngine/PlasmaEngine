#include "Precompiled.hpp"

namespace Plasma
{
    size_t Cookie::GetCookie() const
    {
        return mCookie;
    }

    size_t Cookie::Hash() const
    {
        return HashPolicy<size_t>()(mCookie);
    }

    Cookie::Cookie()
    {

    }

    Cookie::Cookie(size_t cookie)
        : mCookie(cookie)
    {

    }

}