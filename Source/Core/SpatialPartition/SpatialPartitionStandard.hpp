// MIT Licensed (see LICENSE.md).
#pragma once

// Standard includes
#include "Core/Geometry/GeometryStandard.hpp"
#include "Core/Serialization/SerializationStandard.hpp"

namespace Plasma
{

// SpatialPartition library
class PlasmaNoImportExport SpatialPartitionLibrary : public Lightning::StaticLibrary
{
public:
  LightningDeclareStaticLibraryInternals(SpatialPartitionLibrary, "PlasmaEngine");

  static void Initialize();
  static void Shutdown();
};

} // namespace Plasma

// Project includes
#include "BroadPhaseProxy.hpp"
#include "ProxyCast.hpp"
#include "BroadPhase.hpp"
#include "SimpleCastCallbacks.hpp"
#include "BroadPhaseRanges.hpp"
#include "BaseDynamicAabbTreeBroadPhase.hpp"
#include "DynamicTreeHelpers.hpp"
#include "BaseDynamicAabbTree.hpp"
#include "AvlDynamicAabbTree.hpp"
#include "DynamicAabbTree.hpp"
#include "DynamicAabbTreeBroadPhase.hpp"
#include "AvlDynamicAabbTreeBroadPhase.hpp"
#include "BaseNSquared.hpp"
#include "NSquared.hpp"
#include "NSquaredBroadPhase.hpp"
#include "BoundingBox.hpp"
#include "BoundingBoxBroadPhase.hpp"
#include "BoundingSphere.hpp"
#include "BoundingSphereBroadPhase.hpp"
#include "SapContainers.hpp"
#include "Sap.hpp"
#include "SapBroadPhase.hpp"
#include "AabbTreeNode.hpp"
#include "AabbTreeMethods.hpp"
#include "StaticAabbTree.hpp"
#include "StaticAabbTreeBroadPhase.hpp"
#include "BroadPhasePackage.hpp"
#include "BroadPhaseCreator.hpp"
#include "BroadPhaseTracker.hpp"
