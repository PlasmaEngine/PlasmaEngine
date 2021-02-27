// MIT Licensed (see LICENSE.md).
#pragma once

#include "Core/Common/CommonStandard.hpp"
#include "Lightning/LightningCore/Precompiled.hpp"

#include "ForwardDeclarations.hpp"

#include "ShaderAttributes.hpp"
#include "ShaderErrors.hpp"
#include "ShaderCodeBuilder.hpp"

// Grab the latest unified spirv file. Update when switching spirv versions
#include "spirv/unified1/spirv.hpp"
#include "SpirVHelpers.hpp"
#include "LightningShaderIRMeta.hpp"
#include "LightningShaderIRReflection.hpp"
#include "LightningShaderIRShared.hpp"
#include "LightningShaderIRExtendedTypes.hpp"
#include "LightningSpirVSettings.hpp"
#include "ExtensionLibrary.hpp"
#include "OperatorKeys.hpp"
#include "ShaderIRLibraryTranslation.hpp"
#include "LightningShaderIRLibrary.hpp"
#include "LightningShaderIRProject.hpp"
#include "LightningShaderIRCore.hpp"
#include "LibraryTranslationHelpers.hpp"
#include "CommonInstructions.hpp"
#include "ShaderImageIntrinsics.hpp"
#include "ShaderIntrinsicTypes.hpp"

#include "BaseShaderIRTranslator.hpp"
#include "ShaderIntrinsicsStaticLightningLibrary.hpp"
#include "CycleDetection.hpp"
#include "StageRequirementsGatherer.hpp"
#include "SimpleLightningParser.hpp"
#include "EntryPointGeneration.hpp"
#include "LightningSpirVFrontEndValidation.hpp"
#include "LightningSpirVFrontEnd.hpp"

#include "TypeDependencyCollector.hpp"
#include "ShaderByteStream.hpp"
#include "LightningShaderIRTranslationPass.hpp"
#include "LightningShaderIRPasses.hpp"
#include "LightningShaderSpirVBinaryBackend.hpp"
#include "LightningSpirVDisassemblerBackend.hpp"
#include "LightningShaderGlslBackend.hpp"
#include "SpirVSpecializationConstantPass.hpp"
#include "LightningShaderIRCompositor.hpp"
#include "SimpleLightningShaderIRGenerator.hpp"

namespace Lightning
{

LightningDeclareStaticLibrary(ShaderIntrinsicsLibrary, LightningNoNamespace, PlasmaNoImportExport);

} // namespace Lightning

namespace Plasma
{

LightningDeclareStaticLibrary(ShaderSettingsLibrary, LightningNoNamespace, PlasmaNoImportExport);

} // namespace Plasma
