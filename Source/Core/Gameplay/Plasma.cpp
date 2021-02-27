// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// Plasma Static
LightningDefineType(PlasmaStatic, builder, type)
{
  PlasmaBindDocumented();
  LightningBindMethod(Connect);
  LightningBindMethod(Disconnect);
  LightningBindMethodAs(DisconnectAllEvents, "DisconnectAll");

  LightningBindGetter(Keyboard);
  LightningBindGetter(Mouse);
  LightningBindGetter(Editor);
  LightningBindGetter(Engine);
  LightningBindGetter(Environment);
  LightningBindGetter(Gamepads);
  LightningBindGetter(Joysticks);
  LightningBindGetter(ObjectStore);
  LightningBindGetter(ResourceSystem);
  LightningBindGetter(OsShell);
  LightningBindGetter(Audio);
}

void ErrorOnConnect(Function* func)
{
  DoNotifyException("Lightning Error",
                    String::Format("Plasma.Connect must take a delegate that has only one "
                                   "parameter (a reference / class Event type). Function %s "
                                   "has the signature: %s",
                                   func->Name.c_str(),
                                   func->FunctionType->ToString().c_str()));
}

void PlasmaStatic::Connect(Object* target, StringParam eventId, DelegateParam delegate)
{
  if (target == nullptr)
  {
    DoNotifyException("The target object is null", "Cannot connect to a null object");
    return;
  }

  if (delegate.IsNull())
  {
    DoNotifyException("The delegate is null", "Cannot connect a null delegate");
    return;
  }

  Function* callbackFunction = delegate.BoundFunction;
  const ParameterArray& params = callbackFunction->FunctionType->Parameters;

  // The function should only take an event
  if (params.Size() != 1)
  {
    ErrorOnConnect(callbackFunction);
    return;
  }

  // Confirm that the parameter type is an event
  Type* type = params[0].ParameterType;
  BoundType* eventType = Type::DynamicCast<BoundType*>(type);
  if (eventType == nullptr || !eventType->IsA(LightningTypeId(Event)))
  {
    ErrorOnConnect(callbackFunction);
    return;
  }

  // Get the event support object (Dispatcher/Receiver)
  EventDispatcher* dispatcher = target->GetDispatcherObject();
  Object* receiverObject = delegate.ThisHandle.Get<Object*>();

  if (receiverObject == nullptr)
  {
    DoNotifyException("The object is null", "Cannot connect a null object");
    return;
  }

  EventReceiver* receiver = receiverObject->GetReceiverObject();

  if (dispatcher == nullptr)
  {
    DoNotifyException("The dispatcher is null", "Cannot connect a null dispatcher");
    return;
  }

  if (receiver == nullptr)
  {
    DoNotifyException("The receiver is null", "Cannot connect a null receiver");
    return;
  }

  // Create the connection
  LightningScriptConnection* connection = new LightningScriptConnection(dispatcher, eventId, delegate);
  connection->EventType = eventType;

  if (!dispatcher->IsUniqueConnection(connection))
  {
    connection->Flags.SetFlag(ConnectionFlags::DoNotDisconnect);
    delete connection;
    return;
  }

  // Connect
  connection->ConnectToReceiverAndDispatcher(eventId, receiver, dispatcher);
}

void PlasmaStatic::Disconnect(Object* sender, StringParam eventId, Object* receiver)
{
  if (sender == nullptr)
  {
    DoNotifyException("Cannot disconnect", "Sender object is null");
    return;
  }

  if (receiver == nullptr)
  {
    DoNotifyException("Cannot disconnect", "Receiver object is null");
    return;
  }

  EventDispatcher* dispatcher = sender->GetDispatcherObject();
  dispatcher->DisconnectEvent(eventId, receiver);
}

void PlasmaStatic::DisconnectAllEvents(Object* sender, Object* receiver)
{
  if (sender == nullptr)
  {
    DoNotifyException("Cannot disconnect", "Sender object is null");
    return;
  }

  if (receiver == nullptr)
  {
    DoNotifyException("Cannot disconnect", "Receiver object is null");
    return;
  }

  EventDispatcher* dispatcher = sender->GetDispatcherObject();
  dispatcher->Disconnect(receiver);
}

