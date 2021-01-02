#include "RESOURCE_NAME_Precompiled.hpp"

//***************************************************************************
LightningDefineStaticLibraryAndPlugin(RESOURCE_NAME_Library, RESOURCE_NAME_Plugin, LightningDependencyStub(Core) LightninghDependencyStub(PlasmaEngine))
{
  LightningInitializeType(RESOURCE_NAME_);
  LightningInitializeType(RESOURCE_NAME_Event);
  // Auto Initialize (used by Visual Studio plugins, do not remove this line)
}

//***************************************************************************
void RESOURCE_NAME_Plugin::Initialize()
{
  // One time startup logic goes here
  // This runs after our plugin library/reflection is built
  Lightning::Console::WriteLine("RESOURCE_NAME_Plugin::Initialize");
}

//***************************************************************************
void RESOURCE_NAME_Plugin::Uninitialize()
{
  // One time shutdown logic goes here
  Lightning::Console::WriteLine("RESOURCE_NAME_Plugin::Uninitialize");
}
