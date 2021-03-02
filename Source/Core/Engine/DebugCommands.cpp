#include "Precompiled.hpp"
namespace Plasma
{
    namespace Events
    {
        DefineEvent(DebugViewMode);
        DefineEvent(ViewportCameraSettings);
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

    LightningDefineType(ViewportCameraEvent, builder, type)
    {
        PlasmaBindEvent(Events::ViewportCameraSettings, ViewportCameraEvent);
        PlasmaBindDocumented();
        LightningBindDefaultCopyDestructor();
        LightningBindFieldProperty(mSpeed);
        LightningFullBindConstructor(builder, type, LightningSelf, "speed", float);
        type->CreatableInScript = true;
    }
} // namespace Plasma