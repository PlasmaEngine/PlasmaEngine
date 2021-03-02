// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace PL
{
Mouse* gMouse = nullptr;
}

Mouse::Mouse()
{
  mPlatform = PL::gEngine->has(OsShell);
  mCurrentCursor = Cursor::Arrow;
  mCursorMovement = Vec2::cZero;
  mClientPosition = Vec2::cZero;
  mRawMovement = Vec2::cZero;
  mActiveWindow = nullptr;
  for (uint i = 0; i < MouseButtons::Size; ++i)
    mButtonDown[i] = false;
  PL::gMouse = this;
}

LightningDefineType(Mouse, builder, type)
{
  type->HandleManager = LightningManagerId(PointerManager);

  PlasmaBindDocumented();
  LightningBindGetterSetterProperty(Cursor);
  LightningBindMethod(IsButtonDown);

  LightningBindGetterProperty(ClientPosition);
  LightningBindGetterProperty(CursorMovement);
  LightningBindGetterSetterProperty(Trapped);
  LightningBindMethod(ToggleTrapped);

  LightningBindFieldProperty(mRawMovement);
}

bool Mouse::IsButtonDown(MouseButtons::Enum button)
{
  return mButtonDown[button] != 0;
}

Cursor::Enum Mouse::GetCursor()
{
  return mCurrentCursor;
}

void Mouse::SetCursor(Cursor::Enum cursor)
{
  PL::gEngine->has(OsShell)->SetMouseCursor(cursor);
  mCurrentCursor = cursor;
}

bool Mouse::GetTrapped()
{
  if (mActiveWindow)
    return mActiveWindow->GetMouseTrap();
  return false;
}

void Mouse::SetTrapped(bool state)
{
  if (mActiveWindow)
    mActiveWindow->SetMouseTrap(state);
}

void Mouse::ToggleTrapped()
{
  if (mActiveWindow)
    mActiveWindow->SetMouseTrap(!mActiveWindow->GetMouseTrap());
}

} // namespace Plasma
