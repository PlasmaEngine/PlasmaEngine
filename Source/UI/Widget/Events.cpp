// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(MouseEnterPreview);
DefineEvent(KeyPreview);
DefineEvent(HoverKeyPreview);
DefineEvent(HoverKeyDown);
DefineEvent(HoverKeyUp);
DefineEvent(HoverKeyRepeated);
DefineEvent(MouseFileDrop);
DefineEvent(MouseUpdate);
DefineEvent(MouseEnter);
DefineEvent(MouseExit);
DefineEvent(MouseEnterHierarchy);
DefineEvent(MouseExitHierarchy);
DefineEvent(MouseDown);
DefineEvent(MouseUp);
DefineEvent(LeftMouseDown);
DefineEvent(LeftMouseUp);
DefineEvent(RightMouseDown);
DefineEvent(RightMouseUp);
DefineEvent(MiddleMouseDown);
DefineEvent(MiddleMouseUp);
DefineEvent(MouseMove);
DefineEvent(MouseScroll);
DefineEvent(LeftMouseDrag);
DefineEvent(RightMouseDrag);
DefineEvent(MouseHold);
DefineEvent(MouseHover);
DefineEvent(LeftClick);
DefineEvent(RightClick);
DefineEvent(MiddleClick);
DefineEvent(DoubleClick);
DefineEvent(MouseDrop);
DefineEvent(FocusLost);
DefineEvent(FocusGained);
DefineEvent(FocusLostHierarchy);
DefineEvent(FocusGainedHierarchy);
DefineEvent(FocusReset);
DefineEvent(WidgetUpdate);
DefineEvent(Activated);
DefineEvent(Deactivated);
DefineEvent(WidgetShown);
DefineEvent(OnDestroy);
DefineEvent(Closing);
} // namespace Events

String NamedMouseDown[] = {Events::LeftMouseDown, Events::RightMouseDown, Events::MiddleMouseDown};
String NamedMouseUp[] = {Events::LeftMouseUp, Events::RightMouseUp, Events::MiddleMouseUp};
String NamedMouseClick[] = {Events::LeftClick, Events::RightClick, Events::MiddleClick};

LightningDefineType(FocusEvent, builder, type)
{
}

LightningDefineType(HandleableEvent, builder, type)
{
  LightningBindFieldProperty(Handled);
  PlasmaBindDocumented();
}

LightningDefineType(MouseDragEvent, builder, type)
{
}

bool MouseEvent::IsButtonDown(MouseButtons::Enum button)
{
  return mButtonDown[button] != 0;
}

MouseEvent::MouseEvent()
{
  Handled = false;
  Source = NULL;
  Button = MouseButtons::None;
  ButtonDown = false;
  Position = Vec2(0, 0);
  Movement = Vec2(0, 0);
  Scroll = Vec2(0, 0);
  ShiftPressed = false;
  AltPressed = false;
  Handled = false;
  HandledEventScript = false;
}

LightningDefineType(MouseEvent, builder, type)
{
  PlasmaBindDocumented();
  LightningBindFieldProperty(Button);
  LightningBindFieldProperty(ButtonDown);
  LightningBindFieldProperty(Position);
  LightningBindFieldProperty(Movement);
  LightningBindFieldProperty(Scroll);
  LightningBindFieldProperty(ShiftPressed);
  LightningBindFieldProperty(AltPressed);
  LightningBindFieldProperty(CtrlPressed);
  LightningBindGetterProperty(Mouse);
  LightningBindMethod(IsButtonDown);
  LightningBindMethod(IsButtonUp);

  LightningBindFieldPropertyAs(HandledEventScript, "HandledEvent");
}

LightningDefineType(MouseFileDropEvent, builder, type)
{
  LightningBindMember(Files);
}

MouseFileDropEvent::MouseFileDropEvent() : MouseEvent()
{
  Files = LightningAllocate(ArrayString);
}

MouseFileDropEvent::MouseFileDropEvent(const MouseEvent& rhs) : MouseEvent(rhs)
{
  Files = LightningAllocate(ArrayString);
}

void MouseFileDropEvent::Copy(const OsMouseDropEvent& rhs)
{
  Array<HandleOfString>& files = Files->NativeArray;
  files.Insert(files.Begin(), rhs.Files.Begin(), rhs.Files.End());
}

} // namespace Plasma
