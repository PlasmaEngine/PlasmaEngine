// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(JointExceedImpulseLimit);
DefineEvent(JointLowerLimitReached);
DefineEvent(JointUpperLimitReached);
} // namespace Events

LightningDefineType(JointEvent, builder, type)
{
  PlasmaBindDocumented();
  LightningBindGetterProperty(ObjectA);
  LightningBindGetterProperty(ObjectB);
  LightningBindGetterProperty(JointCog);
  LightningBindGetterProperty(Joint);

  PlasmaBindTag(Tags::Physics);
}

JointEvent::JointEvent()
{
  mColliderA = nullptr;
  mColliderB = nullptr;
  mJoint = nullptr;
}

Cog* JointEvent::GetObjectA()
{
  return mColliderA->GetOwner();
}

Cog* JointEvent::GetObjectB()
{
  return mColliderB->GetOwner();
}

Cog* JointEvent::GetJointCog()
{
  return mJoint->GetOwner();
}

Joint* JointEvent::GetJoint()
{
  return mJoint;
}

} // namespace Plasma
