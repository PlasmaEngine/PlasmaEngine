// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void Reactive::Initialize(CogInitializer& initializer)
{
  HasOrAdd<ReactiveSpace>(GetSpace());
}

void Reactive::Serialize(Serializer& stream)
{
  SerializeName(mActive);
}

LightningDefineType(Reactive, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);
  PlasmaBindDocumented();

  LightningBindFieldProperty(mActive);

  PlasmaBindEvent(Events::MouseFileDrop, MouseFileDropEvent);

  PlasmaBindEvent(Events::MouseEnter, ViewportMouseEvent);
  PlasmaBindEvent(Events::MouseEnterPreview, ViewportMouseEvent);
  PlasmaBindEvent(Events::MouseExit, ViewportMouseEvent);

  PlasmaBindEvent(Events::MouseEnterHierarchy, ViewportMouseEvent);
  PlasmaBindEvent(Events::MouseExitHierarchy, ViewportMouseEvent);

  PlasmaBindEvent(Events::MouseMove, ViewportMouseEvent);
  PlasmaBindEvent(Events::MouseUpdate, ViewportMouseEvent);
  PlasmaBindEvent(Events::MouseScroll, ViewportMouseEvent);

  PlasmaBindEvent(Events::DoubleClick, ViewportMouseEvent);

  PlasmaBindEvent(Events::MouseDown, ViewportMouseEvent);
  PlasmaBindEvent(Events::MouseUp, ViewportMouseEvent);

  PlasmaBindEvent(Events::LeftMouseDown, ViewportMouseEvent);
  PlasmaBindEvent(Events::LeftMouseUp, ViewportMouseEvent);

  PlasmaBindEvent(Events::RightMouseDown, ViewportMouseEvent);
  PlasmaBindEvent(Events::RightMouseUp, ViewportMouseEvent);

  PlasmaBindEvent(Events::MiddleMouseDown, ViewportMouseEvent);
  PlasmaBindEvent(Events::MiddleMouseUp, ViewportMouseEvent);

  PlasmaBindEvent(Events::LeftClick, ViewportMouseEvent);
  PlasmaBindEvent(Events::RightClick, ViewportMouseEvent);
  PlasmaBindEvent(Events::MiddleClick, ViewportMouseEvent);

  PlasmaBindEvent(Events::MouseHold, ViewportMouseEvent);
  PlasmaBindEvent(Events::MouseHover, ViewportMouseEvent);
}

void Reactive::SetDefaults()
{
  mActive = true;
}

Reactive::Reactive()
{
}

Reactive::~Reactive()
{
}

LightningDefineType(ReactiveSpace, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindDocumented();

  PlasmaBindDependency(Space);

  LightningBindGetter(Over);
  LightningBindFieldProperty(mRaycaster);
}

void ReactiveSpace::Serialize(Serializer& stream)
{
  bool success = Serialization::Policy<Raycaster>::Serialize(stream, "Raycaster", mRaycaster);
  if (success == false)
  {
    mRaycaster.AddProvider(new PhysicsRaycastProvider());

    GraphicsRaycastProvider* graphicsRaycaster = new GraphicsRaycastProvider();
    graphicsRaycaster->mVisibleOnly = true;
    mRaycaster.AddProvider(graphicsRaycaster);
  }
}

Cog* ReactiveSpace::GetOver()
{
  return mOver;
}

} // namespace Plasma
