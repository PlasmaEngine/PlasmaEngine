// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

const String DataResourceExtension = "data";

namespace Events
{
DefineEvent(ResourceInstanceModified);
DefineEvent(ResourceTagsModified);
} // namespace Events

namespace Tags
{
DefineTag(Resource);
}

const String cNullResource = "null";

// Resource Handle Manager
void ResourceHandleManager::Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags)
{
  handleToInitialize.Flags |= HandleFlags::NoReferenceCounting;

  ResourceHandleData& data = *(ResourceHandleData*)(handleToInitialize.Data);
  data.mRawObject = (Resource*)plAllocate(type->Size);
  memset((void*)data.mRawObject, 0, type->Size);
  data.mDebugResource = data.mRawObject;
  data.mId = 0;
}

void ResourceHandleManager::ObjectToHandle(const ::byte* object, BoundType* type, Handle& handleToInitialize)
{
  if (object == nullptr)
    return;

  Resource* resource = (Resource*)object;

  ResourceHandleData& data = *(ResourceHandleData*)(handleToInitialize.Data);

  data.mDebugResource = resource;

  resource->AddReference();
  if (resource->mResourceId == 0)
    data.mRawObject = resource;
  else
    data.mId = resource->mResourceId;
}

::byte* ResourceHandleManager::HandleToObject(const Handle& handle)
{
  return (::byte*)GetResource(handle, true);
}

void ResourceHandleManager::AddReference(const Handle& handle)
{
  if (Resource* instance = GetResource(handle, false))
    instance->AddReference();
}

ReleaseResult::Enum ResourceHandleManager::ReleaseReference(const Handle& handle)
{
  if (Resource* instance = GetResource(handle, false))
  {
    ErrorIf(instance == nullptr, "I think this should be null when unloading a ResourceLibrary");
    instance->Release();
  }
  return ReleaseResult::TakeNoAction;
}

void ResourceHandleManager::Delete(const Handle& handle)
{
  if (Resource* resource = GetResource(handle, false))
  {
    ErrorIf(!resource->IsRuntime(),
            "Only runtime resources should be able to be deleted through"
            " handles. Otherwise, it should be deleted through its "
            "ResourceLibrary");
    resource->GetManager()->Remove(resource, RemoveMode::Unloading);
    delete resource;
  }
}

bool ResourceHandleManager::CanDelete(const Handle& handle)
{
  // We only allow users to delete runtime resources
  if (Resource* resource = GetResource(handle, false))
    return resource->IsRuntime();
  return false;
}

Resource* ResourceHandleManager::GetResource(const Handle& handle, bool resolveThroughManagerOnNull)
{
  ResourceHandleData& data = *(ResourceHandleData*)(handle.Data);

  if (data.mRawObject)
    return data.mRawObject;

  // If the handle was never assigned a Resource, no reason to use the fallback
  // Resource
  if (data.mId == 0)
    return nullptr;

  // If we can't find the resource, we need to get the fallback from the
  // resource manager if it exists, otherwise it will just be null
  Resource* resource = PL::gResources->GetResource(data.mId);

  if (resource == nullptr && resolveThroughManagerOnNull)
  {
    if (ResourceManager* manager = PL::gResources->GetResourceManager(handle.StoredType))
      resource = manager->GetFallbackResource();
  }
  return resource;
}

void SaveResource(cstr fieldName, Resource* resource, Serializer& serializer)
{
  // If the resource is valid (may be null) read its
  // resourceIdAndName
  StringRange resourceIdAndName = cNullResource;

  // Runtime resources can never be loaded from data, so write out null
  if (resource && !resource->IsRuntime())
    resourceIdAndName = resource->ResourceIdName.All();

  // Serialize the resourceIdAndName
  serializer.StringField("string", fieldName, resourceIdAndName);
}

void LoadResource(HandleParam instance, Property* property, Type* resourceType, StringRange resourceIdName)
{
  // If it's null, do not default
  if (resourceIdName == cNullResource)
  {
    // Set property to null resource.
    property->SetValue(instance, Any(resourceType));
    return;
  }

  Resource* resource;

  // Try to find the resource or use default if it is not found
  ResourceManager* resourceManger = PL::gResources->Managers.FindValue(resourceType->ToString(), nullptr);
  if (resourceManger)
    resource = resourceManger->GetResource(resourceIdName, ResourceNotFound::ReturnNull);
  else
    resource = PL::gResources->GetResourceByName(resourceIdName);

  if (resource)
  {
    // Valid resource was serialized
    property->SetValue(instance, resource);
  }
  else
  {
    // Serialize failed resource is missing so use default resource
    ResourceManager* resourceManger = PL::gResources->Managers.FindValue(resourceType->ToString(), nullptr);
    if (resourceManger)
      property->SetValue(instance, resourceManger->GetDefaultResource());
  }
}

