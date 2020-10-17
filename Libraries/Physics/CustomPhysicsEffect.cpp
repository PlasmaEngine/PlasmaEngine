// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(CustomPhysicsEffectPrecalculatePhase);
DefineEvent(ApplyCustomPhysicsEffect);
} // namespace Events

LightningDefineType(CustomPhysicsEffectEvent, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindDocumented();

  LightningBindFieldProperty(mDt);
  LightningBindFieldProperty(mEffect);
  LightningBindFieldProperty(mRigidBody);
}

CustomPhysicsEffectEvent::CustomPhysicsEffectEvent()
{
  mRigidBody = nullptr;
  mEffect = nullptr;
  mDt = 0;
}

LightningDefineType(CustomPhysicsEffect, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindDocumented();

  PlasmaBindEvent(Events::CustomPhysicsEffectPrecalculatePhase, CustomPhysicsEffectEvent);
  PlasmaBindEvent(Events::ApplyCustomPhysicsEffect, CustomPhysicsEffectEvent);
}

CustomPhysicsEffect::CustomPhysicsEffect()
{
  mEffectType = PhysicsEffectType::Custom;
}

void CustomPhysicsEffect::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void CustomPhysicsEffect::PreCalculate(real dt)
{
  if (!GetActive())
    return;

  CustomPhysicsEffectEvent toSend;
  toSend.mEffect = this;
  toSend.mRigidBody = nullptr;
  toSend.mDt = dt;
  GetOwner()->DispatchEvent(Events::CustomPhysicsEffectPrecalculatePhase, &toSend);
}

void CustomPhysicsEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if (!GetActive())
    return;

  CustomPhysicsEffectEvent toSend;
  toSend.mEffect = this;
  toSend.mRigidBody = obj;
  toSend.mDt = dt;
  GetOwner()->DispatchEvent(Events::ApplyCustomPhysicsEffect, &toSend);
}

} // namespace Plasma
