// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// Ranges
LightningDefineRange(MetaSelection::range);

// METAREFACTOR - move to platform meta library
LightningDefineExternalBaseType(IpAddress, TypeCopyMode::ReferenceType, builder, type)
{
  type->CreatableInScript = true;

  PlasmaBindDocumented();

  LightningBindConstructor();
  LightningBindConstructor(const IpAddress&);
  LightningBindConstructor(StringParam, uint, Plasma::InternetProtocol::Enum);
  LightningBindConstructor(StringParam, uint);

  LightningBindDestructor();

  LightningBindMethod(Clear);

  LightningBindCustomGetterProperty(IsValid);
  LightningBindGetterProperty(InternetProtocol);
  LightningBindGetterProperty(String);
  LightningBindCustomGetterProperty(Hash);

  LightningFullBindGetterSetter(builder,
                            type,
                            &Plasma::IpAddress::GetHost,
                            (String(Plasma::IpAddress::*)() const),
                            &Plasma::IpAddress::SetHost,
                            (void (Plasma::IpAddress::*)(StringParam)),
                            "Host");
  LightningFullBindGetterSetter(builder,
                            type,
                            &Plasma::IpAddress::GetPort,
                            (uint(Plasma::IpAddress::*)() const),
                            &Plasma::IpAddress::SetPort,
                            (void (Plasma::IpAddress::*)(uint)),
                            "Port");

  LightningBindGetterProperty(PortString);
}

// Enums
LightningDefineEnum(SendsEvents);
LightningDefineEnum(InternetProtocol);

PlasmaDefineArrayType(Array<Revision>);