Keyboard* PlasmaStatic::GetKeyboard()
{
  return Keyboard::Instance;
}

Mouse* PlasmaStatic::GetMouse()
{
  return PL::gMouse;
}

Editor* PlasmaStatic::GetEditor()
{
  return PL::gEditor;
}

Engine* PlasmaStatic::GetEngine()
{
  return PL::gEngine;
}

Environment* PlasmaStatic::GetEnvironment()
{
  return Environment::GetInstance();
}

Gamepads* PlasmaStatic::GetGamepads()
{
  return PL::gGamepads;
}

Plasma::Joysticks* PlasmaStatic::GetJoysticks()
{
  return PL::gJoysticks;
}

ObjectStore* PlasmaStatic::GetObjectStore()
{
  return ObjectStore::GetInstance();
}

ResourceSystem* PlasmaStatic::GetResourceSystem()
{
  return PL::gResources;
}

OsShell* PlasmaStatic::GetOsShell()
{
  return GetEngine()->has(OsShell);
}

SoundSystem* PlasmaStatic::GetAudio()
{
  return PL::gSound;
}

// LightningScriptConnection
LightningScriptConnection::LightningScriptConnection(EventDispatcher* dispatcher, StringParam eventId, DelegateParam delagate) :
    EventConnection(dispatcher, eventId)
{
  Flags.SetFlag(ConnectionFlags::Script);
  mDelegate = delagate;
  mStatePatchId = (size_t)-1;
  ThisObject = mDelegate.ThisHandle.Dereference();
}

LightningScriptConnection::~LightningScriptConnection()
{
  // Since LightningScriptConnections contain a handle in the delegate, it is
  // possible for the event connection to be the only thing keeping an object
  // alive. Cannot destruct the object during the event connection destruction
  // because it will also invoke destruction of the event connection being
  // destructed right now.
  sDelayDestructDelegates.PushBack(mDelegate);
}

void LightningScriptConnection::RaiseError(StringParam message)
{
  Function* function = mDelegate.BoundFunction;
  if (function == nullptr)
  {
    DoNotifyException("Null Event Connection", message);
    return;
  }

  // Get the location of the first opcode
  const CodeLocation* location = function->OpcodeLocationToCodeLocation.FindPointer(0);
  if (location)
  {
    String namedMessage = BuildString(function->Name, ": ", message);
    String fullMessage = location->GetFormattedStringWithMessage(MessageFormat::Python, namedMessage);
    LightningScriptManager::GetInstance()->DispatchScriptError(
        Events::UnhandledException, namedMessage, fullMessage, *location);
    return;
  }

  DoNotifyException("Event Connection", String::Format("%s\nFunction: ", function->ToString().c_str()));
}

void LightningScriptConnection::Invoke(Event* e)
{
  // If the patch id matches the one on the state, then it means we disabled
  // this event handler Once the state gets patched, the event handler will auto
  // resume (it may throw again, and that will re-disable it)
  if (mStatePatchId == ExecutableState::CallingState->PatchId)
    return;

  ExceptionReport report;
  Call call(mDelegate);

  // We have to disable debug checks because technically we are doing something
  // unsafe here The Lightning binding will yell at us for trying to pass in an
  // 'Event' where it takes a more derived type
  call.DisableParameterChecks();

  call.SetHandle(0, e);
  call.Invoke(report);

  // If an exception has occurred kill this connection
  // to prevent callbacks like 'KeyDown' from constantly throwing exceptions
  if (report.HasThrownExceptions())
    mStatePatchId = ExecutableState::CallingState->PatchId;
}

DataBlock LightningScriptConnection::GetFunctionPointer()
{
  return DataBlock((byte*)&mDelegate.BoundFunction, sizeof(Function*));
}

} // namespace Plasma
