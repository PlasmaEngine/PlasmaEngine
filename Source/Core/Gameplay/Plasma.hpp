// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class Editor;

// Plasma Static
/// Global functionality exposed to Lightning script. Bound as "Plasma" to script
/// (e.g. Plasma.Keyboard) PlasmaStatic was used to avoid the conflict with
/// namespace Plasma).
class PlasmaStatic
{
public:
  LightningDeclareType(PlasmaStatic, TypeCopyMode::ReferenceType);

  /// Connection invokes the given delegate when sender dispatches the specified
  /// event.
  static void Connect(Object* sender, StringParam eventId, DelegateParam receiverDelegate);
  /// Removes specified event connection,
  /// if connection delegate was a component method then receiver object is just
  /// the component.
  static void Disconnect(Object* sender, StringParam eventId, Object* receiver);
  /// Removes all event connections between sender and receiver,
  /// if connection delegate was a component method then receiver object is just
  /// the component.
  static void DisconnectAllEvents(Object* sender, Object* receiver);

  static Keyboard* GetKeyboard();
  static Mouse* GetMouse();
  static Editor* GetEditor();
  static Engine* GetEngine();
  static Environment* GetEnvironment();
  static Gamepads* GetGamepads();
  static Joysticks* GetJoysticks();
  static ObjectStore* GetObjectStore();
  static ResourceSystem* GetResourceSystem();
  static OsShell* GetOsShell();
  static SoundSystem* GetAudio();

  // static Joysticks* GetJoysticks();
  // static MultiTouch* GetMultiTouch();
};

// LightningScriptConnection
/// LightningScriptConnection enables lightning to connect to any event in the engine.
class LightningScriptConnection : public EventConnection
{
public:
  LightningScriptConnection(EventDispatcher* dispatcher, StringParam eventId, DelegateParam delagate);
  ~LightningScriptConnection();

  void RaiseError(StringParam message) override;
  void Invoke(Event* event) override;
  DataBlock GetFunctionPointer() override;

  size_t mStatePatchId;
  Delegate mDelegate;
};

} // namespace Plasma
