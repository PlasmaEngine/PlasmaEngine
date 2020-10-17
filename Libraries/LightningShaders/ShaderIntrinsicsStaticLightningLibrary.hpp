// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

/// The lightning shader library wrapper around Lightning's ShaderIntrinsics library.
/// This needs to be built and have the Parse function called once before all
/// shader translation. Contains the image/sampler types.
class ShaderIntrinsicsStaticLightningLibrary
{
public:
  static void InitializeInstance();
  static void Destroy();
  static ShaderIntrinsicsStaticLightningLibrary& GetInstance();

  ShaderIntrinsicsStaticLightningLibrary();
  /// Parse the ShaderIntrinsics library and make all backing shader types.
  void Parse(LightningSpirVFrontEnd* translator);
  LightningShaderIRLibraryRef GetLibrary();

private:
  void CreateImageAndSampler(LightningSpirVFrontEnd* translator,
                             LightningShaderIRLibrary* shaderLibrary,
                             Lightning::BoundType* lightningSampledType,
                             Lightning::BoundType* lightningImageType,
                             Lightning::BoundType* lightningSampledImageType,
                             int dimension,
                             int depthMode);
  void CreateStorageImage(LightningSpirVFrontEnd* translator,
                          LightningShaderIRLibrary* shaderLibrary,
                          Lightning::BoundType* lightningSampledType,
                          Lightning::BoundType* lightningImageType,
                          int dimension,
                          int depthMode,
                          int imageFormat);

  void PopulateStageRequirementsData(LightningShaderIRLibrary* shaderLibrary);

  LightningShaderIRLibraryRef mLibraryRef;
  static ShaderIntrinsicsStaticLightningLibrary* mInstance;
};

} // namespace Plasma
