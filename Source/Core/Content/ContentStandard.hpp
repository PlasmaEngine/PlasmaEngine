// MIT Licensed (see LICENSE.md).
#pragma once

#include "Core/Common/CommonStandard.hpp"
#include "PlatformStandard.hpp"
#include "Core/Engine/EngineStandard.hpp"
#include "Core/SpatialPartition/SpatialPartitionStandard.hpp"

namespace Plasma
{
// Forward declarations
class ContentLibrary;
class ContentItem;
class ContentComponent;
class BuildOptions;
class ContentComposition;

// Content library
class PlasmaNoImportExport ContentMetaLibrary : public Lightning::StaticLibrary
{
public:
  LightningDeclareStaticLibraryInternals(ContentMetaLibrary, "PlasmaEngine");

  static void Initialize();
  static void Shutdown();
};

} // namespace Plasma

// Our includes
#include "FileExtensionManager.hpp"
#include "ContentItem.hpp"
#include "ContentLibrary.hpp"
#include "BuildOptions.hpp"
#include "ContentSystem.hpp"
#include "ContentUtility.hpp"
#include "ContentComposition.hpp"
#include "DataContent.hpp"
#include "TagsContent.hpp"
#include "BaseBuilders.hpp"
#include "LightningPluginContent.hpp"
#include "VectorContent.hpp"
#include "TextureBuilder.hpp"
#include "AudioFileEncoder.hpp"
#include "AudioContent.hpp"
#include "ImportOptions.hpp"
#include "ImageContent.hpp"
#include "SpriteBuilder.hpp"
#include "TextContent.hpp"
#include "SupportComponents.hpp"
#include "RichAnimation.hpp"
#include "GeometryContent.hpp"
#include "BinaryContent.hpp"
#include "MeshBuilder.hpp"
#include "ResourceLoaderTemplates.hpp"
