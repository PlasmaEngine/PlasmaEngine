// MIT Licensed (see LICENSE.md).
#pragma once

namespace Lightning
{

class Sampler
{
  LightningDeclareType(Sampler, TypeCopyMode::ValueType);
};

class Image2d
{
  LightningDeclareType(Image2d, TypeCopyMode::ValueType);
};

class StorageImage2d
{
  LightningDeclareType(StorageImage2d, TypeCopyMode::ValueType);
};

class DepthImage2d
{
  LightningDeclareType(DepthImage2d, TypeCopyMode::ValueType);
};

class ImageCube
{
  LightningDeclareType(ImageCube, TypeCopyMode::ValueType);
};

/// Represents a sampler combined with an image.
class SampledImage2d
{
  LightningDeclareType(SampledImage2d, TypeCopyMode::ValueType);
};

class SampledDepthImage2d
{
  LightningDeclareType(SampledDepthImage2d, TypeCopyMode::ValueType);
};

class SampledImageCube
{
  LightningDeclareType(SampledImageCube, TypeCopyMode::ValueType);
};

// User data written to LightningFunctions to make image/sampler functions easier to
// translate
struct ImageUserData
{
  ImageUserData(u32 optionalOperands, u32 flags, Lightning::BoundType* sampledImageType = nullptr)
  {
    mOptionalOperands = optionalOperands;
    mImageOperandFlags = flags;
    mSampledImageType = sampledImageType;
  }
  // How many optional operands exist
  u32 mOptionalOperands;
  // Flags that have to come before the before the optional
  // operands that specify what the extra operands are.
  u32 mImageOperandFlags;
  // The combined SampledImage type that some functions use. Added here to make
  // it easier to not have to look up the type of the SampledImage when needed.
  Lightning::BoundType* mSampledImageType;
};

} // namespace Lightning

namespace Plasma
{

// Represents a sampled image and what sampler + image were combined to make it
// (to make it easier to bind a bunch of functions on this "grouping")
struct SampledImageSet
{
  SampledImageSet()
  {
    mSamplerType = nullptr;
    mImageType = nullptr;
    mSampledImageType = nullptr;
  }
  Lightning::BoundType* mSamplerType;
  Lightning::BoundType* mImageType;
  Lightning::BoundType* mSampledImageType;
};

void AddSampleImplicitLod(Lightning::LibraryBuilder& builder,
                          Lightning::BoundType* type,
                          SampledImageSet& set,
                          Lightning::BoundType* coordinateType,
                          Lightning::BoundType* returnType);
void AddSampleExplicitLod(Lightning::LibraryBuilder& builder,
                          Lightning::BoundType* type,
                          SampledImageSet& set,
                          Lightning::BoundType* coordinateType,
                          Lightning::BoundType* lodType,
                          Lightning::BoundType* returnType);
void AddSampleGradExplicitLod(Lightning::LibraryBuilder& builder,
                              Lightning::BoundType* type,
                              SampledImageSet& set,
                              Lightning::BoundType* coordinateType,
                              Lightning::BoundType* derivativeType,
                              Lightning::BoundType* returnType);
void AddSampleDrefImplicitLod(Lightning::LibraryBuilder& builder,
                              Lightning::BoundType* type,
                              SampledImageSet& set,
                              Lightning::BoundType* coordinateType,
                              Lightning::BoundType* depthType,
                              Lightning::BoundType* returnType);
void AddSampleDrefExplicitLod(Lightning::LibraryBuilder& builder,
                              Lightning::BoundType* type,
                              SampledImageSet& set,
                              Lightning::BoundType* coordinateType,
                              Lightning::BoundType* depthType,
                              Lightning::BoundType* lodType,
                              Lightning::BoundType* returnType);
void AddSampleProjImplicitLod(Lightning::LibraryBuilder& builder,
                              Lightning::BoundType* type,
                              SampledImageSet& set,
                              Lightning::BoundType* coordinateType,
                              Lightning::BoundType* returnType);
void AddSampleProjExplicitLod(Lightning::LibraryBuilder& builder,
                              Lightning::BoundType* type,
                              SampledImageSet& set,
                              Lightning::BoundType* coordinateType,
                              Lightning::BoundType* lodType,
                              Lightning::BoundType* returnType);
void AddSampleProjDrefImplicitLod(Lightning::LibraryBuilder& builder,
                                  Lightning::BoundType* type,
                                  SampledImageSet& set,
                                  Lightning::BoundType* coordinateType,
                                  Lightning::BoundType* depthType,
                                  Lightning::BoundType* returnType);
void AddSampleProjDrefExplicitLod(Lightning::LibraryBuilder& builder,
                                  Lightning::BoundType* type,
                                  SampledImageSet& set,
                                  Lightning::BoundType* coordinateType,
                                  Lightning::BoundType* depthType,
                                  Lightning::BoundType* lodType,
                                  Lightning::BoundType* returnType);
void AddImageFetch(Lightning::LibraryBuilder& builder,
                   Lightning::BoundType* type,
                   SampledImageSet& set,
                   Lightning::BoundType* coordianteType,
                   Lightning::BoundType* lodType,
                   Lightning::BoundType* returnType);
void AddImageQuerySizeLod(Lightning::LibraryBuilder& builder,
                          Lightning::BoundType* type,
                          SampledImageSet& set,
                          Lightning::BoundType* lodType,
                          Lightning::BoundType* returnType);
void AddImageQuerySize(Lightning::LibraryBuilder& builder,
                       Lightning::BoundType* type,
                       SampledImageSet& set,
                       Lightning::BoundType* returnType);
void AddImageQueryLod(Lightning::LibraryBuilder& builder,
                      Lightning::BoundType* type,
                      SampledImageSet& set,
                      Lightning::BoundType* coordinateType,
                      Lightning::BoundType* returnType);
void AddImageQueryLevels(Lightning::LibraryBuilder& builder,
                         Lightning::BoundType* type,
                         SampledImageSet& set,
                         Lightning::BoundType* returnType);

// Add all of the relevant image intrinsics to the shader given bound type.
void AddImageFunctions(Lightning::LibraryBuilder& builder, Lightning::BoundType* type, LightningTypeGroups& types);

} // namespace Plasma
