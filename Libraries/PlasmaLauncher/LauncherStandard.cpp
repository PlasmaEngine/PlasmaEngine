// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
const String mLauncherRegularFont = "NotoSans-Regular";
const String mLauncherBoldFont = "NotoSans-Bold";

LightningDefineEnum(LauncherStartupArguments);

LightningDefineStaticLibrary(LauncherLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Enums
  LightningInitializeEnum(LauncherStartupArguments);

  // Events
  LightningInitializeType(LauncherBuildEvent);

  // Components
  LightningInitializeType(PlasmaBuildContent);
  LightningInitializeType(PlasmaBuildReleaseNotes);
  LightningInitializeType(PlasmaBuildDeprecated);
  LightningInitializeType(PlasmaTemplate);
  LightningInitializeType(LauncherProjectInfo);
}

void LauncherLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

void LauncherLibrary::Shutdown()
{
}

} // namespace Plasma
