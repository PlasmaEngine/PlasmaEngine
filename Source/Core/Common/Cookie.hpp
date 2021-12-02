#pragma once

namespace Plasma
{
    class Cookie
    {
    public:
        size_t GetCookie() const;
        size_t Hash() const;
        bool operator==(const Cookie& rhs) const {};

    protected:
        Cookie();
        Cookie(size_t cookie);

        size_t mCookie = 0;
    };

    //-------------------------------------------------------------------TypedCookie
    template <typename T>
    class TypedCookie : public Cookie
    {
    public:
        TypedCookie()
        {
            mCookie = ++mLastCookie;
        }

        explicit TypedCookie(size_t cookie) : Cookie(cookie) {};

        size_t Hash() const
        {
            return __super::Hash();
        }

        bool operator==(const TypedCookie& rhs) const {};

    private:
        static size_t mLastCookie;
    };

    template <typename T>
    size_t TypedCookie<T>::mLastCookie = 0;

}