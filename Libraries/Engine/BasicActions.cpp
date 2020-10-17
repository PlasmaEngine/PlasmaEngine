// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(ActionDelay, builder, type)
{
  PlasmaBindDocumented();

  LightningBindField(mTimeLeft);
}

} // namespace Plasma