// ResourceMetaSerialization
LightningDefineType(ResourceMetaSerialization, builder, type)
{
}

void ResourceMetaSerialization::SerializeProperty(HandleParam instance, Property* property, Serializer& serializer)
{
  Type* resourceType = property->PropertyType;
  cstr fieldName = property->Name.c_str();

  if (serializer.GetMode() == SerializerMode::Loading)
  {
    // Serialize a Resource to a String that is a resource name and Id.
    StringRange resourceIdAndName;
    if (serializer.StringField("string", fieldName, resourceIdAndName))
      LoadResource(instance, property, resourceType, resourceIdAndName);
  }
  else
  {
    Any value = property->GetValue(instance);
    Resource* resource = value.Get<Resource*>();
    SaveResource(fieldName, resource, serializer);
  }
}

void ResourceMetaSerialization::SetDefault(Type* type, Any& any)
{
  ResourceManager* manager = PL::gResources->Managers.FindValue(type->ToString(), nullptr);
  if (manager != nullptr)
  {
    Resource* defaultResource = manager->GetDefaultResource();
    if (defaultResource != nullptr)
      any = defaultResource;
    else
      any = (Resource*)nullptr;
  }
  else
    any = (Resource*)nullptr;
}

String ResourceMetaSerialization::ConvertToString(AnyParam input)
{
  if (Resource* resource = input.Get<Resource*>())
    return resource->ResourceIdName;

  return String();
}

bool ResourceMetaSerialization::ConvertFromString(StringParam input, Any& output)
{
  // 'output' can be null, but null is a valid result so return true.
  output = PL::gResources->GetResourceByName(input);
  return true;
}

String ResourceToString(const BoundType* type, const ::byte* value)
{
  // Convert a Resource to a String
  Resource* resource = (Resource*)value;
  if (resource)
    return resource->ResourceIdName;
  else
    return String();
}

// METAREFACTOR See below where we set all this information on the meta
// bool ResourceStringConversion(MetaType* metaType, StringParam string,
// Variant& var)
//{
//  // Find a Resource from String. Allows system to pass strings for resource
//  parameters or properties. ResourceManager* resourceManger =
//  PL::gResources->Managers.FindValue(metaType->TypeName, NULL);
//  if(resourceManger)
//  {
//    var = resourceManger->GetResource(string,
//    ResourceNotFound::ErrorFallback); return true;
//  }
//  else
//  {
//    return false;
//  }
//}

// ResourceDisplayFunctions
LightningDefineType(ResourceDisplayFunctions, builder, Type)
{
}

String ResourceDisplayFunctions::GetName(HandleParam object)
{
  Resource* resource = object.Get<Resource*>(GetOptions::AssertOnNull);
  return resource->Name;
}

String ResourceDisplayFunctions::GetDebugText(HandleParam object)
{
  Resource* resource = object.Get<Resource*>(GetOptions::AssertOnNull);
  StringBuilder builder;
  builder << "<";
  builder << object.StoredType->Name << " ";
  builder << resource->ResourceIdName;
  builder << ">";
  return builder.ToString();
}

Memory::Heap* Resource::sHeap = new Memory::Heap("ResourceObjects", Memory::GetRoot());

LightningDefineType(Resource, builder, type)
{
  type->AddAttribute(ObjectAttributes::cResourceInterface);

  // METAREFACTOR Handle all this resource stuff
  // meta->StringConversion = ResourceStringConversion;
  // meta->SetDefaultValue = ResourceSetDefaultValue;
  type->HandleManager = LightningManagerId(ResourceHandleManager);
  type->Add(new ResourceMetaSerialization());
  type->Add(new ResourceDisplayFunctions());
  type->Add(new ResourceMetaOperations());

  // When serialized as a property, Resources save out a resource id and name
  PlasmaBindSerializationPrimitive();

  PlasmaBindDocumented();
  LightningBindFieldGetterProperty(Name);
  LightningBindFieldGetterProperty(mResourceIconName);
  type->ToStringFunction = ResourceToString;

  PlasmaBindTag(Tags::Resource);
}

void* Resource::operator new(size_t size)
{
  return sHeap->Allocate(size);
}

void Resource::operator delete(void* pMem, size_t size)
{
  return sHeap->Deallocate(pMem, size);
}

Resource::Resource()
{
  mResourceIconName = "ResourceIcon";
  mResourceId = 0;
  mManager = nullptr;
  mResourceLibrary = nullptr;
  mContentItem = nullptr;
  mBuilderType = nullptr;
  mIsRuntimeResource = false;
  mReferenceCount = 0;
}

HandleOf<Resource> Resource::Clone()
{
  BoundType* type = LightningVirtualTypeId(this);
  String msg = String::Format("%s's cannot be runtime cloned", type->Name.c_str());
  DoNotifyException("Failed to clone Resource", msg);
  return nullptr;
}

