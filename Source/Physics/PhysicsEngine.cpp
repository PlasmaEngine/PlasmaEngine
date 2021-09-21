#include "Precompiled.hpp"

namespace Plasma
{

System* CreatePhysicsSystem()
{
  return new PhysicsEngine();
}

//-------------------------------------------------------------------PhysicsEngine
LightningDefineType(PhysicsEngine, builder, type)
{
}

PhysicsEngine::PhysicsEngine(void)
{
  mHeap = new Memory::Heap("Physics", Memory::GetRoot());
}

PhysicsEngine::~PhysicsEngine(void)
{
  delete mCollisionManager;
}

cstr PhysicsEngine::GetName()
{
  return "Physics";
}

void PhysicsEngine::Initialize(SystemInitializer& initializer)
{
  ZoneScoped;
  // Allocate the collision manager.
  mCollisionManager = new Physics::CollisionManager();

  // Initialize broad phase static callbacks.
  IBroadPhase::SetCastRayCallBack(&Physics::CollisionManager::TestRayVsObject);
  IBroadPhase::SetCastSegmentCallBack(&Physics::CollisionManager::TestSegmentVsObject);
  IBroadPhase::SetCastAabbCallBack(&Physics::CollisionManager::TestAabbVsObject);
  IBroadPhase::SetCastSphereCallBack(&Physics::CollisionManager::TestSphereVsObject);
  IBroadPhase::SetCastFrustumCallBack(&Physics::CollisionManager::TestFrustumVsObject);
}

void PhysicsEngine::Update(bool debugger)
{
  if (debugger)
    return;

  ZoneScoped;
  ProfileScopeTree("PhysicsEngineUpdate", "Engine", Color::Yellow);

  // Update each physics space
  SpaceList::range r = mSpaces.All();
  while(!r.Empty())
  {
    PhysicsSpace& physicsSpace = r.Front();
    physicsSpace.FrameUpdate();
    r.PopFront();
  }
}

PhysicsEngine::SpaceList::range PhysicsEngine::GetSpaces()
{
  return mSpaces.All();
}

void PhysicsEngine::Publish()
{
  SpaceList::iterator it = mSpaces.Begin();
  for(; it != mSpaces.End(); ++it)
    it->Publish();
}

void PhysicsEngine::AddSpace(PhysicsSpace* space)
{
  mSpaces.PushBack(space);
}

void PhysicsEngine::RemoveSpace(PhysicsSpace* space)
{
  mSpaces.Erase(space);
}

}//namespace Plasma
