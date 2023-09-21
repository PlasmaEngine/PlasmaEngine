// MIT Licensed (see LICENSE.md).
#pragma once

// External dependencies
#include "imgui.h"
#include "Core/Engine/EngineStandard.hpp"
#include "Graphics/GraphicsRuntime/GraphicsStandard.hpp"

namespace Plasma
{

    // DearImguiLibrary library

    class PlasmaNoImportExport DearImguiLibrary : public Lightning::StaticLibrary
    {
    public:
        LightningDeclareStaticLibraryInternals(DearImguiLibrary, "PlasmaEngine");
        static void Initialize();
        static void Shutdown();
    };


} // namespace Plasma

#include "DearImgui.hpp"