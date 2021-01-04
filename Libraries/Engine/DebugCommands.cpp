#include "Precompiled.hpp"
namespace Plasma
{
    namespace Events
    {
        DefineEvent(DebugViewMode);
    } // namespace Events

    LightningDefineType(DebugViewEvent, builder, type)
    {
        PlasmaBindEvent(Events::DebugViewMode, DebugViewEvent);
        PlasmaBindDocumented();
        LightningBindDefaultCopyDestructor();
        LightningBindFieldProperty(mMode);
        LightningFullBindConstructor(builder, type, LightningSelf, "mode", GeometryValue::Enum);
        type->CreatableInScript = true;
    }
} // namespace Plasma