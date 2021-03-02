// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "LightningShaderIRCore.hpp"
#include "ShaderIRLibraryTranslation.hpp"
#include "LightningShadersStandard.hpp"

namespace Plasma
{

ShaderIntrinsicsStaticLightningLibrary* ShaderIntrinsicsStaticLightningLibrary::mInstance = nullptr;

void ShaderIntrinsicsStaticLightningLibrary::InitializeInstance()
{
  ReturnIf(mInstance != nullptr, , "Can't initialize a static library more than once");
  mInstance = new ShaderIntrinsicsStaticLightningLibrary();
}

void ShaderIntrinsicsStaticLightningLibrary::Destroy()
{
  delete mInstance;
  mInstance = nullptr;
}

ShaderIntrinsicsStaticLightningLibrary& ShaderIntrinsicsStaticLightningLibrary::GetInstance()
{
  ErrorIf(mInstance == nullptr, "Attempted to get an uninitialized singleton static library");

  return *mInstance;
}

ShaderIntrinsicsStaticLightningLibrary::ShaderIntrinsicsStaticLightningLibrary()
{
}

void ShaderIntrinsicsStaticLightningLibrary::Parse(LightningSpirVFrontEnd* translator)
{
  LightningShaderIRLibrary* shaderLibrary = new LightningShaderIRLibrary();
  shaderLibrary->mLightningLibrary = Lightning::ShaderIntrinsicsLibrary::GetInstance().GetLibrary();
  shaderLibrary->mDependencies = new LightningShaderIRModule();
  shaderLibrary->mDependencies->PushBack(LightningShaderIRCore::GetInstance().GetLibrary());
  mLibraryRef = shaderLibrary;
  translator->mLibrary = shaderLibrary;

  Lightning::Core& core = Lightning::Core::GetInstance();
  Lightning::Library* lightningLibrary = shaderLibrary->mLightningLibrary;

  // Declare the unsigned int type. As this is currently a hack type, only do this for the scalar version. @JoshD: Cleanup
  LightningShaderIRType* uintType = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Uint, 1, nullptr, LightningTypeId(Lightning::UnsignedInt));

  // Grabbed a bunch of lightning types
  Lightning::BoundType* lightningSamplerType = LightningTypeId(Lightning::Sampler);
  Lightning::BoundType* lightningImage2d = LightningTypeId(Lightning::Image2d);
  Lightning::BoundType* lightningDepthImage2d = LightningTypeId(Lightning::DepthImage2d);
  Lightning::BoundType* lightningImageCube = LightningTypeId(Lightning::ImageCube);
  Lightning::BoundType* lightningSampledImage2d = LightningTypeId(Lightning::SampledImage2d);
  Lightning::BoundType* lightningSampledDepthImage2d = LightningTypeId(Lightning::SampledDepthImage2d);
  Lightning::BoundType* lightningSampledImageCube = LightningTypeId(Lightning::SampledImageCube);
  Lightning::BoundType* lightningStorageImage2d = LightningTypeId(Lightning::StorageImage2d);

  // Create the sampler type
  LightningShaderIRType* samplerType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                  ShaderIRTypeBaseType::Sampler,
                                                                  lightningSamplerType->Name,
                                                                  lightningSamplerType,
                                                                  spv::StorageClassUniformConstant);
  translator->MakeShaderTypeMeta(samplerType, nullptr);

  // Create images + sampledImaged types
  CreateImageAndSampler(translator, shaderLibrary, core.RealType, lightningImage2d, lightningSampledImage2d, spv::Dim2D, 0);
  CreateImageAndSampler(
      translator, shaderLibrary, core.RealType, lightningDepthImage2d, lightningSampledDepthImage2d, spv::Dim2D, 1);
  CreateImageAndSampler(
      translator, shaderLibrary, core.RealType, lightningImageCube, lightningSampledImageCube, spv::DimCube, 0);
  CreateStorageImage(
      translator, shaderLibrary, core.RealType, lightningStorageImage2d, spv::Dim2D, 0, spv::ImageFormatRgba32f);

  RegisterShaderIntrinsics(translator, shaderLibrary);

  // Register the template resolver for fixed array
  shaderLibrary->RegisterTemplateResolver("FixedArray[Type,Integer]", FixedArrayResolver);
  FixedArrayResolver(translator, shaderLibrary->mLightningLibrary->BoundTypes["FixedArray[Real4x4, 80]"]);
  String runtimeArayResolverName = BuildString(SpirVNameSettings::mRuntimeArrayTypeName, "[Type]");
  shaderLibrary->RegisterTemplateResolver(runtimeArayResolverName, RuntimeArrayResolver);

  shaderLibrary->RegisterTemplateResolver("PointInput[Type]", GeometryStreamInputResolver);
  shaderLibrary->RegisterTemplateResolver("LineInput[Type]", GeometryStreamInputResolver);
  shaderLibrary->RegisterTemplateResolver("TriangleInput[Type]", GeometryStreamInputResolver);
  shaderLibrary->RegisterTemplateResolver("PointOutput[Type]", GeometryStreamOutputResolver);
  shaderLibrary->RegisterTemplateResolver("LineOutput[Type]", GeometryStreamOutputResolver);
  shaderLibrary->RegisterTemplateResolver("TriangleOutput[Type]", GeometryStreamOutputResolver);
  shaderLibrary->mTranslated = true;

  PopulateStageRequirementsData(shaderLibrary);
}

