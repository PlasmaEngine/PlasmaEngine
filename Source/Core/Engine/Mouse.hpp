// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

/// Mouse object for Display System.
class Mouse : public ExplicitSingleton<Mouse, EventObject>
{
public:
  LightningDeclareType(Mouse, TypeCopyMode::ReferenceType);

  Mouse();

  /// The position of the mouse cursor relative to the application's top-left
  /// corner in pixels.
  Vec2 GetClientPosition()
  {
    return mClientPosition;
  }

  /// The movement of the mouse in pixels.
  Vec2 GetCursorMovement()
  {
    return mCursorMovement;
  }

  /// Is a mouse button currently down?
  bool IsButtonDown(MouseButtons::Enum button);

  /// Set the cursor of the mouse.
  Cursor::Enum GetCursor();
  void SetCursor(Cursor::Enum cursor);

  /// Trap the mouse preventing it from moving.
  bool GetTrapped();
  void SetTrapped(bool state);

  /// Toggles if the mouse is currently trapped.
  void ToggleTrapped();

  friend class RootWidget;

  /// High precision raw movement of the mouse.
  Vec2 mRawMovement;
  Vec2 mCursorMovement;

  // Hack to make public for now
  OsWindow* mActiveWindow;

private:
  Cursor::Enum mCurrentCursor;
  OsShell* mPlatform;
  Vec2 mClientPosition;
  ::byte mButtonDown[MouseButtons::Size];
};

namespace PL
{
extern Mouse* gMouse;
}

} // namespace Plasma
