// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(JointConfigOverride, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindDocumented();
  PlasmaBindDependency(Joint);

  LightningBindGetterSetterProperty(Slop)->PlasmaSerialize(real(0.0));
  LightningBindGetterSetterProperty(LinearBaumgarte)->PlasmaSerialize(real(4.5));
  LightningBindGetterSetterProperty(AngularBaumgarte)->PlasmaSerialize(real(4.5));
  LightningBindGetterSetterProperty(LinearErrorCorrection)->PlasmaSerialize(real(0.2));
  LightningBindGetterSetterProperty(AngularErrorCorrection)->PlasmaSerialize(real(0.2));
  LightningBindGetterSetterProperty(PositionCorrectionType)->PlasmaSerialize(ConstraintPositionCorrection::Inherit);

  PlasmaBindTag(Tags::Physics);
  PlasmaBindTag(Tags::Joint);
}

JointConfigOverride::JointConfigOverride()
{
}

JointConfigOverride::~JointConfigOverride()
{
  if (mNode == nullptr)
    return;

  mNode->mConfigOverride = nullptr;
  mNode = nullptr;
}

void JointConfigOverride::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void JointConfigOverride::Initialize(CogInitializer& initializer)
{
  Joint* joint = GetOwner()->has(Joint);
  if (joint)
  {
    mNode = joint->mNode;
    mNode->mConfigOverride = this;
  }

  // If the joint is dynamically created, grab the default values from the
  // current config
  bool dynamicallyCreated = (initializer.Flags & CreationFlags::DynamicallyAdded) != 0;
  if (dynamicallyCreated)
  {
    PhysicsSolverConfig* config = joint->mSpace->GetPhysicsSolverConfig();
    ConstraintConfigBlock& configBlock = config->mJointBlocks[joint->GetJointType()];
    mSlop = configBlock.GetSlop();
    mLinearBaumgarte = configBlock.GetLinearBaumgarte();
    mAngularBaumgarte = configBlock.GetAngularBaumgarte();
    mLinearErrorCorrection = configBlock.GetLinearErrorCorrection();
    mAngularErrorCorrection = configBlock.GetAngularErrorCorrection();
  }
}

real JointConfigOverride::GetSlop() const
{
  return mSlop;
}

void JointConfigOverride::SetSlop(real slop)
{
  mSlop = slop;
}

real JointConfigOverride::GetLinearBaumgarte() const
{
  return mLinearBaumgarte;
}

void JointConfigOverride::SetLinearBaumgarte(real linearBaumgarte)
{
  mLinearBaumgarte = linearBaumgarte;
}

real JointConfigOverride::GetAngularBaumgarte() const
{
  return mAngularBaumgarte;
}

void JointConfigOverride::SetAngularBaumgarte(real angularBaumgarte)
{
  mAngularBaumgarte = angularBaumgarte;
}

real JointConfigOverride::GetLinearErrorCorrection()
{
  return mLinearErrorCorrection;
}

void JointConfigOverride::SetLinearErrorCorrection(real maxError)
{
  if (maxError < 0)
  {
    DoNotifyWarning("Invalid Value", "LinearErrorCorrection must be positive");
    maxError = 0;
  }

  mLinearErrorCorrection = maxError;
}

real JointConfigOverride::GetAngularErrorCorrection()
{
  return mAngularErrorCorrection;
}

void JointConfigOverride::SetAngularErrorCorrection(real maxError)
{
  if (maxError < 0)
  {
    DoNotifyWarning("Invalid Value", "AngularErrorCorrection must be positive");
    maxError = 0;
  }

  mAngularErrorCorrection = maxError;
}

ConstraintPositionCorrection::Enum JointConfigOverride::GetPositionCorrectionType() const
{
  return mPositionCorrectionType;
}

void JointConfigOverride::SetPositionCorrectionType(ConstraintPositionCorrection::Enum correctionType)
{
  mPositionCorrectionType = correctionType;
}

} // namespace Plasma
