// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(JointSpring, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindDocumented();
  PlasmaBindDependency(Joint);

  LightningBindGetterSetterProperty(Active)->PlasmaSerialize(true);
  LightningBindGetterSetterProperty(FrequencyHz)->PlasmaSerialize(real(2.0));
  LightningBindGetterSetterProperty(DampingRatio)->PlasmaSerialize(real(0.7));

  LightningBindGetterSetter(AtomIds)->PlasmaSerialize(255u);

  PlasmaBindTag(Tags::Physics);
  PlasmaBindTag(Tags::Joint);
}

JointSpring::JointSpring()
{
  mNode = nullptr;
  mActive = true;
  mSpringAtom.mDampingRatio = real(0.0);
  mSpringAtom.mFrequencyHz = real(2.0);
  mAtomIds = static_cast<uint>(-1);
}

JointSpring::~JointSpring()
{
  if (mNode == nullptr)
    return;

  mNode->mSpring = nullptr;
  mNode = nullptr;
}

void JointSpring::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void JointSpring::Initialize(CogInitializer& initializer)
{
  Joint* joint = GetOwner()->has(Joint);
  if (joint)
  {
    mNode = joint->mNode;
    mNode->mSpring = this;
  }
}

bool JointSpring::GetAtomIndexActive(uint atomIndexMask)
{
  return GetActive() != false && (mAtomIds & atomIndexMask) != 0;
}

bool JointSpring::IsValid()
{
  return mNode != nullptr;
}

bool JointSpring::GetActive() const
{
  return mActive;
}

void JointSpring::SetActive(bool active)
{
  mActive = active;
  if (!IsValid())
    return;

  Physics::JointHelpers::ForceAwakeJoint(mNode->mJoint);
}

uint JointSpring::GetAtomIds() const
{
  return mAtomIds;
}

void JointSpring::SetAtomIds(uint atomIds)
{
  mAtomIds = atomIds;
  if (!IsValid())
    return;

  Physics::JointHelpers::ForceAwakeJoint(mNode->mJoint);
}

real JointSpring::GetFrequencyHz() const
{
  return mSpringAtom.mFrequencyHz;
}

void JointSpring::SetFrequencyHz(real frequency)
{
  mSpringAtom.mFrequencyHz = frequency;
  if (!IsValid())
    return;

  Physics::JointHelpers::ForceAwakeJoint(mNode->mJoint);
}

real JointSpring::GetDampingRatio() const
{
  return mSpringAtom.mDampingRatio;
}

void JointSpring::SetDampingRatio(real dampRatio)
{
  mSpringAtom.mDampingRatio = dampRatio;
  if (!IsValid())
    return;

  Physics::JointHelpers::ForceAwakeJoint(mNode->mJoint);
}

} // namespace Plasma
