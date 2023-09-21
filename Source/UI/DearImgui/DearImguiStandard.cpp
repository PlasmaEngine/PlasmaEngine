// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

    
    LightningDefineStaticLibrary(DearImguiLibrary)
    {
      builder.CreatableInScriptDefault = false;
      EngineLibraryExtensions::AddNativeExtensions(builder);
    }
    void DearImguiLibrary::Initialize()
    {
      BuildStaticLibrary();
      MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
    }
    void DearImguiLibrary::Shutdown()
    {
      GetLibrary()->ClearComponents();
    }

} // namespace Plasma