// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{
class ContentItemHandleManager : public HandleManager
{
public:
  ContentItemHandleManager(ExecutableState* state) : HandleManager(state)
  {
  }

  // HandleManager Interface
  void ObjectToHandle(const ::byte* object, BoundType* type, Handle& handleToInitialize) override;
  ::byte* HandleToObject(const Handle& handle) override;
};

typedef uint ContentItemId;

// A content item is an object that represents a content generating
// item in the library. This is usually a single file and is generated
// from that file's meta file.
class ContentItem : public EventObject
{
public:
  LightningDeclareType(ContentItem, TypeCopyMode::ReferenceType);

  ContentItem();
  virtual ~ContentItem();

  String GetName();

  // Core Content Item
  ContentItemId Id;

  // The library this content item is contained within.
  ContentLibrary* mLibrary;

  // Short File name of this content item relative to the library
  String Filename;

  // Unique Id of the file
  String UniqueFileId;

  // Is this content item edited by the editor
  // with a resource or by editing the content
  // item directly?
  ContentEditMode::Enum EditMode;
  // Is there only one resource built from this content item?
  // Then rename or removed the resource renames or removed this
  // content item.
  bool mResourceIsContentItem;
  // The resource created from the content item.
  ResourceId mRuntimeResource;
  // If we should ignore warnings about multiple resources being
  // added from one content item when we go through the
  // 'add resource' path (only expects one resource)
  bool mIgnoreMultipleResourcesWarning;

  bool ShowInEditor;

  bool mNeedsEditorProcessing = false;

  virtual void AddComponent(ContentComponent* cc) = 0;

  // Save the content object meta file data.
  void SaveMetaFile();
  // Delete the meta data file.
  bool DeleteMetaFile();
  // Get the path to the meta file.
  String GetMetaFilePath();
  // Get the full path to the content file.
  String GetFullPath();

  /// Adds all tags of this content item to the given array.
  void GetTags(Array<String>& tags);
  void GetTags(HashSet<String>& tags);
  void SetTag(StringParam tag);
  void SetTags(HashSet<String>& tags);

  /// If applicable, remove all tags in the given set from this content item.
  void RemoveTags(HashSet<String>& tags);

  bool HasTag(StringParam tag);

  // Content Item Interface

  // Save the content file and meta file.
  void SaveContent();

  // Get the object the editor should use to edit the content item
  // (may be the content item or the runtime resource)
  virtual Object* GetEditingObject(Resource* resource);

  // Helper that builds the content item using build options from the content
  // libarary.
  void BuildContentItem(bool useJob);

  // Build the resource listing that this content item makes
  virtual void BuildListing(ResourceListing& listing);

  // Serialize this content item.
  virtual void Serialize(Serializer& stream);

  // Initialize the content item
  virtual void Initialize(ContentLibrary* library);

  // Get a content component
  virtual ContentComponent* QueryComponentId(BoundType* typeId);

  // Typed based interface for accessing components.
  template <typename type>
  type* Has()
  {
    return static_cast<type*>(QueryComponentId(LightningTypeId(type)));
  }

  // Called when content item is initialized for derived classes.
  virtual void OnInitialize();

protected:
  // Build the content item
  virtual void BuildContentItem(BuildOptions& buildOptions) = 0;
};

// Resource Meta Operations
class ContentItemMetaOperations : public MetaOperations
{
public:
  LightningDeclareType(ContentItemMetaOperations, TypeCopyMode::ReferenceType);

  /// MetaOperations Interface.
  void ObjectModified(HandleParam object, bool intermediateChange) override;
};

} // namespace Plasma
