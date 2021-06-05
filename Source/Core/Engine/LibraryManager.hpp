#pragma once

#include <functional>

namespace Plasma
{
    class LibraryManager : public ExplicitSingleton<LibraryManager>
    {
    public:
        LibraryManager(){}
        ~LibraryManager(){}

        static bool RegisterStatic(Lightning::StaticLibrary* library);
        static Array<Lightning::StaticLibrary*> StaticLibraries;
    };
}