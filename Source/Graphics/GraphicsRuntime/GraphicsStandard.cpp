// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Plasma
{

// Ranges
LightningDefineRange(GraphicalEntryRange);
LightningDefineRange(MultiSpriteEntryRange);
LightningDefineRange(VertexSemanticRange);
LightningDefineRange(ParticleListRange);
LightningDefineRange(Array<HandleOf<Material>>);
LightningDefineRange(Array<HandleOf<RenderGroup>>);

// Enums
LightningDefineEnum(BlendMode);
LightningDefineEnum(BlendFactor);
LightningDefineEnum(BlendEquation);
LightningDefineEnum(CullMode);
LightningDefineEnum(DepthMode);
LightningDefineEnum(GraphicalSortMethod);
LightningDefineEnum(MeshEmitMode);
LightningDefineEnum(PerspectiveMode);
LightningDefineEnum(PrimitiveType);
LightningDefineEnum(SpriteGeometryMode);
LightningDefineEnum(SpriteParticleAnimationMode);
LightningDefineEnum(SpriteParticleGeometryMode);
LightningDefineEnum(SpriteParticleSortMode);
LightningDefineEnum(StencilMode);
LightningDefineEnum(StencilOp);
LightningDefineEnum(SystemSpace);
LightningDefineEnum(TextAlign);
LightningDefineEnum(TextureCompareFunc);
LightningDefineEnum(TextureCompareMode);
LightningDefineEnum(VertexElementType);
LightningDefineEnum(VertexSemantic);
LightningDefineEnum(ViewportScaling);

LightningDeclareExternalType(GraphicsDriverSupport);
LightningDeclareExternalType(SamplerSettings);

LightningDefineStaticLibrary(GraphicsLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  LightningInitializeRange(GraphicalEntryRange);
  LightningInitializeRange(MultiSpriteEntryRange);
  LightningInitializeRange(ParticleListRange);
  LightningInitializeRange(VertexSemanticRange);
  LightningInitializeRange(Array<HandleOf<Material>>);
  LightningInitializeRange(Array<HandleOf<RenderGroup>>);

  // Enums
  LightningInitializeEnum(BlendMode);
  LightningInitializeEnum(BlendFactor);
  LightningInitializeEnum(BlendEquation);
  LightningInitializeEnum(CullMode);
  LightningInitializeEnum(DepthMode);
  LightningInitializeEnum(GraphicalSortMethod);
  LightningInitializeEnum(MeshEmitMode);
  LightningInitializeEnum(PerspectiveMode);
  LightningInitializeEnum(PrimitiveType);
  LightningInitializeEnum(SpriteGeometryMode);
  LightningInitializeEnum(SpriteParticleAnimationMode);
  LightningInitializeEnum(SpriteParticleGeometryMode);
  LightningInitializeEnum(SpriteParticleSortMode);
  LightningInitializeEnum(StencilMode);
  LightningInitializeEnum(StencilOp);
  LightningInitializeEnum(SystemSpace);
  LightningInitializeEnum(TextAlign);
  LightningInitializeEnum(TextureCompareFunc);
  LightningInitializeEnum(TextureCompareMode);
  LightningInitializeEnum(VertexElementType);
  LightningInitializeEnum(VertexSemantic);
  LightningInitializeEnum(ViewportScaling);

  // Meta Components
  LightningInitializeType(MaterialFactory);
  LightningInitializeType(CompositionLabelExtension);
  LightningInitializeType(HideBaseFilter);

  // Events
  LightningInitializeType(GraphicalEvent);
  LightningInitializeType(GraphicalSortEvent);
  LightningInitializeType(ParticleEvent);
  LightningInitializeType(RenderTasksEvent);
  LightningInitializeType(ResourceListEvent);
  LightningInitializeType(ShaderInputsEvent);

  LightningInitializeType(Graphical);
  LightningInitializeType(SlicedDefinition);

  LightningInitializeType(Atlas);
  LightningInitializeType(BaseSprite);
  LightningInitializeTypeAs(GraphicsBlendSettings, "BlendSettings");
  LightningInitializeType(BlendSettingsMrt);
  LightningInitializeType(Bone);
  LightningInitializeType(Camera);
  LightningInitializeType(ColorTargetMrt);
  LightningInitializeType(DebugGraphical);
  LightningInitializeType(DebugGraphicalPrimitive);
  LightningInitializeType(DebugGraphicalThickLine);
  LightningInitializeType(DebugGraphicalText);
  LightningInitializeType(DefinitionSet);
  LightningInitializeTypeAs(GraphicsDepthSettings, "DepthSettings");
  // LightningInitializeType(DynamicMeshParticleEmitter);
  LightningInitializeType(Font);
  LightningInitializeType(GraphicalEntry);
  LightningInitializeType(GraphicalRangeInterface);
  LightningInitializeExternalType(GraphicsDriverSupport);
  LightningInitializeType(GraphicsEngine);
  LightningInitializeType(GraphicsRaycastProvider);
  LightningInitializeType(GraphicsSpace);
  LightningInitializeType(HeightMapModel);
  LightningInitializeType(ImageDefinition);
  LightningInitializeType(IndexBuffer);
  LightningInitializeType(Material);
  LightningInitializeType(MaterialBlock);
  LightningInitializeType(MaterialList);
  LightningInitializeType(Mesh);
  LightningInitializeType(Model);
  LightningInitializeType(MultiRenderTarget);
  LightningInitializeType(MultiSprite);
  LightningInitializeType(MultiSpriteEntry);
  LightningInitializeType(Particle);
  LightningInitializeType(ParticleAnimator);
  LightningInitializeType(ParticleCollisionPlane);
  LightningInitializeType(ParticleCollisionHeightmap);
  LightningInitializeType(ParticleEmitter);
  LightningInitializeType(ParticleSystem);
  LightningInitializeTypeAs(ProxyObject<MaterialBlock>, "MaterialBlockProxy");
  LightningInitializeType(RenderGroup);
  LightningInitializeType(RenderGroupList);
  LightningInitializeTypeAs(GraphicsRenderSettings, "RenderSettings");
  LightningInitializeType(RenderTarget);
  LightningInitializeExternalType(SamplerSettings);
  LightningInitializeType(SelectionIcon);
  LightningInitializeType(ShaderInputs);
  LightningInitializeType(Skeleton);
  LightningInitializeType(SkinnedModel);
  LightningInitializeType(Sprite);
  LightningInitializeType(SpriteParticleSystem);
  LightningInitializeType(SpriteSource);
  LightningInitializeType(SpriteText);
  LightningInitializeType(SubRenderGroupPass);
  LightningInitializeType(TextDefinition);
  LightningInitializeType(Texture);
  LightningInitializeType(TextureData);
  LightningInitializeType(VertexBuffer);
  LightningInitializeType(ViewportInterface);
  LightningInitializeType(LightningFragment);

  // Dependent on RenderGroupList
  LightningInitializeType(ChildRenderGroupList);

  LightningInitializeTypeAs(GraphicsStatics, "Graphics");

  // Particle Animators
  LightningInitializeType(LinearParticleAnimator);
  LightningInitializeType(ParticleAttractor);
  LightningInitializeType(ParticleColorAnimator);
  LightningInitializeType(ParticleTwister);
  LightningInitializeType(ParticleWander);

  // Particle Emitters
  LightningInitializeType(ParticleEmitterShared);
  LightningInitializeType(BoxParticleEmitter);
  LightningInitializeType(MeshParticleEmitter);
  LightningInitializeType(SphericalParticleEmitter);
  LightningInitializeType(MeshParticleEmitter);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void GraphicsLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  ShaderIntrinsicsLibrary::InitializeInstance();

  InitializeResourceManager(DefinitionSetManager);
  InitializeResourceManager(FontManager);

  InitializeResourceManager(AtlasManager);
  InitializeResourceManager(MaterialManager);
  InitializeResourceManager(MeshManager);
  InitializeResourceManager(RenderGroupManager);
  InitializeResourceManager(SpriteSourceManager);
  InitializeResourceManager(TextureManager);
  InitializeResourceManager(LightningFragmentManager);

  ResourceLibrary::sFragmentType = LightningTypeId(LightningFragment);
}

void GraphicsLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
  ShaderIntrinsicsLibrary::Destroy();
}

} // namespace Plasma
