// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Plasma
{

String GraphicsRaycastProviderToString(const BoundType* type, const ::byte* value)
{
  static String Name("Graphics");
  return Name;
}

LightningDefineType(GraphicsRaycastProvider, builder, type)
{
  LightningBindFieldProperty(mMultiSelectGraphical);
  LightningBindFieldProperty(mVisibleOnly);
  LightningBindConstructor();

  type->ToStringFunction = GraphicsRaycastProviderToString;
  type->Add(new StringNameDisplay("Graphics"));
  PlasmaBindExpanded();
}

GraphicsRaycastProvider::GraphicsRaycastProvider()
{
  mMultiSelectGraphical = true;
  mVisibleOnly = false;
}

void GraphicsRaycastProvider::RayCast(Ray& ray, CastInfo& castInfo, RaycastResultList& results)
{
  GraphicsSpace* graphicsSpace = castInfo.mTargetSpace->has(GraphicsSpace);

  GraphicsRayCast rayCast;
  rayCast.mRay = ray;
  rayCast.mObject = nullptr;
  rayCast.mPosition = Vec3(0.0f);
  rayCast.mNormal = Vec3(0.0f);
  rayCast.mUv = Vec2(0.0f);
  rayCast.mT = 0.0f;

  forRangeBroadphaseTree(GraphicsBroadPhase, graphicsSpace->mBroadPhase, Ray, ray)
  {
    if (RayTest(range.Front(), rayCast, castInfo))
      results.AddItem(
          rayCast.mObject, rayCast.mT, ray.Start + ray.Direction * rayCast.mT, rayCast.mNormal, rayCast.mUv, true);
  }

  forRange (Graphical& graphical, graphicsSpace->mGraphicalsNeverCulled.All())
  {
    if (RayTest(&graphical, rayCast, castInfo))
      results.AddItem(
          rayCast.mObject, rayCast.mT, ray.Start + ray.Direction * rayCast.mT, rayCast.mNormal, rayCast.mUv, true);
  }

  if (mVisibleOnly == false)
  {
    forRange (Graphical& graphical, graphicsSpace->mGraphicalsAlwaysCulled.All())
    {
      if (RayTest(&graphical, rayCast, castInfo))
        results.AddItem(
            rayCast.mObject, rayCast.mT, ray.Start + ray.Direction * rayCast.mT, rayCast.mNormal, rayCast.mUv, true);
    }
  }
}

void GraphicsRaycastProvider::FrustumCast(Frustum& frustum, CastInfo& castInfo, RaycastResultList& results)
{
  // Skip Graphical tests
  if (mMultiSelectGraphical == false)
    return;

  GraphicsSpace* graphicsSpace = castInfo.mTargetSpace->has(GraphicsSpace);

  forRangeBroadphaseTree(GraphicsBroadPhase, graphicsSpace->mBroadPhase, Frustum, frustum)
  {
    if (FrustumTest(range.Front(), frustum, castInfo))
      results.AddItem(range.Front()->GetOwner(), 0.0f, Vec3::cZero, Vec3::cZero);
  }

  forRange (Graphical& graphical, graphicsSpace->mGraphicalsNeverCulled.All())
  {
    if (FrustumTest(&graphical, frustum, castInfo))
      results.AddItem(graphical.GetOwner(), 0.0f, Vec3::cZero, Vec3::cZero);
  }

  if (mVisibleOnly == false)
  {
    forRange (Graphical& graphical, graphicsSpace->mGraphicalsAlwaysCulled.All())
    {
      if (FrustumTest(&graphical, frustum, castInfo))
        results.AddItem(graphical.GetOwner(), 0.0f, Vec3::cZero, Vec3::cZero);
    }
  }
}

bool GraphicsRaycastProvider::RayTest(Graphical* graphical, GraphicsRayCast& rayCast, CastInfo& castInfo)
{
  Cog* cog = graphical->GetOwner();
  if (cog->mFlags.IsSet(CogFlags::Locked))
    return false;

  if (cog->mFlags.IsSet(CogFlags::SelectionLimited) && cog->has(SelectionIcon) != graphical)
    return false;

  return graphical->TestRay(rayCast, castInfo);
}

bool GraphicsRaycastProvider::FrustumTest(Graphical* graphical, Frustum& frustum, CastInfo& castInfo)
{
  Cog* cog = graphical->GetOwner();
  if (cog->mFlags.IsSet(CogFlags::Locked))
    return false;

  if (cog->mFlags.IsSet(CogFlags::SelectionLimited) && cog->has(SelectionIcon) != graphical)
    return false;

  return graphical->TestFrustum(frustum, castInfo);
}

} // namespace Plasma
