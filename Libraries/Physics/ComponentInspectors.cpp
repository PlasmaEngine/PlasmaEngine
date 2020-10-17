// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(ColliderInspector, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindDependency(Collider);
  PlasmaBindSetup(SetupMode::DefaultSerialization);

  LightningBindMethod(GetColliderLocalCenterOfMass);
  LightningBindMethod(ComputeMass);
  LightningBindMethod(GetLocalInverseInertiaTensorRow0);
  LightningBindMethod(GetLocalInverseInertiaTensorRow1);
  LightningBindMethod(GetLocalInverseInertiaTensorRow2);
  type->AddAttribute(::Plasma::ObjectAttributes::cDoNotDocument);
}

Vec3 ColliderInspector::GetColliderLocalCenterOfMass() const
{
  return GetOwner()->has(Collider)->GetColliderLocalCenterOfMass();
}

real ColliderInspector::ComputeMass() const
{
  return GetOwner()->has(Collider)->ComputeMass();
}

Vec3 ColliderInspector::GetLocalInverseInertiaTensorRow0() const
{
  return GetInertia()[0];
}

Vec3 ColliderInspector::GetLocalInverseInertiaTensorRow1() const
{
  return GetInertia()[1];
}

Vec3 ColliderInspector::GetLocalInverseInertiaTensorRow2() const
{
  return GetInertia()[2];
}

Mat3 ColliderInspector::GetInertia() const
{
  Collider* collider = GetOwner()->has(Collider);
  real mass = ComputeMass();
  Mat3 inertia;
  collider->ComputeLocalInverseInertiaTensor(mass, inertia);
  return inertia;
}

} // namespace Plasma