DataNode* Resource::GetDataTree()
{
  return nullptr;
}

Resource* Resource::GetBaseResource()
{
  return PL::gResources->GetResourceByName(mBaseResourceIdName);
}

bool Resource::InheritsFrom(Resource* baseResource)
{
  forRange (Resource* curr, GetBaseResources())
  {
    if (curr == baseResource)
      return true;
  }

  return false;
}

bool Resource::CanReference(Resource* resource)
{
  return mResourceLibrary->CanReference(resource->mResourceLibrary);
}

ResourceTemplate* Resource::GetResourceTemplate()
{
  if (mContentItem)
    return mContentItem->has(ResourceTemplate);
  return nullptr;
}

Resource::InheritRange Resource::GetBaseResources()
{
  InheritRange r(this);
  r.PopFront();
  return r;
}

BuilderComponent* Resource::GetBuilder()
{
  if (!mContentItem)
    return nullptr;
  return (BuilderComponent*)mContentItem->QueryComponentId(mBuilderType);
}

void Resource::AddReference()
{
  AtomicPreIncrement(&mReferenceCount);
}

int Resource::Release()
{
  ErrorIf(mReferenceCount == 0, "Invalid Release.");

  int referenceCount = (int)AtomicPreDecrement(&mReferenceCount);
  if (referenceCount == 0)
  {
    // Resources owned by a resource library should only be deleted by that
    // library. Library unloading flag will only be true while a library is
    // clearing its own handles.
    ErrorIf(!IsRuntime() && !ResourceLibrary::IsLibraryUnloading(),
            "A library resource is being removed but not by the library.");

    mManager->Remove(this, RemoveMode::Deleted);
    delete this;
  }

  return referenceCount;
}

void Resource::GetDependencies(HashSet<ContentItem*>& dependencies, HandleParam instance)
{
  Handle resourceInstance = instance;
  if (resourceInstance.IsNull())
    resourceInstance = this;

  // Get all resources used by this component
  HashSet<Resource*> usedResources;
  GetResourcesFromProperties(resourceInstance, usedResources);

  // Filter runtime and non-writable resources
  forRange (Resource* resource, usedResources.All())
  {
    if (resource->IsWritable() && !resource->IsRuntime())
    {
      dependencies.Insert(resource->mContentItem);

      // Add all dependencies of the resource
      resource->GetDependencies(dependencies);
    }
  }
}

void Resource::UpdateContentItem(ContentItem* contentItem)
{
  mContentItem = contentItem;
}

void Resource::GetTags(Array<String>& tags)
{
  // Add both core and user tags to the same array
  GetTags(tags, tags);
}

void Resource::GetTags(HashSet<String>& tags)
{
  // Add both core and user tags to the same array
  static Array<String> temp;
  temp.Clear();
  GetTags(temp, temp);

  forRange (String tag, temp.All())
  {
    tags.Insert(tag);
  }
}

void Resource::GetTags(Array<String>& coreTags, Array<String>& userTags)
{
  // First add the resource type as a tag
  coreTags.PushBack(LightningVirtualTypeId(this)->Name);

  if (!FilterTag.Empty())
    coreTags.PushBack(FilterTag);

  // Add all tags from the content item
  if (mContentItem != nullptr)
    mContentItem->GetTags(userTags);
}

void Resource::AddTags(HashSet<String>& tags)
{
  Array<String> tagArray;
  GetTags(tagArray);
  forRange (String tag, tagArray.All())
  {
    tags.Insert(tag);
  }
}

void Resource::SetTags(HashSet<String>& tags)
{
  if (mContentItem != nullptr)
    mContentItem->SetTags(tags);
}

void Resource::RemoveTags(HashSet<String>& tags)
{
  if (mContentItem != nullptr)
    mContentItem->RemoveTags(tags);
}

bool Resource::HasTag(StringParam tag)
{
  if (mContentItem)
    return mContentItem->HasTag(tag);
  return false;
}

String Resource::GetNameOrFilePath()
{
  if (mContentItem)
    return mContentItem->GetFullPath();

  return Name;
}

String Resource::GetOrigin()
{
  // This will ensure it's always unique
  return ResourceIdName;
}

bool Resource::IsWritable()
{
  if (mContentItem)
  {
    bool isWritable = mContentItem->mLibrary->GetWritable();

    // Check dev config to override what the content item says
    if (!isWritable)
    {
      if (DeveloperConfig* devConfig = PL::gEngine->GetConfigCog()->has(Plasma::DeveloperConfig))
        isWritable = devConfig->mCanModifyReadOnlyResources;
    }

    return isWritable;
  }
  return false;
}

bool Resource::IsRuntime()
{
  return mIsRuntimeResource;
}

ResourceManager* Resource::GetManager()
{
  return mManager;
}

