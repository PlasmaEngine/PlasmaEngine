// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(AreaChanged);
}

LightningDefineType(AreaEvent, builder, type)
{
  PlasmaBindDocumented();

  LightningBindField(mArea);
}

static const float cAreaEpsilon = 0.001f;
// Thickness used for converting an Area to an Aabb
static const float cAreaThickness = 0.001f;

LightningDefineType(Area, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindDocumented();
  PlasmaBindSetup(SetupMode::CallSetDefaults);
  PlasmaBindEvent(Events::AreaChanged, AreaEvent);
  PlasmaBindDependency(Transform);

  LightningBindGetterSetterProperty(Origin)->PlasmaLocalModificationOverride();
  LightningBindGetterSetterProperty(Size)->PlasmaLocalModificationOverride();

  LightningBindGetterSetter(LocalRectangle);
  LightningBindGetterSetter(WorldRectangle);

  LightningBindMethod(GetLocalLocation);
  LightningBindMethod(SetLocalLocation);
  LightningBindMethod(GetWorldLocation);
  LightningBindMethod(SetWorldLocation);

  LightningBindGetter(TopLeft)->AddAttribute(DeprecatedAttribute);
  LightningBindGetter(TopCenter)->AddAttribute(DeprecatedAttribute);
  LightningBindGetter(TopRight)->AddAttribute(DeprecatedAttribute);
  LightningBindGetter(CenterLeft)->AddAttribute(DeprecatedAttribute);
  LightningBindGetter(Center)->AddAttribute(DeprecatedAttribute);
  LightningBindGetter(CenterRight)->AddAttribute(DeprecatedAttribute);
  LightningBindGetter(BottomLeft)->AddAttribute(DeprecatedAttribute);
  LightningBindGetter(BottomCenter)->AddAttribute(DeprecatedAttribute);
  LightningBindGetter(BottomRight)->AddAttribute(DeprecatedAttribute);

  LightningBindGetter(LocalTopLeft);
  LightningBindGetter(LocalTopCenter);
  LightningBindGetter(LocalTopRight);
  LightningBindGetter(LocalCenterLeft);
  LightningBindGetter(LocalCenter);
  LightningBindGetter(LocalCenterRight);
  LightningBindGetter(LocalBottomLeft);
  LightningBindGetter(LocalBottomCenter);
  LightningBindGetter(LocalBottomRight);

  LightningBindGetter(WorldTopLeft);
  LightningBindGetter(WorldTopCenter);
  LightningBindGetter(WorldTopRight);
  LightningBindGetter(WorldCenterLeft);
  LightningBindGetter(WorldCenter);
  LightningBindGetter(WorldCenterRight);
  LightningBindGetter(WorldBottomLeft);
  LightningBindGetter(WorldBottomCenter);
  LightningBindGetter(WorldBottomRight);

  LightningBindGetterSetter(LocalTop);
  LightningBindGetterSetter(WorldTop);
  LightningBindGetterSetter(LocalRight);
  LightningBindGetterSetter(WorldRight);
  LightningBindGetterSetter(LocalBottom);
  LightningBindGetterSetter(WorldBottom);
  LightningBindGetterSetter(LocalLeft);
  LightningBindGetterSetter(WorldLeft);

  LightningBindMethod(LocalOffsetOf);
}

void Area::SetDefaults()
{
  mSize = Vec2(1, 1);
  mOrigin = Location::Center;
}

void Area::Initialize(CogInitializer& initializer)
{
  mTransform = GetOwner()->has(Transform);
}

void Area::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSize, Vec2(1, 1));
  SerializeEnumName(Location, mOrigin);
}

Vec2 Area::GetSize()
{
  return mSize;
}

void Area::SetSize(Vec2 newSize)
{
  Math::Clamp(&newSize, 0.0f, 10000.0f);
  mSize = newSize;
  DoAreaChanged();
}

Location::Enum Area::GetOrigin()
{
  return mOrigin;
}

void Area::SetOrigin(Location::Enum origin)
{
  if (mOrigin != origin)
  {
    mOrigin = origin;
    DoAreaChanged();
  }
}

Vec2 Area::LocalOffsetOf(Location::Enum location)
{
  return OffsetOfOffset(location) * mSize;
}

Vec2 Area::OffsetOfOffset(Location::Enum location)
{
  return Location::GetDirection(mOrigin, location);
}

Aabb Area::GetLocalAabb()
{
  Vec3 offset = Vec3(-Location::GetDirection(mOrigin), 0);
  Vec3 size = Vec3(mSize, cAreaThickness);

  return Aabb(offset * size, size * 0.5f);
}

Aabb Area::GetAabb()
{
  Vec3 offset = Vec3(-Location::GetDirection(mOrigin), 0);
  Vec3 size = Vec3(mSize, cAreaThickness);

  return FromTransformAndExtents(mTransform, size * 0.5f, offset * size);
}

Vec2 Area::GetLocalTranslation()
{
  return ToVector2(mTransform->GetLocalTranslation());
}

