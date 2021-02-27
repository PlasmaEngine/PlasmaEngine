// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

/// Events sent by the DebugEngine.
namespace Events
{
DeclareEvent(DebuggerPaused);
DeclareEvent(DebuggerResumed);
DeclareEvent(SyntaxError);
DeclareEvent(UnhandledException);
} // namespace Events

class DocumentResource;

class ScriptEvent : public Event
{
public:
  LightningDeclareType(ScriptEvent, TypeCopyMode::ReferenceType);

  ScriptEvent()
  {
    Script = nullptr;
  }
  CodeLocation Location;
  String Message;
  /// The document resource that should be displayed due to an error
  DocumentResource* Script;
};

} // namespace Plasma