void Resource::SendModified()
{
  ResourceEvent event;
  event.Manager = mManager;
  event.EventResource = this;
  mManager->DispatchEvent(Events::ResourceModified, &event);
  PL::gResources->DispatchEvent(Events::ResourceModified, &event);
  DispatchEvent(Events::ObjectStructureModified, &event);
}

Resource::InheritRange::InheritRange(Resource* current) : mCurrent(current)
{
}

Resource* Resource::InheritRange::Front()
{
  return mCurrent;
}

void Resource::InheritRange::PopFront()
{
  ReturnIf(Empty(), , "Cannot pop front on an empty range.");

  mCurrent = mCurrent->GetBaseResource();
}

bool Resource::InheritRange::Empty()
{
  return (mCurrent == nullptr);
}

LightningDefineType(DataResource, builder, type)
{
  type->Add(new DataResourceInheritance());
  type->AddAttribute(ObjectAttributes::cResourceInterface);

  PlasmaBindDocumented();
}

void DataResource::Save(StringParam filename)
{
  // Cannot use ObjectSaver with the current way resource id's are expected to
  // be assigned. Added resources that don't come from a template save to temp
  // file before an id is given by the content system, but interfaces used by
  // ObjectSaver require a handle and resource handles require an id...
  SaveToDataFile(*this, filename);

  // ObjectSaver saver;
  // Status status;
  // saver.Open(status, filename.c_str());
  // ReturnIf(status.Failed(), , "Failed to save resource '%s' to '%s'\n",
  // Name.c_str(), filename.c_str());

  // saver.SaveDefinition(this);
  // saver.Close();
}

HandleOf<Resource> DataResource::Clone()
{
  HandleOf<DataResource> resourceHandle(mManager->CreateRuntimeInternal());
  DataResource* resource = resourceHandle;

  // Save ourself to a buffer
  ObjectSaver saver;
  saver.OpenBuffer();
  saver.SaveInstance(this);

  // Serialize the new resource with the saved data
  DataTreeLoader loader;
  Status status;
  loader.OpenBuffer(status, saver.GetString());
  ReturnIf(status.Failed(), nullptr, "Failed to serialize runtime clone");

  PolymorphicNode resourceNode;
  loader.GetPolymorphic(resourceNode);
  resource->Serialize(loader);
  loader.EndPolymorphic();

  resource->Initialize();

  return resourceHandle;
}

DataNode* DataResource::GetDataTree()
{
  Status status;
  ObjectLoader loader;
  loader.OpenFile(status, mContentItem->GetFullPath());
  return loader.TakeOwnershipOfFirstRoot();
}

// Resource Inheritance
LightningDefineType(DataResourceInheritance, builder, type)
{
}

String DataResourceInheritance::GetInheritId(HandleParam object, InheritIdContext::Enum context)
{
  DataResource* resource = object.Get<DataResource*>();
  if (context == InheritIdContext::Definition)
    return resource->mBaseResourceIdName;
  return String();
}

void DataResourceInheritance::SetInheritId(HandleParam object, StringParam inheritId)
{
  DataResource* resource = object.Get<DataResource*>();
  resource->mBaseResourceIdName = inheritId;
}

bool DataResourceInheritance::ShouldStoreLocalModifications(HandleParam object)
{
  DataResource* resource = object.Get<DataResource*>();
  return !resource->mBaseResourceIdName.Empty();
}

// Resource Meta Operations
LightningDefineType(ResourceMetaOperations, builder, type)
{
}

u64 ResourceMetaOperations::GetUndoHandleId(HandleParam object)
{
  Resource* resource = object.Get<Resource*>();
  return (u64)resource->mResourceId;
}

Any ResourceMetaOperations::GetUndoData(HandleParam object)
{
  Resource* resource = object.Get<Resource*>(GetOptions::AssertOnNull);
  bool isModified = PL::gResources->mModifiedResources.Contains(resource->mResourceId);

  // Temporary until we fix issues with how this works
  isModified = true;

  return isModified;
}

void ResourceMetaOperations::ObjectModified(HandleParam object, bool intermediateChange)
{
  MetaOperations::ObjectModified(object, intermediateChange);

  if (!intermediateChange)
  {
    Resource* resource = object.Get<Resource*>(GetOptions::AssertOnNull);
    resource->ResourceModified();
    PL::gResources->mModifiedResources.Insert(resource->mResourceId);
  }

  // We used to dispatch an event on the Manager. Should we?
}

void ResourceMetaOperations::RestoreUndoData(HandleParam object, AnyParam undoData)
{
  Resource* resource = object.Get<Resource*>(GetOptions::AssertOnNull);
  bool wasModified = undoData.Get<bool>();

  if (wasModified == false)
    PL::gResources->mModifiedResources.Erase(resource->mResourceId);
}

} // namespace Plasma
