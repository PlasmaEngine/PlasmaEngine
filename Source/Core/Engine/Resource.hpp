// MIT Licensed (see LICENSE.md).
#pragma once
namespace Plasma
{

// Forward Declarations
class ContentItem;
class ByteStream;
class BuilderComponent;
class ResourceManager;
class ResourceLibrary;
class ResourceEntry;
class Resource;
class ResourceTemplate;

DeclareEnum4(ResourceEditType, Data, Text, Custom, None);

const String cAnimationIcon = "AnimationIcon";
const String cGraphicsIcon = "MaterialIcon";
const String cAudioIcon = "AudioIcon";
const String cPhysicsSmallIcon = "SmallPhysicsIcon";
const String cCodeIcon = "ScriptIcon";
  
/// Resource Ids are 64 bit numbers. Stored in text files at a 16 digit hex
/// value.
typedef Guid ResourceId;

namespace Events
{
DeclareEvent(ResourceInstanceModified);
DeclareEvent(ResourceTagsModified);
} // namespace Events

namespace Tags
{
DeclareTag(Resource);
}

// Behavior for what happens when the resource id / name
// can not be found in the resource manager
DeclareEnum3(ResourceNotFound,
             // Return fallback resource and signal error
             ErrorFallback,
             // Return NULL no error
             ReturnNull,
             // Return Default Resource
             ReturnDefault);

// Resource Handle Manager
class ResourceHandleData
{
public:
  ResourceHandleData()
    : mDebugResource(nullptr), mRawObject(nullptr) {
  }

  // This will always be set for visual studio visualizers. If it's garbage,
  // it's because the resource was deleted. If we could call functions in
  // visualizers, this wouldn't be needed.
  Resource* mDebugResource;

  // Actual handle data
  Resource* mRawObject;
  ResourceId mId;
};

class ResourceHandleManager : public HandleManager
{
public:
  ResourceHandleManager(ExecutableState* state) : HandleManager(state)
  {
  }

  // HandleManager interface
  void Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags) override;
  void ObjectToHandle(const ::byte* object, BoundType* type, Handle& handleToInitialize) override;
  ::byte* HandleToObject(const Handle& handle) override;
  void AddReference(const Handle& handle) override;
  ReleaseResult::Enum ReleaseReference(const Handle& handle) override;
  void Delete(const Handle& handle) override;
  bool CanDelete(const Handle& handle) override;

  Resource* GetResource(const Handle& handle, bool resolveThroughManagerOnNull);
};

