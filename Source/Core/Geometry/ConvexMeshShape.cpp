// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

ConvexMeshShape::ConvexMeshShape()
{
  mWorldAabb.Plasma();
}

ConvexMeshShape::ConvexMeshShape(const Intersection::SupportShape& support) : mSupport(support)
{
  mWorldAabb.Plasma();
}

void ConvexMeshShape::Support(Vec3Param direction, Vec3Ptr support) const
{
  mSupport.Support(direction, support);
}

void ConvexMeshShape::GetCenter(Vec3Ref center) const
{
  return mSupport.GetCenter(&center);
}

Obb ConvexMeshShape::Transform(Mat4Param transformation) const
{
  return mWorldAabb.Transform(transformation);
}

} // namespace Plasma
