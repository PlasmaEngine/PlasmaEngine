#pragma once
// We use precompiled headers to support the fastest possible compilation
#define PlasmaImportDll
#include "Lightning.hpp"
#include "Core.hpp"
#include "PlasmaEngine.hpp"

// Declares our Lightning Library (where we get LibraryBuilder from)
// This also handles the plugin initialization
LightningDeclareStaticLibraryAndPlugin(RESOURCE_NAME_Library, RESOURCE_NAME_Plugin);

// We also encourage including all files within here, rather than within headers
// If another project needed to include any headers from our project, then they would simply
// include our precompiled header instead (ideally within their own precompiled header)
// This also must means you must order headers in dependency order (who depends on who)

#include "RESOURCE_NAME_.hpp"
// Auto Includes (used by Visual Studio plugins, do not remove this line)
