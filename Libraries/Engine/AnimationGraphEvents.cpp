// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(AnimationBlendEnded);
DefineEvent(AnimationEnded);
DefineEvent(AnimationLooped);
DefineEvent(AnimationPostUpdate);
} // namespace Events

LightningDefineType(AnimationGraphEvent, builder, type)
{
  PlasmaBindDocumented();
  LightningBindFieldGetter(mAnimation);
  LightningBindFieldGetter(mNode);
  LightningBindFieldGetter(mPlayMode);
}

} // namespace Plasma
