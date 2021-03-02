// MIT Licensed (see LICENSE.md).
#pragma once

// Standard Library Dependencies
#include "Core/Common/CommonStandard.hpp"
#include "PlatformStandard.hpp"
#include "Core/Geometry/GeometryStandard.hpp"
#include "Core/Meta/MetaStandard.hpp"
#include "Core/Support/SupportStandard.hpp"

// Lightning Library Dependencies
#include "Lightning/LightningCore/Precompiled.hpp"
using namespace Lightning;

namespace Plasma
{
class PlasmaNoImportExport ExportToolLibrary : public Lightning::StaticLibrary
{
public:
  LightningDeclareStaticLibraryInternals(ExportToolLibrary, "PlasmaEngine");

  static void Initialize();
  static void Shutdown();
};
}

// Core Library Dependencies
#include "Core/Engine/EngineStandard.hpp"
#include "UI/Widget/WidgetStandard.hpp"


#include "Exporter.hpp"