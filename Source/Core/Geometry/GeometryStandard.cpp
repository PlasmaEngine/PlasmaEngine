// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineExternalBaseType(Ray, TypeCopyMode::ValueType, builder, type)
{
  LightningBindDefaultCopyDestructor();
  LightningFullBindConstructor(builder, type, LightningSelf, "start, direction", Vec3Param, Vec3Param);

  LightningBindMemberProperty(Start);
  LightningBindMemberProperty(Direction);
  LightningFullBindMethod(builder, type, &Ray::GetPoint, LightningNoOverload, "GetPoint", "tValue")->Description =
      LightningDocumentString("Returns the point at the given t-value.");
  LightningFullBindMethod(builder, type, &Ray::GetTValue, LightningNoOverload, "GetTValue", "point")->Description =
      LightningDocumentString("Returns the t-value that would result in the given "
                          "point projected onto the ray.");
  type->ToStringFunction = Lightning::BoundTypeToGlobalToString<Ray>;
  type->AddAttribute(ExportDocumentation);
}

LightningDefineExternalBaseType(Segment, TypeCopyMode::ValueType, builder, type)
{
  LightningBindDefaultCopyDestructor();
  LightningFullBindConstructor(builder, type, LightningSelf, "start, end", Vec3Param, Vec3Param);

  LightningBindMemberProperty(Start);
  LightningBindMemberProperty(End);
  LightningFullBindMethod(builder, type, &Segment::GetPoint, LightningNoOverload, "GetPoint", "tValue")->Description =
      LightningDocumentString("Returns the point at the given t-value.");
  LightningFullBindMethod(builder, type, &Segment::GetTValue, LightningNoOverload, "GetTValue", "point")->Description =
      LightningDocumentString("Returns the t-value that would result in the given "
                          "point projected onto the segment.");
  type->ToStringFunction = Lightning::BoundTypeToGlobalToString<Segment>;
  type->AddAttribute(ExportDocumentation);
}

LightningDefineExternalBaseType(Aabb, TypeCopyMode::ValueType, builder, type)
{
  LightningBindDefaultCopyDestructor();
  LightningFullBindConstructor(builder, type, LightningSelf, "center, halfExtents", Vec3Param, Vec3Param);

  LightningBindMemberProperty(mMin);
  LightningBindMemberProperty(mMax);

  LightningBindOverloadedMethodAs(Expand, LightningStaticOverload(Aabb, const Aabb&, Vec3Param), "Expand")->Description =
      LightningDocumentString("Creates an aabb that contains the given aabb and point.");
  LightningBindOverloadedMethodAs(Combine, LightningStaticOverload(Aabb, const Aabb&, const Aabb&), "Expand")->Description =
      LightningDocumentString("Creates an aabb that contains the two given aabbs.");
  LightningBindOverloadedMethodAs(Expand, LightningInstanceOverload(void, Vec3Param), "Expand")->Description =
      LightningDocumentString("Expand this aabb to contain the given point.");
  LightningBindOverloadedMethodAs(Combine, LightningInstanceOverload(void, const Aabb&), "Expand")->Description =
      LightningDocumentString("Expand this aabb to contain the given aabb.");
  LightningBindMethod(ContainsPoint)->Description = LightningDocumentString("Does this aabb contain the given point?");
  LightningBindMethodAs(Overlap, "Overlaps")->Description =
      LightningDocumentString("Does this aabb overlap/intersect the given aabb?");
  LightningFullBindMethod(builder, type, &Aabb::Set, LightningNoOverload, "Set", "point");
  LightningFullBindMethod(builder, type, &Aabb::SetCenterAndHalfExtents, LightningNoOverload, "Set", "center, halfExtents");
  LightningBindMethod(SetInvalid)->Description = LightningDocumentString("Sets this aabb to an invalid aabb "
                                                                 "(Real3.PositiveMax, Real3.NegativeMin)). "
                                                                 "This also makes expansion easier.");

  LightningBindGetterSetter(Extents);
  LightningBindGetterSetter(HalfExtents);
  LightningFullBindGetterSetter(builder,
                            type,
                            &Aabb::GetCenter,
                            (Vec3(Aabb::*)() const),
                            &Aabb::SetCenter,
                            (void (Aabb::*)(Vec3Param)),
                            "Center");

  // Expose later when we do a more full pass on the math library and geometry
  // LightningFullBindMethod(builder, type, &Aabb::TransformAabb,
  // LightningConstInstanceOverload(Aabb, Mat4Param), "Transform", "transform")
  //  ->Description = LightningDocumentString("Computes the aabb of the current aabb
  //  after applying the given transformation.");

  LightningBindGetter(Volume);
  LightningBindGetter(SurfaceArea);

  LightningBindMethodAs(Plasma, "ZeroOut");
  type->ToStringFunction = Lightning::BoundTypeToGlobalToString<Aabb>;
  type->AddAttribute(ExportDocumentation);

  // Deprecated fns
  Lightning::Function* overlapFn = LightningBindMethodAs(Overlap, "Overlap");
  overlapFn->Description = "This function is deprecated. Use Overlaps instead";
  overlapFn->AddAttribute(DeprecatedAttribute);
}

