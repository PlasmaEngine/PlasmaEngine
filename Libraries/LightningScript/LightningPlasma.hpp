// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

// Lightning Component
/// A base class for all Lightning components defined in script
class LightningComponent : public Component
{
public:
  LightningDeclareInheritableType(LightningComponent, TypeCopyMode::ReferenceType);

  // Component interface
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void ScriptInitialize(CogInitializer& initializer) override;
  void OnDestroy(uint flags) override;
  void Serialize(Serializer& stream) override;
  ObjPtr GetEventThisObject() override;
  void DebugDraw() override;
  void Delete() override;
};

// Lightning Event
/// A base class for all Lightning events defined in script
class LightningEvent : public Event
{
public:
  LightningDeclareInheritableType(LightningEvent, TypeCopyMode::ReferenceType);

  // Event interface
  void Serialize(Serializer& stream) override;
  void Delete() override;
};

// Lightning Object
/// A base class for any object in Lightning that we want to have properties / meta
/// / send and receive events
class LightningObject : public EventObject
{
public:
  LightningDeclareInheritableType(LightningObject, TypeCopyMode::ReferenceType);

  // Object interface
  void Serialize(Serializer& stream) override;
  void Delete() override;
  void DispatchEvent(StringParam eventId, Event* event);
};

DeclareEnum2(FindLightningComponentResult, Success, NameConflictedWithNative);

} // namespace Plasma
