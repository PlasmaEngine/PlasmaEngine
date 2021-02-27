// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// Ranges
LightningDefineRange(CastResultsRange);
LightningDefineRange(ContactPointRange);
LightningDefineRange(SweepResultRange);
LightningDefineRange(CastResults::range);
LightningDefineRange(ContactRange);
LightningDefineRange(JointRange);
LightningDefineRange(PhysicsMeshVertexData::RangeType);
LightningDefineRange(PhysicsMeshIndexData::RangeType);
LightningDefineRange(MultiConvexMeshVertexData::RangeType);
LightningDefineRange(MultiConvexMeshIndexData::RangeType);
LightningDefineRange(MultiConvexMeshSubMeshData::RangeType);

// Enums
LightningDefineEnum(RigidBodyDynamicState);
LightningDefineEnum(CastFilterState);
LightningDefineEnum(PhysicsEffectType);
LightningDefineEnum(PhysicsSolverPositionCorrection);
LightningDefineEnum(ConstraintPositionCorrection);
LightningDefineEnum(PhysicsSolverType);
LightningDefineEnum(PhysicsSolverSubType);
LightningDefineEnum(PhysicsIslandType);
LightningDefineEnum(PhysicsIslandPreProcessingMode);
LightningDefineEnum(PhysicsContactTangentTypes);
LightningDefineEnum(JointFrameOfReference);
LightningDefineEnum(AxisDirection);
LightningDefineEnum(PhysicsEffectInterpolationType);
LightningDefineEnum(PhysicsEffectEndCondition);
LightningDefineEnum(Mode2DStates);
LightningDefineEnum(CapsuleScalingMode);
LightningDefineEnum(CollisionFilterCollisionFlags);
LightningDefineEnum(CollisionFilterBlockType);
LightningDefineEnum(SpringDebugDrawMode);
LightningDefineEnum(SpringDebugDrawType);
LightningDefineEnum(SpringSortOrder);

// Bind the joint types special because they're generated using the #define
// #include trick
LightningDefineExternalBaseType(JointTypes::Enum, TypeCopyMode::ValueType, builder, type)
{
  LightningFullBindEnum(builder, type, SpecialType::Enumeration);
  // Add all of the joint types
  for (size_t i = 0; i < JointTypes::Size; ++i)
  {
    LightningFullBindEnumValue(builder, type, i, JointTypes::Names[i]);
  }
}

