// MIT Licensed (see LICENSE.md).

#pragma once

// Other projects
#include "Core/Common/CommonStandard.hpp"
#include "PlatformStandard.hpp"
#include "Core/Engine/EngineStandard.hpp"
#include "Core/Meta/MetaStandard.hpp"
#include "Core/SpatialPartition/SpatialPartitionStandard.hpp"
#include "Lightning/LightningShaders/LightningShadersStandard.hpp"

namespace Plasma
{

// Graphics library
class PlasmaNoImportExport GraphicsLibrary : public Lightning::StaticLibrary
{
public:
  LightningDeclareStaticLibraryInternals(GraphicsLibrary, "PlasmaEngine");

  static void Initialize();
  static void Shutdown();
};

} // namespace Plasma

#include "ForwardDeclarations.hpp"
#include "UtilityStructures.hpp"
#include "ResourceLists.hpp"

// No Dependencies
#include "Camera.hpp"
#include "Font.hpp"
#include "GraphicalEntry.hpp"
#include "GraphicsRaycastProvider.hpp"
#include "MaterialBlock.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Particle.hpp"
#include "ParticleAnimator.hpp"
#include "ParticleEmitter.hpp"
#include "PerspectiveTransforms.hpp"
#include "PixelBuffer.hpp"
#include "RenderGroup.hpp"
#include "RendererGlobal.hpp"
#include "RenderSettings.hpp"
#include "RenderTarget.hpp"
#include "RenderTasks.hpp"
#include "Skeleton.hpp"
#include "SpriteSource.hpp"
#include "Texture.hpp"
#include "TextureData.hpp"
#include "TextureLoader.hpp"
#include "TextureUtilities.hpp"
#include "ViewportInterface.hpp"
#include "VisibilityFlag.hpp"
#include "LightningFragment.hpp"
#include "PlasmaLightningShaderGlslBackend.hpp"
#include "LightningShaderGenerator.hpp"

// Some Dependencies
#include "Atlas.hpp"
#include "FontPattern.hpp"
#include "MaterialFactory.hpp"
#include "ParticleEmitters.hpp"
#include "RendererThread.hpp"

// Base Graphicals
#include "Graphical.hpp"
#include "ParticleSystem.hpp"
#include "ParticleAnimators.hpp"

// Graphicals
#include "DebugGraphical.hpp"
#include "HeightMapModel.hpp"
#include "Model.hpp"
#include "SelectionIcon.hpp"
#include "SkinnedModel.hpp"
#include "Sprite.hpp"
#include "SpriteSystem.hpp"

#include "GraphicsSpace.hpp"

#include "GraphicsEngine.hpp"

// Deprecate
#include "Definition.hpp"
#include "Image.hpp"
#include "Text.hpp"
