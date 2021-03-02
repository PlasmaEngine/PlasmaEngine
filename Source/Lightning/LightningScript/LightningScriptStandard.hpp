// MIT Licensed (see LICENSE.md).
#pragma once

#include "Core/Engine/EngineStandard.hpp"

#include "LightningScript.hpp"
#include "LightningPlasma.hpp"
#include "LightningPlugin.hpp"

#include "Editor/EditorCore/EditorStandard.hpp"

namespace Plasma
{
// Forward declarations
class LightningPluginLibrary;

// Lightning Script library
class PlasmaNoImportExport LightningScriptLibrary : public Lightning::StaticLibrary
{
public:
  LightningDeclareStaticLibraryInternals(LightningScriptLibrary, "PlasmaEngine");

  static void Initialize();
  static void Shutdown();
};

} // namespace Plasma