class ResourceDisplayFunctions : public MetaDisplay
{
public:
  LightningDeclareType(ResourceDisplayFunctions, TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

/// Base Resource class.
class Resource : public EventObject
{
public:
  LightningDeclareType(Resource, TypeCopyMode::ReferenceType);

  /// Memory heap for all resources
  static Memory::Heap* sHeap;
  OverloadedNew();

  /// Default Constructor / Destructor
  Resource();
  virtual ~Resource()
  {
  }

  // If a resource type holds any references to other resources, the handles
  // must be cleared in this method so that dangling references can be caught
  // when libraries unload.
  virtual void Unload()
  {
  }

  // To support runtime cloning, implement 'HandleOf<ResourceType>
  // RuntimeClone()' for the resource. Override this method to call RuntimeClone
  // and bind the RuntimeClone method. This method is needed for the
  // [RuntimeClone] feature in script.
  virtual HandleOf<Resource> Clone();

  /// Gets the name of the resource (or the contents file path if we have it)
  String GetNameOrFilePath();

  /// Gets the origin (named specifically to be used with Lightning compilation).
  /// This is what gets set on the Origin member of all CodeLocations.
  /// It's also used for setting breakpoints and should be unique per file.
  String GetOrigin();

  /// Can this resource be saved?
  bool IsWritable();

  /// Is the resource marked as runtime?
  bool IsRuntime();

  /// Resource Manager for this resource.
  ResourceManager* GetManager();

  /// Set the resource to default values
  virtual void SetDefaults()
  {
  }

  /// Save the content item to the given file.
  virtual void Save(StringParam filename)
  {
  }

  /// Called when a content item is added to the resource.
  virtual void UpdateContentItem(ContentItem* contentItem);

  /// How this resource is edited
  virtual ResourceEditType::Type GetEditType()
  {
    return ResourceEditType::None;
  }

  virtual void ResourceModified()
  {
  }

  void SendModified();

  /// Fills the given array with all tags associated with this resource.
  virtual void GetTags(Array<String>& coreTags, Array<String>& userTags);
  void GetTags(Array<String>& tags);
  void GetTags(HashSet<String>& tags);
  void AddTags(HashSet<String>& tags);
  void SetTags(HashSet<String>& tags);
  /// If applicable, remove all tags in the given set from this resource.
  void RemoveTags(HashSet<String>& tags);

  bool HasTag(StringParam tag);

  virtual void GetDependencies(HashSet<ContentItem*>& dependencies, HandleParam instance = nullptr);

  virtual DataNode* GetDataTree();

  /// Returns the base Resource that this Resource inherited form.
  Resource* GetBaseResource();

  /// Returns whether or not this Resource inherits from the given base
  /// Resource.
  bool InheritsFrom(Resource* baseResource);

  /// Can this resource hold a reference to the given resource. Checks resource
  /// library dependencies to see if the given resource is in a dependent
  /// (parent) library.
  bool CanReference(Resource* resource);

  ResourceTemplate* GetResourceTemplate();

  // Inherit Range
  struct InheritRange
  {
    InheritRange(Resource* current);

    Resource* Front();
    void PopFront();
    bool Empty();
    InheritRange& All()
    {
      return *this;
    }

    Resource* mCurrent;
  };

  InheritRange GetBaseResources();

  BuilderComponent* GetBuilder();

  /// Name of resource for display.
  String Name;

  /// Used to set the resource icon in the editor.
  String mResourceIconName;

  /// Global Unique Id for this resource
  ResourceId mResourceId;

  /// Name and resource id combined in Id:Name format.
  String ResourceIdName;

  /// Used for Resource inheritance.
  String mBaseResourceIdName;

  /// Filter tag for resources with sub types
  String FilterTag;

  /// Resource Manager for this resource.
  ResourceManager* mManager;

  /// Resource library that loaded this resource.
  ResourceLibrary* mResourceLibrary;

  /// Content System Values only valid if content system is loaded.
  /// Content Item used to build this resource.
  ContentItem* mContentItem;

  /// Builder type used for this content item. It's safe to store a pointer to
  /// the type because it will always be a native type.
  BoundType* mBuilderType;

  /// Denotes a resource created at runtime (not loaded from a file).
  bool mIsRuntimeResource;

private:
  friend class ResourceLibrary;
  friend class ResourceManager;
  friend class ResourceHandleManager;

  /// Reference counting
  void AddReference();
  int Release();
  int GetReferenceCount()
  {
    return mReferenceCount;
  }

  int mReferenceCount;
  // Can not copy resources
  Resource(const Resource& source);
  Resource& operator=(const Resource& source);
};

class ResourceMetaSerialization : public MetaSerialization
{
public:
  LightningDeclareType(ResourceMetaSerialization, TypeCopyMode::ReferenceType);
  void SerializeProperty(HandleParam instance, Property* property, Serializer& serializer) override;
  void SetDefault(Type* type, Any& any) override;

  String ConvertToString(AnyParam input) override;
  bool ConvertFromString(StringParam input, Any& output) override;
};

extern const String DataResourceExtension;

// Data Resource
/// A resource that is stored in our serialization data format and is directly
/// edited by the editor.
class DataResource : public Resource
{
public:
  /// Meta Initialization
  LightningDeclareType(DataResource, TypeCopyMode::ReferenceType);

  ResourceEditType::Type GetEditType() override
  {
    return ResourceEditType::Data;
  }
  void Save(StringParam filename) override;
  HandleOf<Resource> Clone() override;
  virtual void Serialize(Serializer& stream) = 0;
  virtual void Initialize()
  {
  }
  DataNode* GetDataTree() override;
};

// Resource Inheritance
class DataResourceInheritance : public MetaDataInheritanceRoot
{
public:
  LightningDeclareType(DataResourceInheritance, TypeCopyMode::ReferenceType);
  String GetInheritId(HandleParam object, InheritIdContext::Enum context) override;
  void SetInheritId(HandleParam object, StringParam inheritId) override;
  bool ShouldStoreLocalModifications(HandleParam object) override;
  void RebuildObject(HandleParam object) override
  {
  }
};

// Resource Meta Operations
class ResourceMetaOperations : public MetaOperations
{
public:
  LightningDeclareType(ResourceMetaOperations, TypeCopyMode::ReferenceType);

  u64 GetUndoHandleId(HandleParam object) override;
  Any GetUndoData(HandleParam object);
  void ObjectModified(HandleParam object, bool intermediateChange) override;
  void RestoreUndoData(HandleParam object, AnyParam undoData);
};

#define ResourceProperty(type, name)                                                                                   \
  HandleOf<type> m##name;                                                                                              \
  type* Get##name()                                                                                                    \
  {                                                                                                                    \
    return m##name;                                                                                                    \
  }                                                                                                                    \
  void Set##name(type* newResource)                                                                                    \
  {                                                                                                                    \
    if (newResource)                                                                                                   \
      m##name = newResource;                                                                                           \
  }

#define ResourceHandleProperty(type, name)                                                                             \
  HandleOf<type> m##name;                                                                                              \
  type* Get##name()                                                                                                    \
  {                                                                                                                    \
    return m##name;                                                                                                    \
  }                                                                                                                    \
  void Set##name(type* newResource)                                                                                    \
  {                                                                                                                    \
    m##name = newResource;                                                                                             \
  }

} // namespace Plasma
