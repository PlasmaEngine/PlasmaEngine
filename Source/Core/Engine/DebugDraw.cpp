// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#define AddDefinitions(DebugObjectType)                                                                                \
  void DebugDraw::Add(Debug::DebugObjectType& debugObject)                                                             \
  {                                                                                                                    \
    if (&debugObject != nullptr)                                                                                       \
      gDebugDraw->Add(debugObject);                                                                                    \
    else                                                                                                               \
      DoNotifyException("Null debug object", "You must initialize the debug object.");                                 \
  }                                                                                                                    \
  void DebugDraw::Add(uint spaceId, Debug::DebugObjectType& debugObject)                                               \
  {                                                                                                                    \
    if (&debugObject != nullptr)                                                                                       \
      gDebugDraw->Add(spaceId, debugObject);                                                                           \
    else                                                                                                               \
      DoNotifyException("Null debug object", "You must initialize the debug object.");                                 \
  }                                                                                                                    \
  void DebugDraw::Add(Space* space, Debug::DebugObjectType& debugObject)                                               \
  {                                                                                                                    \
    if (&debugObject != nullptr)                                                                                       \
      gDebugDraw->Add(space->GetRuntimeId(), debugObject);                                                             \
    else                                                                                                               \
      DoNotifyException("Null debug object", "You must initialize the debug object.");                                 \
  }

#define AddBindings(DebugObjectType)                                                                                   \
  LightningBindOverloadedMethod(Add, (void (*)(Debug::DebugObjectType&)));                                                 \
  LightningFullBindMethod(builder, type, LightningSelf::Add, (void (*)(Space*, Debug::DebugObjectType&)), "Add", "space, shape")

namespace Plasma
{
LightningDefineType(DebugDraw, builder, type)
{
  PlasmaBindDocumented();
#define PlasmaDebugPrimitive(X) AddBindings(X);
#include "Core/Geometry/DebugPrimitives.inl"
#undef PlasmaDebugPrimitive
}

#define PlasmaDebugPrimitive(X) AddDefinitions(X);
#include "Core/Geometry/DebugPrimitives.inl"
#undef PlasmaDebugPrimitive

} // namespace Plasma

#undef AddDefinitions
