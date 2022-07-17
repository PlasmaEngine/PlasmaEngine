#pragma once

namespace Plasma
{
    class LibraryInitializer
    {
    public:
        virtual void Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual void ClearLibrary() = 0;
        virtual void Destroy() = 0;
    };

    template<typename T>
    class TypedLibraryInitializer : public LibraryInitializer
    {
    public: 
        void Initialize() override
        {
            T::Initialize();
        }
        
        void Shutdown() override
        {
            T::Shutdown();
        }

        void ClearLibrary() override
        {
            T::GetInstance().ClearLibrary();
        }

        void Destroy() override
        {
            T::Destroy();
        }
    };
}