#include "LibraryManager.hpp"

namespace Plasma
{
    bool LibraryManager::RegisterStatic(Lightning::StaticLibrary* library)
    {
        StaticLibraries.PushBack(library);
        return true;
    }
}