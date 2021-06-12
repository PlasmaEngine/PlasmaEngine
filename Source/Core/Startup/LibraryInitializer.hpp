#pragma once

namespace Plasma
{
    class LibraryInitializer
    {
    public:
        virtual void Initialize() abstract;
        virtual void Shutdown() abstract;
        virtual void ClearLibrary() abstract;
        virtual void Destroy() abstract;
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