// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

/// The lightning shader wrapper around Lightning's core library. This needs to be
/// built and have the Parse function called once before all shader translation.
/// Contains the primitive types for translation.
class LightningShaderIRCore
{
public:
  static void InitializeInstance();
  static void Destroy();
  static LightningShaderIRCore& GetInstance();

  LightningShaderIRCore();
  /// Parse the core library and make all backing shader types.
  void Parse(LightningSpirVFrontEnd* translator);
  LightningShaderIRLibraryRef GetLibrary();

  LightningTypeGroups mLightningTypes;
  ShaderTypeGroups mShaderTypes;
  SpirVExtensionLibrary* mGlsl450ExtensionsLibrary;

private:
  void MakeMathTypes(LightningSpirVFrontEnd* translator,
                     LightningShaderIRLibrary* shaderLibrary,
                     ShaderTypeGroups& types);
  void RegisterPrimitiveFunctions(LightningSpirVFrontEnd* translator,
                                  LightningShaderIRLibrary* shaderLibrary,
                                  ShaderTypeGroups& types,
                                  LightningShaderIRType* shaderType);
  void RegisterVectorFunctions(LightningSpirVFrontEnd* translator,
                               LightningShaderIRLibrary* shaderLibrary,
                               ShaderTypeGroups& types,
                               Array<LightningShaderIRType*>& vectorTypes);
  void RegisterMatrixFunctions(LightningSpirVFrontEnd* translator,
                               LightningShaderIRLibrary* shaderLibrary,
                               ShaderTypeGroups& types,
                               Array<LightningShaderIRType*>& matrixTypes);
  void RegisterQuaternionFunctions(LightningSpirVFrontEnd* translator,
                                   LightningShaderIRLibrary* shaderLibrary,
                                   ShaderTypeGroups& types,
                                   LightningShaderIRType* quaternionType);

  LightningShaderIRLibraryRef mLibraryRef;
  static LightningShaderIRCore* mInstance;
};

} // namespace Plasma
