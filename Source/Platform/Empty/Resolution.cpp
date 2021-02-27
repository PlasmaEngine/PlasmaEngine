// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
Resolution GetDesktopResolution()
{
  return Resolution(cMinimumMonitorSize.x, cMinimumMonitorSize.y);
}

void Enumerate(Array<Resolution>& resolutions, int bitDepth, Resolution aspect)
{
  resolutions.PushBack(Resolution(cMinimumMonitorSize.x, cMinimumMonitorSize.y));
}

} // namespace Plasma