LightningDefineStaticLibrary(MetaLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  LightningInitializeRangeAs(MetaSelection::range, "MetaSelectionRange");

  // Enums
  LightningInitializeEnum(SendsEvents);
  LightningInitializeEnum(InternetProtocol);

  // This needs to be the first thing initialized
  LightningInitializeType(Object);
  LightningInitializeType(EventObject);

  // Basic handles
  LightningInitializeType(ReferenceCountedEmpty);
  LightningInitializeType(SafeId32);
  LightningInitializeType(SafeId64);
  LightningInitializeType(ThreadSafeId32);
  LightningInitializeType(ThreadSafeId64);
  LightningInitializeType(ReferenceCountedSafeId32);
  LightningInitializeType(ReferenceCountedSafeId64);
  LightningInitializeType(ReferenceCountedThreadSafeId32);
  LightningInitializeType(ReferenceCountedThreadSafeId64);

  // Object handles
  LightningInitializeType(ReferenceCountedObject);
  LightningInitializeType(SafeId32Object);
  LightningInitializeType(SafeId64Object);
  LightningInitializeType(ThreadSafeId32Object);
  LightningInitializeType(ThreadSafeId64Object);
  LightningInitializeType(ReferenceCountedSafeId32Object);
  LightningInitializeType(ReferenceCountedSafeId64Object);
  LightningInitializeType(ReferenceCountedThreadSafeId32Object);
  LightningInitializeType(ReferenceCountedThreadSafeId64Object);

  // EventObject handles
  LightningInitializeType(ReferenceCountedEventObject);
  LightningInitializeType(SafeId32EventObject);
  LightningInitializeType(SafeId64EventObject);
  LightningInitializeType(ThreadSafeId32EventObject);
  LightningInitializeType(ThreadSafeId64EventObject);
  LightningInitializeType(ReferenceCountedSafeId32EventObject);
  LightningInitializeType(ReferenceCountedSafeId64EventObject);
  LightningInitializeType(ReferenceCountedThreadSafeId32EventObject);
  LightningInitializeType(ReferenceCountedThreadSafeId64EventObject);

  LightningInitializeType(ThreadSafeReferenceCounted);

  // Meta Components
  LightningInitializeType(MetaAttribute);
  LightningInitializeType(CogComponentMeta);
  LightningInitializeType(MetaOwner);
  LightningInitializeType(MetaGroup);
  LightningInitializeType(MetaCustomUi);
  LightningInitializeType(MetaOperations);
  LightningInitializeType(MetaPropertyFilter);
  LightningInitializeType(MetaPropertyBasicFilter);
  LightningInitializeType(MetaEditorGizmo);
  LightningInitializeType(MetaDisplay);
  LightningInitializeType(TypeNameDisplay);
  LightningInitializeType(StringNameDisplay);
  LightningInitializeType(MetaTransform);
  LightningInitializeType(MetaPropertyRename);
  LightningInitializeType(MetaShaderInput);
  LightningInitializeType(EditorPropertyExtension);
  LightningInitializeType(EditorIndexedStringArray);
  LightningInitializeType(EditorRange);
  LightningInitializeType(EditorSlider);
  LightningInitializeType(EditorRotationBasis);
  LightningInitializeType(MetaEditorResource);
  LightningInitializeType(MetaDataInheritance);
  LightningInitializeType(MetaDataInheritanceRoot);
  LightningInitializeType(MetaSerializedProperty);
  LightningInitializeType(MetaComposition);
  LightningInitializeType(MetaArray);
  LightningInitializeType(MetaArrayWrapper);

  // Events
  LightningInitializeType(Event);
  LightningInitializeType(MetaLibraryEvent);
  LightningInitializeType(SelectionChangedEvent);
  LightningInitializeType(NotifyEvent);
  LightningInitializeType(PropertyEvent);
  LightningInitializeType(TypeEvent);

  LightningInitializeType(MetaSelection);
  LightningInitializeType(ObjectEvent);
  LightningInitializeType(LocalModifications);
  LightningInitializeType(PropertyPath);
  LightningInitializeExternalType(IpAddress);

  LightningInitializeType(PropertyPath);
  LightningInitializeType(Revision);

  PlasmaInitializeArrayTypeAs(Array<Revision>, "Revisions");

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

void MetaLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  AttributeExtensions::Initialize();

  RegisterFunctionAttribute(Lightning::StaticAttribute)->AllowStatic(true);
  RegisterFunctionAttribute(Lightning::VirtualAttribute);
  RegisterFunctionAttribute(Lightning::OverrideAttribute);
  RegisterFunctionAttribute(PropertyAttributes::cInternal);
  RegisterFunctionAttribute(FunctionAttributes::cDisplay)->AllowStatic(true);

  RegisterPropertyAttribute(Lightning::StaticAttribute)->AllowStatic(true);
  RegisterPropertyAttribute(Lightning::VirtualAttribute);
  RegisterPropertyAttribute(Lightning::OverrideAttribute);
  RegisterPropertyAttribute(PropertyAttributes::cProperty);
  RegisterPropertyAttribute(PropertyAttributes::cInternal);
  RegisterPropertyAttribute(PropertyAttributes::cSerialize);
  RegisterPropertyAttribute(PropertyAttributes::cDeprecatedSerialized);
  RegisterPropertyAttribute(PropertyAttributes::cDisplay)->AllowStatic(true);
  RegisterPropertyAttribute(PropertyAttributes::cDeprecatedEditable)->AllowStatic(true);
  RegisterPropertyAttribute(PropertyAttributes::cRuntimeClone)->AllowStatic(true);
  RegisterPropertyAttributeType(PropertyAttributes::cShaderInput, MetaShaderInput)->AllowMultiple(true);
  RegisterPropertyAttributeType(PropertyAttributes::cRenamedFrom, MetaPropertyRename);
  RegisterPropertyAttribute(PropertyAttributes::cLocalModificationOverride);
  RegisterPropertyAttributeType(PropertyAttributes::cGroup, MetaGroup)->AllowStatic(true);
  RegisterPropertyAttributeType(PropertyAttributes::cRange, EditorRange)->TypeMustBe(float)->AllowStatic(true);
  RegisterPropertyAttributeType(PropertyAttributes::cSlider, EditorSlider)->TypeMustBe(float)->AllowStatic(true);
}

void MetaLibrary::Shutdown()
{
  AttributeExtensions::Destroy();

  GetLibrary()->ClearComponents();
}

} // namespace Plasma