LightningDefineExternalBaseType(Sphere, TypeCopyMode::ValueType, builder, type)
{
  LightningBindDefaultCopyDestructor();
  LightningFullBindConstructor(builder, type, LightningSelf, "center, radius", Vec3Param, real);

  LightningBindMemberProperty(mCenter);
  LightningBindMemberProperty(mRadius);
  LightningBindOverloadedMethodAs(Expand, LightningStaticOverload(Sphere, const Sphere&, Vec3Param), "Expand")->Description =
      LightningDocumentString("Creates a sphere that contains the given sphere and point.");
  LightningBindOverloadedMethodAs(Expand, LightningInstanceOverload(void, Vec3Param), "Expand")->Description =
      LightningDocumentString("Expand this sphere to contain the given point.");
  LightningBindMethodAs(Overlap, "Overlaps")->Description =
      LightningDocumentString("Does this sphere overlap/intersect the given sphere?");
  LightningBindGetter(Volume);
  LightningBindGetter(SurfaceArea);
  type->ToStringFunction = Lightning::BoundTypeToGlobalToString<Sphere>;
  type->AddAttribute(ExportDocumentation);

  // Deprecated fns
  Lightning::Function* overlapFn = LightningBindMethodAs(Overlap, "Overlap");
  overlapFn->Description = "This function is deprecated. Use Overlaps instead";
  overlapFn->AddAttribute(DeprecatedAttribute);
}

LightningDefineExternalBaseType(Plane, TypeCopyMode::ValueType, builder, type)
{
  LightningBindDefaultCopyDestructor();
  LightningFullBindConstructor(builder, type, LightningSelf, "data", Vec4Param);
  LightningFullBindConstructor(builder, type, LightningSelf, "normal point", Vec3Param, Vec3Param);
  LightningFullBindMethod(builder, type, &LightningSelf::Set, LightningInstanceOverload(void, Vec4Param), "Set", "data");
  LightningFullBindMethod(
      builder, type, &LightningSelf::Set, LightningInstanceOverload(void, Vec3Param, Vec3Param), "Set", "normal point");
  LightningBindGetter(Normal);
  LightningBindGetter(Distance);
  LightningBindMemberProperty(mData);
  type->ToStringFunction = Lightning::BoundTypeToGlobalToString<Plane>;
  type->AddAttribute(ExportDocumentation);
}

LightningDefineExternalBaseType(Frustum, TypeCopyMode::ValueType, builder, type)
{
  //@JoshD: Make the bound data actually useful (also fix ToString)
  LightningBindDefaultCopyDestructor();
  LightningBindMethod(Get);
  LightningBindOverloadedMethod(Set, LightningInstanceOverload(void, uint, const Plane&));
  LightningBindMethod(GetAabb);
  type->ToStringFunction = Lightning::BoundTypeToGlobalToString<Frustum>;
  type->AddAttribute(ExportDocumentation);
}

LightningDefineStaticLibrary(GeometryLibrary)
{
  LightningInitializeExternalType(Ray);
  LightningInitializeExternalType(Segment);
  LightningInitializeExternalType(Aabb);
  LightningInitializeExternalType(Sphere);
  LightningInitializeExternalType(Plane);
  LightningInitializeExternalType(Frustum);

#define PlasmaDebugPrimitive(X) LightningInitializeTypeAs(Debug::X, "Debug" #X);
#include "Core/Geometry/DebugPrimitives.inl"
#undef PlasmaDebugPrimitive
}

void GeometryLibrary::Initialize()
{
  BuildStaticLibrary();
}

void GeometryLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Plasma