LightningShaderIRLibraryRef ShaderIntrinsicsStaticLightningLibrary::GetLibrary()
{
  return mLibraryRef;
}

void ShaderIntrinsicsStaticLightningLibrary::CreateImageAndSampler(LightningSpirVFrontEnd* translator,
                                                               LightningShaderIRLibrary* shaderLibrary,
                                                               Lightning::BoundType* lightningSampledType,
                                                               Lightning::BoundType* lightningImageType,
                                                               Lightning::BoundType* lightningSampledImageType,
                                                               int dimension,
                                                               int depthMode)
{
  LightningShaderIRType* sampledType = shaderLibrary->FindType(lightningSampledType);
  // Create the base image type for a sampled image
  LightningShaderIRType* imageType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                ShaderIRTypeBaseType::Image,
                                                                lightningImageType->Name,
                                                                lightningImageType,
                                                                spv::StorageClassUniformConstant);
  imageType->mParameters.PushBack(sampledType);                                              // SampledType
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(dimension)); // Dim
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(depthMode)); // Depth
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(0));         // Arrayed
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(0));         // MultiSampled
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(1));         // Sampled
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(spv::ImageFormatUnknown));
  translator->MakeShaderTypeMeta(imageType, nullptr);

  LightningShaderIRType* sampledImageType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                       ShaderIRTypeBaseType::SampledImage,
                                                                       lightningSampledImageType->Name,
                                                                       lightningSampledImageType,
                                                                       spv::StorageClassUniformConstant);
  sampledImageType->mParameters.PushBack(imageType);
  translator->MakeShaderTypeMeta(sampledImageType, nullptr);
}

void ShaderIntrinsicsStaticLightningLibrary::CreateStorageImage(LightningSpirVFrontEnd* translator,
                                                            LightningShaderIRLibrary* shaderLibrary,
                                                            Lightning::BoundType* lightningSampledType,
                                                            Lightning::BoundType* lightningImageType,
                                                            int dimension,
                                                            int depthMode,
                                                            int imageFormat)
{
  LightningShaderIRType* sampledType = shaderLibrary->FindType(lightningSampledType);
  // Create the base image type for a sampled image
  LightningShaderIRType* imageType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                ShaderIRTypeBaseType::Image,
                                                                lightningImageType->Name,
                                                                lightningImageType,
                                                                spv::StorageClassUniformConstant);
  imageType->mParameters.PushBack(sampledType);                                              // SampledType
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(dimension)); // Dim
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(depthMode)); // Depth
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(0));         // Arrayed
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(0));         // MultiSampled
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(2));         // Sampled
  imageType->mParameters.PushBack(translator->GetOrCreateConstantIntegerLiteral(imageFormat));
  translator->MakeShaderTypeMeta(imageType, nullptr);
}

void ShaderIntrinsicsStaticLightningLibrary::PopulateStageRequirementsData(LightningShaderIRLibrary* shaderLibrary)
{
  // Find all lightning functions that have the [RequiresPixel] attribute.
  // These need to be processed into the stage requirements cached in the
  // library/
  Lightning::Library* lightningLibrary = shaderLibrary->mLightningLibrary;
  for (size_t i = 0; i < lightningLibrary->OwnedFunctions.Size(); ++i)
  {
    Lightning::Function* lightningFunction = lightningLibrary->OwnedFunctions[i];
    if (lightningFunction->HasAttribute(SpirVNameSettings::mRequiresPixelAttribute))
      shaderLibrary->mStageRequirementsData[lightningFunction].Combine(
          nullptr, lightningFunction->Location, ShaderStage::Pixel);
  }
}

} // namespace Plasma
