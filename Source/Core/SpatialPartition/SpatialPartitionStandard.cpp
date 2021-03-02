// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineStaticLibrary(SpatialPartitionLibrary)
{
  builder.CreatableInScriptDefault = false;

  LightningInitializeType(IBroadPhase);
  LightningInitializeType(NSquaredBroadPhase);
  LightningInitializeType(BoundingBoxBroadPhase);
  LightningInitializeType(BoundingSphereBroadPhase);
  LightningInitializeType(StaticAabbTreeBroadPhase);
  LightningInitializeType(SapBroadPhase);
  LightningInitializeType(DynamicAabbTreeBroadPhase);
  LightningInitializeType(AvlDynamicAabbTreeBroadPhase);
  LightningInitializeType(DynamicBroadphasePropertyExtension);
  LightningInitializeType(StaticBroadphasePropertyExtension);

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

void SpatialPartitionLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

void SpatialPartitionLibrary::Shutdown()
{
}

} // namespace Plasma
