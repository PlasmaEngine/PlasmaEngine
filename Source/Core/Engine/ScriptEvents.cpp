// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(DebuggerPaused);
DefineEvent(DebuggerResumed);
DefineEvent(SyntaxError);
DefineEvent(UnhandledException);
} // namespace Events

LightningDefineType(ScriptEvent, builder, type)
{
}

} // namespace Plasma
