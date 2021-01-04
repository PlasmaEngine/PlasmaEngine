// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(ScriptInitialize);
DefineEvent(DebugViewMode);
} // namespace Events

LightningDefineType(DebugViewEvent, builder, type)
{
  PlasmaBindEvent(Events::DebugViewMode, DebugViewEvent);
  PlasmaBindDocumented();
  LightningBindDefaultCopyDestructor();
  LightningBindFieldProperty(mMode);
  LightningFullBindConstructor(builder, type, LightningSelf, "mode", int);
  type->CreatableInScript = true;
}
  
LightningDefineType(TextEvent, builder, type)
{
  LightningBindFieldProperty(Text);
}

LightningDefineType(TextErrorEvent, builder, type)
{
}

LightningDefineType(ProgressEvent, builder, type)
{
}

LightningDefineType(BlockingTaskEvent, builder, type)
{
}

ProgressEvent::ProgressEvent()
{
  ProgressType = ProgressType::Normal;
  Percentage = 0.0f;
}

} // namespace Plasma
