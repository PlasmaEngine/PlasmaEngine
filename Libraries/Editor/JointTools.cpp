// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(JointTool, builder, type)
{
  PlasmaBindComponent();
  LightningBindGetterSetterProperty(OverrideLength);
  LightningBindGetterSetterProperty(Length);
  LightningBindGetterSetterProperty(MaxImpulse);
  LightningBindGetterSetterProperty(UseCenter);
  LightningBindGetterSetterProperty(AutoSnaps);
  LightningBindGetterSetterProperty(AttachToWorld);
  LightningBindGetterSetterProperty(AttachToCommonParent);
  LightningBindFieldProperty(mJointType);
}

JointTool::JointTool()
{
  mJointType = JointToolTypes::StickJoint;
}

void JointTool::OnMouseEndDrag(Event* e)
{
  if (GetAttachToWorld())
    ObjectB = ObjectA;

  ObjectConnectingTool::OnMouseEndDrag(e);
}

void JointTool::DoConnection()
{
  if (Cog* a = ObjectA)
  {
    if (Cog* b = ObjectB)
    {
      Transform* t0 = a->has(Transform);
      Transform* t1 = b->has(Transform);
      if (t0 == nullptr || t1 == nullptr)
        return;

      Cog* jointCog = mJointCreator.CreateWorldPoints(a, b, GetJointName(), PointOnObjectA, PointOnObjectB);
      // Queue up an undo operation on this joint being created
      if (jointCog != nullptr)
      {
        OperationQueue* queue = PL::gEditor->GetOperationQueue();
        ObjectCreated(queue, jointCog);
      }
    }
  }
}

cstr JointTool::GetJointName()
{
  return JointToolTypes::Names[mJointType];
}

} // namespace Plasma
