// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// Enums
LightningDefineEnum(AudioCueImport);
LightningDefineEnum(BasisType);
LightningDefineEnum(ConflictAction);
LightningDefineEnum(ImageImport);
LightningDefineEnum(LoopingMode);
LightningDefineEnum(MeshImport);
LightningDefineEnum(PhysicsImport);
LightningDefineEnum(PhysicsMeshType);
LightningDefineEnum(ScaleConversion);
LightningDefineEnum(SpriteFill);
LightningDefineEnum(SpriteSampling);
LightningDefineEnum(TrackType);
LightningDefineEnum(TextureAddressing);
LightningDefineEnum(TextureAnisotropy);
LightningDefineEnum(TextureCompression);
LightningDefineEnum(TextureFace);
LightningDefineEnum(TextureFiltering);
LightningDefineEnum(TextureFormat);
LightningDefineEnum(TextureMipMapping);
LightningDefineEnum(TextureType);
LightningDefineEnum(AudioFileLoadType);

PlasmaDefineArrayType(Array<AnimationClip>);

LightningDefineStaticLibrary(ContentMetaLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Enums
  LightningInitializeEnum(AudioCueImport);
  LightningInitializeEnum(BasisType);
  LightningInitializeEnum(ConflictAction);
  LightningInitializeEnum(ImageImport);
  LightningInitializeEnum(LoopingMode);
  LightningInitializeEnum(MeshImport);
  LightningInitializeEnum(PhysicsImport);
  LightningInitializeEnum(PhysicsMeshType);
  LightningInitializeEnum(ScaleConversion);
  LightningInitializeEnum(SpriteFill);
  LightningInitializeEnum(SpriteSampling);
  LightningInitializeEnum(TrackType);
  LightningInitializeEnum(TextureAddressing);
  LightningInitializeEnum(TextureAnisotropy);
  LightningInitializeEnum(TextureCompression);
  LightningInitializeEnum(TextureFace);
  LightningInitializeEnum(TextureFiltering);
  LightningInitializeEnum(TextureFormat);
  LightningInitializeEnum(TextureMipMapping);
  LightningInitializeEnum(TextureType);
  LightningInitializeEnum(AudioFileLoadType);

  PlasmaInitializeArrayTypeAs(Array<AnimationClip>, "AnimationClips");

  // Meta Components
  LightningInitializeType(ContentMetaComposition);
  LightningInitializeType(ContentItemMetaOperations);

  // Events
  LightningInitializeType(ContentSystemEvent);
  LightningInitializeType(KeyFrameEvent);
  LightningInitializeType(TrackEvent);

  LightningInitializeType(ContentItem);
  LightningInitializeType(ContentLibrary);
  LightningInitializeType(ContentSystem);
  LightningInitializeType(ContentComposition);
  LightningInitializeType(ContentComponent);
  LightningInitializeType(BuilderComponent);
  LightningInitializeType(DataContent);
  LightningInitializeType(DataBuilder);
  LightningInitializeType(ContentTags);
  LightningInitializeType(LightningPluginBuilder);
  LightningInitializeType(FontContent);
  LightningInitializeType(FontBuilder);
  LightningInitializeType(ImageContent);
  LightningInitializeType(ImageOptions);
  LightningInitializeType(ShowNormalGenerationOptionsFilter);
  LightningInitializeType(GeometryOptions);
  LightningInitializeType(AudioOptions);
  LightningInitializeType(ConflictOptions);
  LightningInitializeType(ImportOptions);
  LightningInitializeType(ShowPremultipliedAlphaFilter);
  LightningInitializeType(ShowGammaCorrectionFilter);
  LightningInitializeType(TextureBuilder);
  LightningInitializeType(SpriteData);
  LightningInitializeType(SpriteSourceBuilder);
  LightningInitializeType(TextContent);
  LightningInitializeType(BaseTextBuilder);
  LightningInitializeType(TextBuilder);
  LightningInitializeType(LightningScriptBuilder);
  LightningInitializeType(LightningFragmentBuilder);
  LightningInitializeType(ContentCopyright);
  LightningInitializeType(ContentHistory);
  LightningInitializeType(ContentNotes);
  LightningInitializeType(ContentEditorOptions);
  LightningInitializeType(ResourceTemplate);
  LightningInitializeType(RichAnimation);
  LightningInitializeType(RichAnimationBuilder);
  LightningInitializeType(TrackNode);
  LightningInitializeType(GeometryImport);
  LightningInitializeType(GeometryResourceEntry);
  LightningInitializeType(MeshBuilder);
  LightningInitializeType(PhysicsMeshBuilder);
  LightningInitializeType(AnimationClip);
  LightningInitializeType(AnimationBuilder);
  LightningInitializeType(TextureContent);
  LightningInitializeType(GeometryContent);
  LightningInitializeType(AudioContent);
  LightningInitializeType(TextureInfo);
  LightningInitializeType(SoundBuilder);
  LightningInitializeType(BinaryContent);
  LightningInitializeType(BinaryBuilder);
  LightningInitializeType(GeneratedArchetype);

  // @trevor.sundberg: The content and engine libraries are co-dependent, and
  // since content references the Archetype type, we get an error that it hasn't
  // yet been initialized since Content is initialized first. This prevents the
  // assert:
  LightningTypeId(Archetype)->AssertOnInvalidBinding = &IgnoreOnInvalidBinding;

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

void ContentMetaLibrary::Initialize()
{
  ContentSystem::Initialize();
  PL::gContentSystem = ContentSystem::GetInstance();

  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

void ContentMetaLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
  ContentSystem::Destroy();
}

} // namespace Plasma
