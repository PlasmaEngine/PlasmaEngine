#include "Precompiled.hpp"

namespace Plasma
{
  
LightningDefineType(ThrustEffect, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindDocumented();
  
  LightningBindGetterSetterProperty(LocalSpaceDirection)->PlasmaSerialize(true);
  LightningBindGetterSetterProperty(ForceStrength)->PlasmaSerialize(real(10));
  LightningBindGetterSetterProperty(ForceDirection)->PlasmaSerialize(Vec3(0, 1, 0));
  LightningBindGetterProperty(WorldForceDirection);
}

ThrustEffect::ThrustEffect()
{
  mEffectType = PhysicsEffectType::Thrust;
}

void ThrustEffect::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void ThrustEffect::DebugDraw()
{
  if(!GetDebugDrawEffect())
    return;

  PreCalculate(0);
  Vec3 pos = mWorldThrustCenter;
  Vec3 dir = mWorldThrustDirection * mForceStrength;
  gDebugDraw->Add(Debug::Line(pos, pos + dir).HeadSize(0.1f));
}

void ThrustEffect::PreCalculate(real dt)
{
  if(!GetActive())
    return;

  // Update the cached center and direction
  mWorldThrustCenter = TransformLocalPointToWorld(Vec3::cZero);
  mWorldThrustDirection = GetWorldForceDirection();
}

void ThrustEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if(!GetActive())
    return;

  ZoneScoped;

  // Apply the force to the rigid body at our thrust center
  Vec3 force = mWorldThrustDirection * mForceStrength;
  obj->ApplyForceAtPointNoWakeUp(force, mWorldThrustCenter);
}

bool ThrustEffect::GetLocalSpaceDirection() const
{
  return mThrustFlags.IsSet(ThrustFlags::LocalSpaceDirection);
}

void ThrustEffect::SetLocalSpaceDirection(bool state)
{
  mThrustFlags.SetState(ThrustFlags::LocalSpaceDirection,state);
  CheckWakeUp();
}

real ThrustEffect::GetForceStrength() const
{
  return mForceStrength;
}

void ThrustEffect::SetForceStrength(real strength)
{
  mForceStrength = strength;
  CheckWakeUp();
}

Vec3 ThrustEffect::GetForceDirection() const
{
  return mForceDirection;
}

void ThrustEffect::SetForceDirection(Vec3Param force)
{
  mForceDirection = force;
  CheckWakeUp();
}

Vec3 ThrustEffect::GetWorldForceDirection() const
{
  Vec3 worldThrustDirection = mForceDirection;
  if(mThrustFlags.IsSet(ThrustFlags::LocalSpaceDirection))
    worldThrustDirection = TransformLocalDirectionToWorld(worldThrustDirection);
  // Always re-normalize the world axis
  return worldThrustDirection.AttemptNormalized();
}

}//namespace Plasma
