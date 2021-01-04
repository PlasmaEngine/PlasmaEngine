#pragma once
namespace Plasma
{
    DeclareEnum9(GeometryValue, None, Normal, Depth, Roughness,  Albedo,  Metallic, Specular, Emissive, AO )
  
    namespace Events
    {
        DeclareEvent(ScriptInitialize);
        DeclareEvent(DebugViewMode);
    } // namespace Eventsd

    class DebugViewEvent : public Event
    {
    public:
        LightningDeclareType(DebugViewEvent, TypeCopyMode::ReferenceType);
        DebugViewEvent(GeometryValue::Enum mode): mMode(mode)  { }
        DebugViewEvent()  { }
        ~DebugViewEvent() { }
        GeometryValue::Enum mMode = GeometryValue::Enum::None;
    };

} // Namespace Plasma