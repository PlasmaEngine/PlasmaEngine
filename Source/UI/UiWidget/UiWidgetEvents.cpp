// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(UiFocusGainedPreview);
DefineEvent(UiFocusLost);
DefineEvent(UiFocusGained);
DefineEvent(UiFocusLostHierarchy);
DefineEvent(UiFocusGainedHierarchy);
DefineEvent(UiFocusReset);

} // namespace Events

LightningDefineType(UiFocusEvent, builder, type)
{
  PlasmaBindDocumented();
  LightningBindFieldGetter(mReceivedFocus);
  LightningBindFieldGetter(mLostFocus);
}

UiFocusEvent::UiFocusEvent(UiWidget* focusGained, UiWidget* focusLost) :
    mReceivedFocus(focusGained),
    mLostFocus(focusLost)
{
}

} // namespace Plasma