void Area::SetLocalTranslation(Vec2Param translation)
{
  Vec3 localTranslation = mTransform->GetLocalTranslation();

  if (OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(mTransform, "Translation", localTranslation);

  localTranslation = Vec3(translation, localTranslation.z);
  mTransform->SetLocalTranslation(localTranslation);
}

Vec2 Area::GetWorldTranslation()
{
  Vec2 localTranslation = ToVector2(mTransform->GetLocalTranslation());

  if (Transform* parent = mTransform->TransformParent)
    localTranslation = ToVector2(parent->TransformPointInverse(Vec3(localTranslation)));

  return localTranslation;
}

void Area::SetWorldTranslation(Vec2Param worldTranslation)
{
  Vec2 localTranslation = worldTranslation;

  if (Transform* parent = mTransform->TransformParent)
    localTranslation = ToVector2(parent->TransformPointInverse(Vec3(worldTranslation)));

  SetLocalTranslation(localTranslation);
}

Rectangle Area::GetLocalRectangle()
{
  return Rectangle::CenterAndSize(GetLocalLocation(Location::Center), mSize);
}

void Area::SetLocalRectangle(RectangleParam rectangle)
{
  Vec2 size = rectangle.GetSize();
  Math::Clamp(&size, 0.0f, 10000.0f);
  mSize = size;

  SetLocalTranslation(rectangle.GetLocation(mOrigin));
  DoAreaChanged();
}

Rectangle Area::GetWorldRectangle()
{
  return Rectangle::CenterAndSize(GetWorldLocation(Location::Center), mSize);
}

void Area::SetWorldRectangle(RectangleParam rectangle)
{
  Vec2 size = rectangle.GetSize();
  Math::Clamp(&size, 0.0f, 10000.0f);
  mSize = size;

  SetWorldTranslation(rectangle.GetLocation(mOrigin));
  DoAreaChanged();
}

Vec2 Area::GetLocalLocation(Location::Enum location)
{
  Vec2 offset = Location::GetDirection(mOrigin, location);
  Vec2 local = offset * mSize;

  return ToVector2(mTransform->GetLocalTranslation()) + local;
}

void Area::SetLocalLocation(Location::Enum location, Vec2Param localTranslation)
{
  Vec2 offset = Location::GetDirection(mOrigin, location);
  Vec2 local = offset * mSize;

  SetLocalTranslation(localTranslation - local);
}

Vec2 Area::GetWorldLocation(Location::Enum location)
{
  Vec2 worldTranslation = GetLocalLocation(location);

  if (Transform* parent = mTransform->TransformParent)
    return ToVector2(parent->TransformPoint(Vec3(worldTranslation)));

  return worldTranslation;
}

void Area::SetWorldLocation(Location::Enum location, Vec2Param worldTranslation)
{
  Vec2 localTranslation = worldTranslation;

  if (Transform* parent = mTransform->TransformParent)
    localTranslation = ToVector2(parent->TransformPointInverse(Vec3(worldTranslation)));

  SetLocalLocation(location, localTranslation);
}

float Area::GetLocalTop()
{
  return GetLocalTopRight().y;
}

void Area::SetLocalTop(float localTop)
{
  Vec2 topRight = GetLocalTopRight();
  topRight.y = localTop;
  SetLocalTopRight(topRight);
}

float Area::GetWorldTop()
{
  return GetWorldTopRight().y;
}

void Area::SetWorldTop(float worldTop)
{
  Vec2 topRight = GetWorldTopRight();
  topRight.y = worldTop;
  SetWorldTopRight(topRight);
}

float Area::GetLocalRight()
{
  return GetLocalTopRight().x;
}

void Area::SetLocalRight(float localRight)
{
  Vec2 topRight = GetLocalTopRight();
  topRight.x = localRight;
  SetLocalTopRight(topRight);
}

float Area::GetWorldRight()
{
  return GetWorldTopRight().x;
}

void Area::SetWorldRight(float worldRight)
{
  Vec2 topRight = GetWorldTopRight();
  topRight.x = worldRight;
  SetWorldTopRight(topRight);
}

float Area::GetLocalBottom()
{
  return GetLocalBottomLeft().y;
}

void Area::SetLocalBottom(float localBottom)
{
  Vec2 bottomLeft = GetLocalBottomLeft();
  bottomLeft.y = localBottom;
  SetLocalBottomLeft(bottomLeft);
}

float Area::GetWorldBottom()
{
  return GetWorldBottomLeft().y;
}

void Area::SetWorldBottom(float worldBottom)
{
  Vec2 bottomLeft = GetWorldBottomLeft();
  bottomLeft.y = worldBottom;
  SetWorldBottomLeft(bottomLeft);
}

float Area::GetLocalLeft()
{
  return GetLocalBottomLeft().x;
}

void Area::SetLocalLeft(float localLeft)
{
  Vec2 bottomLeft = GetLocalBottomLeft();
  bottomLeft.x = localLeft;
  SetLocalBottomLeft(bottomLeft);
}

float Area::GetWorldLeft()
{
  return GetWorldBottomLeft().x;
}

void Area::SetWorldLeft(float worldLeft)
{
  Vec2 bottomLeft = GetWorldBottomLeft();
  bottomLeft.x = worldLeft;
  SetWorldBottomLeft(bottomLeft);
}

void Area::DoAreaChanged()
{
  TransformUpdateInfo info;
  info.TransformFlags = TransformUpdateFlags::Translation;
  GetOwner()->TransformUpdate(info);

  AreaEvent event;
  event.mArea = this;
  this->GetOwner()->DispatchEvent(Events::AreaChanged, &event);
}

void Area::DebugDraw()
{
}

} // namespace Plasma
