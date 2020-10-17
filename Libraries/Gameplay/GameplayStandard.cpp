// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "IndexedHalfEdgeMesh.hpp"
#include "QuickHull3DBindings.hpp"

namespace Plasma
{

// Enums
LightningDefineEnum(OrientationBases);
LightningDefineEnum(SplineAnimatorMode);
LightningDefineEnum(PathFinderStatus);

LightningDefineRange(IndexedHalfEdgeMeshVertexArray::RangeType);
LightningDefineRange(IndexedHalfEdgeMeshEdgeArray::RangeType);
LightningDefineRange(IndexedHalfEdgeFaceEdgeIndexArray::RangeType);
LightningDefineRange(IndexedHalfEdgeMeshFaceArray::RangeType);

LightningDefineStaticLibrary(GameplayLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Enums
  LightningInitializeEnum(OrientationBases);
  LightningInitializeEnum(SplineAnimatorMode);
  LightningInitializeEnum(PathFinderStatus);

  // Ranges
  LightningInitializeRangeAs(IndexedHalfEdgeMeshVertexArray::RangeType, "IndexedHalfEdgeMeshVertexArrayRange");
  LightningInitializeRangeAs(IndexedHalfEdgeMeshEdgeArray::RangeType, "IndexedHalfEdgeMeshEdgeArrayRange");
  LightningInitializeRangeAs(IndexedHalfEdgeFaceEdgeIndexArray::RangeType, "IndexedHalfEdgeFaceEdgeIndexArrayRange");
  LightningInitializeRangeAs(IndexedHalfEdgeMeshFaceArray::RangeType, "IndexedHalfEdgeMeshFaceArrayRange");

  // Events
  LightningInitializeType(MouseEvent);
  LightningInitializeType(MouseFileDropEvent);
  LightningInitializeType(ViewportMouseEvent);

  LightningInitializeType(Viewport);
  LightningInitializeType(ReactiveViewport);
  LightningInitializeType(GameWidget);

  LightningInitializeType(TileMapSource);
  LightningInitializeType(Reactive);
  LightningInitializeType(ReactiveSpace);
  LightningInitializeType(MouseCapture);
  LightningInitializeType(Orientation);
  LightningInitializeType(TileMap);
  LightningInitializeType(RandomContext);
  LightningInitializeType(CameraViewport);
  LightningInitializeType(DefaultGameSetup);
  LightningInitializeType(PathFinderBaseEvent);
  LightningInitializeTypeAs(PathFinderEvent<Vec3>, "PathFinderEvent");
  LightningInitializeTypeAs(PathFinderEvent<IntVec3>, "PathFinderGridEvent");
  LightningInitializeType(PathFinder);
  LightningInitializeType(PathFinderRequest);
  LightningInitializeType(PathFinderGrid);
  LightningInitializeType(PathFinderMesh);

  LightningInitializeType(SplineParticleEmitter);
  LightningInitializeType(SplineParticleAnimator);

  LightningInitializeType(UnitTestSystem);
  LightningInitializeType(UnitTestEvent);
  LightningInitializeType(UnitTestEndFrameEvent);
  LightningInitializeType(UnitTestBaseMouseEvent);
  LightningInitializeType(UnitTestMouseEvent);
  LightningInitializeType(UnitTestMouseDropEvent);
  LightningInitializeType(UnitTestKeyboardEvent);
  LightningInitializeType(UnitTestKeyboardTextEvent);
  LightningInitializeType(UnitTestWindowEvent);

  LightningInitializeType(IndexedHalfEdgeMeshVertexArray);
  LightningInitializeType(IndexedHalfEdgeMeshEdgeArray);
  LightningInitializeType(IndexedHalfEdgeFaceEdgeIndexArray);
  LightningInitializeType(IndexedHalfEdgeMeshFaceArray);
  LightningInitializeType(IndexedHalfEdge);
  LightningInitializeType(IndexedHalfEdgeFace);
  LightningInitializeType(IndexedHalfEdgeMesh);
  LightningInitializeTypeAs(QuickHull3DInterface, "QuickHull3D");

  LightningInitializeTypeAs(PlasmaStatic, "Plasma");

  // @trevor.sundberg: The Gameplay and Editor libraries are co-dependent
  LightningTypeId(Editor)->AssertOnInvalidBinding = &IgnoreOnInvalidBinding;

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void GameplayLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  InitializeResourceManager(TileMapSourceManager);
}

void GameplayLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Plasma
