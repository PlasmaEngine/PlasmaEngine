// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

DeclareBitField7(PhysicsSpaceDebugDrawFlags,
                 DrawDebug,
                 DrawOnTop,
                 DrawBroadPhase,
                 DrawConstraints,
                 DrawSleeping,
                 DrawSleepPreventors,
                 DrawCenterMass);

} // namespace Plasma
