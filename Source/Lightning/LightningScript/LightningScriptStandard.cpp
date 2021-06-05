// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void LightningConsolePrint(ConsoleEvent* e);

LightningDefineStaticLibrary(LightningScriptLibrary)
{
  builder.CreatableInScriptDefault = false;

  LightningInitializeType(LightningComponent);
  LightningInitializeType(LightningEvent);
  LightningInitializeType(LightningObject);
  LightningInitializeType(LightningScript);

  LightningInitializeType(LightningPluginSource);
  LightningInitializeType(LightningPluginLibrary);

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

void LightningScriptLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  InitializeResourceManager(LightningScriptManager);
  InitializeResourceManager(LightningPluginLibraryManager);
  InitializeResourceManager(LightningPluginSourceManager);

  ResourceLibrary::sScriptType = LightningTypeId(LightningScript);

  EventConnect(&Lightning::Console::Events, Lightning::Events::ConsoleWrite, LightningConsolePrint);
}

void LightningScriptLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

void LightningConsolePrint(ConsoleEvent* e)
{
  // Print out the standard formatted error message to the console
  Console::Print(Filter::DefaultFilter, "%s", e->Text.c_str());
}

} // namespace Plasma
