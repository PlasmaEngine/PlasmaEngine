// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

// All Components need to call this in their meta initialization
#define PlasmaBindComponent()                                                                                            \
  LightningBindDefaultConstructor();                                                                                       \
  LightningBindDestructor();

// Meta Resource
// If a Type was created from a Resource, the resource id will be available as a
// type component.
class MetaResource : public ReferenceCountedEventObject
{
public:
  LightningDeclareType(MetaResource, TypeCopyMode::ReferenceType);

  MetaResource()
  {
  }
  MetaResource(Resource* resource);

  void SetResource(Resource* resource);

  // The resource this type is defined in.
  ResourceId mResourceId;

  // The location this type is defined at.
  CodeLocation mClassLocation;
};

// Editor Script Object
class MetaEditorScriptObject : public MetaAttribute
{
public:
  LightningDeclareType(MetaEditorScriptObject, TypeCopyMode::ReferenceType);
  MetaEditorScriptObject();

  void PostProcess(Status& status, ReflectionObject* owner) override;

  bool mAutoRegister;
};

// Meta Dependency
class MetaDependency : public MetaAttribute
{
public:
  LightningDeclareType(MetaDependency, TypeCopyMode::ReferenceType);

  void PostProcess(Status& status, ReflectionObject* owner) override;
};

// Meta Interface
class MetaInterface : public MetaAttribute
{
public:
  LightningDeclareType(MetaInterface, TypeCopyMode::ReferenceType);

  void PostProcess(Status& status, ReflectionObject* owner) override;
};

} // namespace Plasma
