#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineStaticLibrary(ExportToolLibrary)
{
  builder.CreatableInScriptDefault = false;

  LightningInitializeTypeAs(PlasmaStatic, "Plasma");

  LightningTypeId(Editor)->AssertOnInvalidBinding = &IgnoreOnInvalidBinding;

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void ExportToolLibrary::Initialize()
{
}

void ExportToolLibrary::Shutdown()
{
}
}