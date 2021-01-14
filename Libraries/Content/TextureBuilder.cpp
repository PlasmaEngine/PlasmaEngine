// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "ContentEnumerations.hpp"

namespace Plasma
{

LightningDefineType(TextureInfo, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);
  PlasmaBindDependency(ImageContent);
  PlasmaBindExpanded();

  LightningBindGetterProperty(FileType);
  LightningBindGetterProperty(LoadFormat);
  LightningBindGetterProperty(Dimensions);
  LightningBindGetterProperty(Size);
}

void TextureInfo::Serialize(Serializer& stream)
{
  SerializeNameDefault(mFileType, String(""));
  SerializeNameDefault(mLoadFormat, String(""));
  SerializeNameDefault(mDimensions, String(""));
  SerializeNameDefault(mSize, String(""));
}

void TextureInfo::Generate(ContentInitializer& initializer)
{
}

String TextureInfo::GetFileType()
{
  return mFileType;
}

String TextureInfo::GetLoadFormat()
{
  return mLoadFormat;
}

String TextureInfo::GetDimensions()
{
  return mDimensions;
}

String TextureInfo::GetSize()
{
  return mSize;
}

LightningDefineType(ShowPremultipliedAlphaFilter, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

bool ShowPremultipliedAlphaFilter::Filter(Member* prop, HandleParam instance)
{
  TextureBuilder* builder = instance.Get<TextureBuilder*>();
  ImageContent* content = (ImageContent*)builder->mOwner;
  TextureInfo* info = content->has(TextureInfo);
  if (info == nullptr)
    return true;

  return info->mLoadFormat == "RGBA8" || info->mLoadFormat == "RGBA16" || info->mLoadFormat == "SRGB8A8";
}

LightningDefineType(ShowGammaCorrectionFilter, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

bool ShowGammaCorrectionFilter::Filter(Member* prop, HandleParam instance)
{
  TextureBuilder* builder = instance.Get<TextureBuilder*>();
  ImageContent* content = (ImageContent*)builder->mOwner;
  TextureInfo* info = content->has(TextureInfo);
  if (info == nullptr)
    return true;

  return info->mLoadFormat != "RGB32f";
}

LightningDefineType(TextureBuilder, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindDependency(ImageContent);
  PlasmaBindExpanded();

  LightningBindFieldProperty(Name);
  LightningBindFieldProperty(mType);
  LightningBindFieldProperty(mCompression);
  LightningBindFieldProperty(mAddressingX);
  LightningBindFieldProperty(mAddressingY);
  LightningBindFieldProperty(mFiltering);
  LightningBindFieldProperty(mAnisotropy);
  LightningBindFieldProperty(mMipMapping);
  LightningBindGetterSetterProperty(HalfScaleCount);
  LightningBindFieldProperty(mPremultipliedAlpha)->Add(new ShowPremultipliedAlphaFilter());
  LightningBindFieldProperty(mGammaCorrection)->Add(new ShowGammaCorrectionFilter());
}

void TextureBuilder::Serialize(Serializer& stream)
{
  SerializeName(Name);
  SerializeName(mResourceId);

  SerializeEnumNameDefault(TextureType, mType, TextureType::Texture2D);
  SerializeEnumNameDefault(TextureCompression, mCompression, TextureCompression::None);
  SerializeEnumNameDefault(TextureAddressing, mAddressingX, TextureAddressing::Repeat);
  SerializeEnumNameDefault(TextureAddressing, mAddressingY, TextureAddressing::Repeat);
  SerializeEnumNameDefault(TextureFiltering, mFiltering, TextureFiltering::Trilinear);
  SerializeEnumNameDefault(TextureAnisotropy, mAnisotropy, TextureAnisotropy::x16);
  SerializeEnumNameDefault(TextureMipMapping, mMipMapping, TextureMipMapping::PreGenerated);
  SerializeNameDefault(mHalfScaleCount, 0);
  SerializeNameDefault(mPremultipliedAlpha, false);
  SerializeNameDefault(mGammaCorrection, false);
}

void TextureBuilder::Initialize(ContentComposition* item)
{
  BuilderComponent::Initialize(item);
}

void TextureBuilder::Generate(ContentInitializer& initializer)
{
  mResourceId = GenerateUniqueId64();
  Name = initializer.Name;

  mType = TextureType::Texture2D;
  mCompression = TextureCompression::None;
  mAddressingX = TextureAddressing::Repeat;
  mAddressingY = TextureAddressing::Repeat;
  mFiltering = TextureFiltering::Trilinear;
  mAnisotropy = TextureAnisotropy::x16;
  mMipMapping = TextureMipMapping::PreGenerated;

  mPremultipliedAlpha = false;
  mGammaCorrection = false;

  String filename = initializer.Filename.ToLower();
  if (initializer.Extension.ToLower() == "hdr")
  {
    mType = TextureType::TextureCube;
    mAddressingX = TextureAddressing::Clamp;
    mAddressingY = TextureAddressing::Clamp;
  }
  else if (AlbedoString(filename))
  {
    mCompression = TextureCompression::BC1;
    mGammaCorrection = true;
  }
  else if (NormalString(filename))
    mCompression = TextureCompression::BC5;
  else if (MetallicString(filename) && RoughnessString(filename))
    mCompression = TextureCompression::BC5;
  else if (MetallicString(filename) || RoughnessString(filename) || AOString(filename))
    mCompression = TextureCompression::BC4;
}

bool TextureBuilder::AlbedoString(String name)
{
  return name.Contains("albedo") || name.Contains("diff") || name.Contains("diffuse");
}

bool TextureBuilder::NormalString(String name)
{
  return name.Contains("normal") || name.Contains("norm");
}

bool TextureBuilder::MetallicString(String name)
{
  return name.Contains("metallic") || name.Contains("metalness") || name.Contains("mtl") || name.Contains("metal");
}

bool TextureBuilder::RoughnessString(String name)
{
  return name.Contains("roughness") || name.Contains("rough") || name.Contains("_r");
}

bool TextureBuilder::AOString(String name)
{
  return name.Contains("_ao") || name.Contains("occlusion");
}

bool TextureBuilder::NeedsBuilding(BuildOptions& options)
{
  String destFile = FilePath::Combine(options.OutputPath, GetOutputFile());
  String sourceFile = FilePath::Combine(options.SourcePath, mOwner->Filename);
  return CheckFileAndMeta(options, sourceFile, destFile);
}

void TextureBuilder::BuildListing(ResourceListing& listing)
{
  String loader = PTexLoader;
  String destFile = GetOutputFile();
  listing.PushBack(ResourceEntry(0, loader, Name, destFile, mResourceId, this->mOwner, this));
}

void TextureBuilder::BuildContent(BuildOptions& buildOptions)
{
  String inputFile = FilePath::Combine(buildOptions.SourcePath, mOwner->Filename);
  String outputFile = FilePath::Combine(buildOptions.OutputPath, GetOutputFile());

  TextureImporter importer(inputFile, outputFile, String());

  Status status;
  ImageProcessorCodes::Enum result = importer.ProcessTexture(status);

  switch (result)
  {
  case ImageProcessorCodes::Success:
    break;
  case ImageProcessorCodes::Failed:
    buildOptions.Failure = true;
    buildOptions.Message = "Failed to process texture.";
    break;
  case ImageProcessorCodes::Reload:
    ((ImageContent*)mOwner)->mReload = true;
    break;
  }
}

void TextureBuilder::Rename(StringParam newName)
{
  Name = newName;
}

int TextureBuilder::GetHalfScaleCount()
{
  return mHalfScaleCount;
}

void TextureBuilder::SetHalfScaleCount(int halfScaleCount)
{
  mHalfScaleCount = Math::Max(halfScaleCount, 0);
}

String TextureBuilder::GetOutputFile()
{
  return BuildString(Name, ".ptex");
}

// PlasmaDefineType(HeightToNormalBuilder);
//
// void HeightToNormalBuilder::Generate(ContentInitializer& initializer)
//{
//  mNormalSource = NormalGeneration::AverageRGB;
//  mNormalFilter = NormalFilter::n3x3;
//  mStoreHeightInAlpha = false;
//  mBumpiness = 1.0f;
//}
//
// void HeightToNormalBuilder::Serialize(Serializer& stream)
//{
//  SerializeEnumName(NormalGeneration, mNormalSource);
//  SerializeEnumName(NormalFilter, mNormalFilter);
//  SerializeName(mBumpiness);
//  SerializeName(mStoreHeightInAlpha);
//}
//
// void HeightToNormalBuilder::InitializeMeta(MetaType* meta)
//{
//  PlasmaBindDependency(ImageContent);
//  PlasmaBindDependency(TextureBuilder);
//}

} // namespace Plasma