LightningDefineStaticLibrary(PhysicsLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  LightningInitializeRangeAs(ContactPointRange, "ContactPointRange");
  LightningInitializeRange(ContactRange);
  LightningInitializeRange(JointRange);
  LightningInitializeRange(CastResultsRange);
  LightningInitializeRangeAs(CastResults::range, "CastResultsArrayRange");
  LightningInitializeRange(SweepResultRange);
  LightningInitializeRangeAs(PhysicsMeshVertexData::RangeType, "PhysicsMeshVertexRange");
  LightningInitializeRangeAs(PhysicsMeshIndexData::RangeType, "PhysicsMeshIndexRange");

  // Enums
  LightningInitializeEnum(RigidBodyDynamicState);
  LightningInitializeEnum(CastFilterState);
  LightningInitializeEnum(PhysicsEffectType);
  LightningInitializeEnum(PhysicsSolverPositionCorrection);
  LightningInitializeEnum(ConstraintPositionCorrection);
  LightningInitializeEnum(PhysicsSolverType);
  LightningInitializeEnum(PhysicsSolverSubType);
  LightningInitializeEnum(PhysicsIslandType);
  LightningInitializeEnum(PhysicsIslandPreProcessingMode);
  LightningInitializeEnum(PhysicsContactTangentTypes);
  LightningInitializeEnum(JointFrameOfReference);
  LightningInitializeEnum(AxisDirection);
  LightningInitializeEnum(PhysicsEffectInterpolationType);
  LightningInitializeEnum(PhysicsEffectEndCondition);
  LightningInitializeEnum(Mode2DStates);
  LightningInitializeEnum(CapsuleScalingMode);
  LightningInitializeEnum(CollisionFilterCollisionFlags);
  LightningInitializeEnum(CollisionFilterBlockType);
  LightningInitializeEnum(SpringDebugDrawMode);
  LightningInitializeEnum(SpringDebugDrawType);
  LightningInitializeEnum(SpringSortOrder);
  LightningInitializeEnum(JointTypes);
  LightningInitializeEnum(SpringDebugDrawMode);
  LightningInitializeEnum(SpringDebugDrawType);
  LightningInitializeEnum(SpringSortOrder);

  // Meta Components
  LightningInitializeType(CollisionFilterMetaComposition);
  LightningInitializeType(PhysicsSolverConfigMetaComposition);
  // Events
  LightningInitializeType(BaseCollisionEvent);
  LightningInitializeType(CollisionEvent);
  LightningInitializeType(CollisionGroupEvent);
  LightningInitializeType(CustomJointEvent);
  LightningInitializeType(JointEvent);
  LightningInitializeType(CustomPhysicsEffectEvent);
  LightningInitializeType(CastFilterEvent);
  LightningInitializeType(PreSolveEvent);

  LightningInitializeType(PhysicsEngine);
  LightningInitializeType(PhysicsSpace);
  LightningInitializeType(RigidBody);
  LightningInitializeType(SpringSystem);
  LightningInitializeType(DecorativeCloth);
  LightningInitializeType(DecorativeRope);
  LightningInitializeType(Region);
  LightningInitializeType(MassOverride);

  // Colliders
  LightningInitializeType(Collider);
  LightningInitializeType(BoxCollider);
  LightningInitializeType(CapsuleCollider);
  LightningInitializeType(ConvexMeshCollider);
  LightningInitializeType(CylinderCollider);
  LightningInitializeType(EllipsoidCollider);
  LightningInitializeType(HeightMapCollider);
  LightningInitializeType(MeshCollider);
  LightningInitializeType(MultiConvexMeshCollider);
  LightningInitializeType(SphereCollider);

  // PhysicsEffects
  LightningInitializeType(PhysicsEffect);
  LightningInitializeType(BasicDirectionEffect);
  LightningInitializeType(ForceEffect);
  LightningInitializeType(GravityEffect);
  LightningInitializeType(BasicPointEffect);
  LightningInitializeType(PointForceEffect);
  LightningInitializeType(PointGravityEffect);
  LightningInitializeType(BuoyancyEffect);
  LightningInitializeType(DragEffect);
  LightningInitializeType(FlowEffect);
  LightningInitializeType(IgnoreSpaceEffects);
  LightningInitializeType(ThrustEffect);
  LightningInitializeType(TorqueEffect);
  LightningInitializeType(VortexEffect);
  LightningInitializeType(WindEffect);
  LightningInitializeType(CustomPhysicsEffect);

  // Joints
  LightningInitializeType(CustomConstraintInfo);
  LightningInitializeType(JointSpring);
  LightningInitializeType(JointLimit);
  LightningInitializeType(JointMotor);
  LightningInitializeType(JointDebugDrawConfig);
  LightningInitializeType(JointConfigOverride);
  LightningInitializeType(Joint);
  LightningInitializeType(CustomJoint);
  LightningInitializeType(ConstraintConfigBlock);
  // Joints and JointBlocks
#define JointType(jointType)                                                                                           \
  LightningInitializeType(jointType);                                                                                      \
  LightningInitializeType(jointType##Block);
#include "JointList.hpp"
#undef JointType
  LightningInitializeType(ContactBlock);

  // FilterBlocks
  LightningInitializeType(CollisionFilterBlock);
  LightningInitializeType(CollisionStartBlock);
  LightningInitializeType(CollisionPersistedBlock);
  LightningInitializeType(CollisionEndBlock);
  LightningInitializeType(PreSolveBlock);

  // Resources
  LightningInitializeType(PhysicsMaterial);
  LightningInitializeType(GenericPhysicsMesh);
  LightningInitializeType(ConvexMesh);
  LightningInitializeType(MultiConvexMesh);
  LightningInitializeType(PhysicsMesh);
  LightningInitializeType(CollisionFilter);
  LightningInitializeType(CollisionGroup);
  LightningInitializeType(CollisionTable);
  LightningInitializeType(PhysicsSolverConfig);
  // Resource Helpers
  LightningInitializeType(PhysicsMeshVertexData);
  LightningInitializeType(PhysicsMeshIndexData);
  LightningInitializeType(MultiConvexMeshVertexData);
  LightningInitializeType(MultiConvexMeshIndexData);
  LightningInitializeType(SubConvexMesh);
  LightningInitializeType(MultiConvexMeshSubMeshData);
  LightningInitializeRangeAs(MultiConvexMeshVertexData::RangeType, "MultiConvexMeshVertexRange");
  LightningInitializeRangeAs(MultiConvexMeshIndexData::RangeType, "MultiConvexMeshIndexRange");
  LightningInitializeRangeAs(MultiConvexMeshSubMeshData::RangeType, "MultiConvexMeshSubMeshRange");

  // Casting
  LightningInitializeType(BaseCastFilter);
  LightningInitializeType(CastFilter);
  LightningInitializeType(CastResult);
  LightningInitializeType(CastResults);
  LightningInitializeType(SweepResult);

  // Misc
  LightningInitializeType(PhysicsCar);
  LightningInitializeTypeAs(PhysicsCar::CarWheelRef, "CarWheelRef");
  LightningInitializeTypeAs(PhysicsCar::CarWheelArray, "CarWheelArray");
  LightningInitializeType(PhysicsCarWheel);
  LightningInitializeType(CustomCollisionEventTracker);

  LightningInitializeType(JointCreator);
  LightningInitializeType(DynamicMotor);
  LightningInitializeType(PhysicsRaycastProvider);
  LightningInitializeTypeAs(ContactPoint, "ContactPoint");
  LightningInitializeType(ContactGraphEdge);
  LightningInitializeType(JointGraphEdge);

  // Not ready for consumption yet, but I want to test it with dev config
  // METAREFACTOR they should always be initialized, but hidden with an
  // attribute
  // LightningInitializeType(SpringSystem);
  // LightningInitializeType(DecorativeCloth);
  // LightningInitializeType(DecorativeRope);
  // LightningInitializeType(GjkDebug);
  // LightningInitializeType(MeshDebug);
  // LightningInitializeType(TimeOfImpactDebug);
  // LightningInitializeType(ColliderInspector);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void PhysicsLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  // Create the material manager.
  InitializeResourceManager(PhysicsMaterialManager);
  InitializeResourceManager(PhysicsSolverConfigManager);
  InitializeResourceManager(PhysicsMeshManager);
  InitializeResourceManager(CollisionGroupManager);
  InitializeResourceManager(CollisionTableManager);
  InitializeResourceManager(ConvexMeshManager);
  InitializeResourceManager(MultiConvexMeshManager);
}

void PhysicsLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Plasma
