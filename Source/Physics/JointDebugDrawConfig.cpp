// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(JointDebugDrawConfig, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindDocumented();
  PlasmaBindDependency(Joint);

  LightningBindGetterSetterProperty(Active)->PlasmaSerialize(true);
  LightningBindFieldProperty(mSize)->PlasmaSerialize(real(1.0));
  LightningBindFieldProperty(mDetail)->PlasmaSerialize(real(10.0));
  LightningBindGetterSetterProperty(ObjectAPerspective)->PlasmaSerialize(true);
  LightningBindGetterSetterProperty(ObjectBPerspective)->PlasmaSerialize(false);

  PlasmaBindTag(Tags::Physics);
  PlasmaBindTag(Tags::Joint);
}

JointDebugDrawConfig::JointDebugDrawConfig()
{
}

JointDebugDrawConfig::~JointDebugDrawConfig()
{
}

void JointDebugDrawConfig::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

bool JointDebugDrawConfig::GetActive() const
{
  return mPerspective.IsSet(JointDebugDrawConfigFlags::Active);
}

void JointDebugDrawConfig::SetActive(bool state)
{
  mPerspective.SetState(JointDebugDrawConfigFlags::Active, state);
}

bool JointDebugDrawConfig::GetObjectAPerspective() const
{
  return mPerspective.IsSet(JointDebugDrawConfigFlags::ObjectAPerspective);
}

void JointDebugDrawConfig::SetObjectAPerspective(bool state)
{
  mPerspective.SetState(JointDebugDrawConfigFlags::ObjectAPerspective, state);
}

bool JointDebugDrawConfig::GetObjectBPerspective() const
{
  return mPerspective.IsSet(JointDebugDrawConfigFlags::ObjectBPerspective);
}

void JointDebugDrawConfig::SetObjectBPerspective(bool state)
{
  mPerspective.SetState(JointDebugDrawConfigFlags::ObjectBPerspective, state);
}

} // namespace Plasma
