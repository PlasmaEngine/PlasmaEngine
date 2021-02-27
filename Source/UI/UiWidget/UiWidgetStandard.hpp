// MIT Licensed (see LICENSE.md).
#pragma once

// External dependencies
#include "Core/Engine/EngineStandard.hpp"
#include "Graphics/GraphicsRuntime/GraphicsStandard.hpp"
#include "UI/Widget/WidgetStandard.hpp"
#include "Core/Gameplay/GameplayStandard.hpp"

namespace Plasma
{

// UiWidget library
class PlasmaNoImportExport UiWidgetLibrary : public Lightning::StaticLibrary
{
public:
  LightningDeclareStaticLibraryInternals(UiWidgetLibrary, "PlasmaEngine");

  static void Initialize();
  static void Shutdown();
};

} // namespace Plasma

// Widget Core
#include "UiWidget.hpp"
#include "UiRootWidget.hpp"
#include "UiWidgetEvents.hpp"

// Layouts
#include "UiLayout.hpp"
#include "UiStackLayout.hpp"
#include "UiFillLayout.hpp"
#include "UiDockLayout.hpp"
