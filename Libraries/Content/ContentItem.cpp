// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
LightningDefineType(ContentItem, builder, type)
{
  type->HandleManager = LightningManagerId(ContentItemHandleManager);
  type->Add(new ContentItemMetaOperations());

  LightningBindGetterProperty(Name);
}

void ContentItemHandleManager::ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize)
{
  if (object == nullptr)
    return;

  ContentItem* item = (ContentItem*)object;
  handleToInitialize.HandleU32 = item->Id;
}

byte* ContentItemHandleManager::HandleToObject(const Handle& handle)
{
  return (byte*)PL::gContentSystem->LoadedItems.FindValue(handle.HandleU32, nullptr);
}

ContentItem::ContentItem()
{
  mLibrary = nullptr;
  mRuntimeResource = 0;
  ShowInEditor = true;
  EditMode = ContentEditMode::NoEdit;
  mResourceIsContentItem = false;
  mIgnoreMultipleResourcesWarning = false;

  // Generate an Id for this Content Item
  ++PL::gContentSystem->mIdCount;
  Id = PL::gContentSystem->mIdCount;

  // Store the handle
  PL::gContentSystem->LoadedItems[Id] = this;
}

ContentItem::~ContentItem()
{
  PL::gContentSystem->LoadedItems.Erase(Id);
}

String ContentItem::GetName()
{
  return FilePath::GetFileNameWithoutExtension(Filename);
}

void ContentItem::SaveContent()
{
  ErrorIf(EditMode == ContentEditMode::NoEdit, "Can not save. No edit type.");

  if (EditMode == ContentEditMode::ResourceObject)
  {
    // Ask the resource to save itself
    Resource* resource = PL::gResources->GetResource(mRuntimeResource);
    ErrorIf(resource == nullptr, "Resource has been removed but this content item still refers to it.");

    if (resource)
    {
      if (PL::gContentSystem->mHistoryEnabled)
      {
        String backUpPath = PL::gContentSystem->GetHistoryPath(mLibrary);
        BackUpFile(backUpPath, GetFullPath());
      }
      resource->Save(this->GetFullPath());
    }
  }

  // Save the meta file
  SaveMetaFile();
}

void ContentItem::SaveMetaFile()
{
  String metaFilePath = GetMetaFilePath();
  SaveToDataFile(*this, metaFilePath);
}

String ContentItem::GetMetaFilePath()
{
  return BuildString(GetFullPath(), ".meta");
}

String ContentItem::GetFullPath()
{
  return FilePath::Combine(mLibrary->SourcePath, Filename);
}

void ContentItem::GetTags(Array<String>& tags)
{
  ContentTags* contentTags = this->has(ContentTags);
  if (contentTags == nullptr)
    return;

  forRange (String currTag, contentTags->mTags.All())
  {
    tags.PushBack(currTag);
  }
}

void ContentItem::GetTags(HashSet<String>& tags)
{
  ContentTags* contentTags = this->has(ContentTags);
  if (contentTags == nullptr)
    return;

  forRange (String currTag, contentTags->mTags.All())
  {
    tags.Insert(currTag);
  }
}

void ContentItem::SetTag(StringParam tag)
{
  ContentTags* contentTags = this->has(ContentTags);
  if (contentTags == nullptr)
  {
    contentTags = new ContentTags();
    AddComponent(contentTags);
  }

  // Insert tag
  contentTags->mTags.Insert(tag);
}

void ContentItem::SetTags(HashSet<String>& tags)
{
  if (tags.Empty())
    return;

  ContentTags* contentTags = this->has(ContentTags);
  if (contentTags == nullptr)
  {
    contentTags = new ContentTags();
    AddComponent(contentTags);
  }

  // Insert each tag
  forRange (String tag, tags.All())
  {
    contentTags->mTags.Insert(tag);
  }
}

void ContentItem::RemoveTags(HashSet<String>& tags)
{
  ContentTags* contentTags = this->has(ContentTags);
  if (contentTags == nullptr)
    return;

  forRange (String& tag, tags.All())
  {
    contentTags->mTags.Erase(tag);
  }
}

bool ContentItem::HasTag(StringParam tag)
{
  ContentTags* contentTags = this->has(ContentTags);
  if (contentTags)
    return contentTags->mTags.Contains(tag);
  return false;
}

Object* ContentItem::GetEditingObject(Resource* resource)
{
  ErrorIf(EditMode == ContentEditMode::NoEdit, "Can not edit. No edit type.");
  if (EditMode == ContentEditMode::ResourceObject)
  {
    if (resource == nullptr)
      return PL::gResources->GetResource(mRuntimeResource);

    BoundType* resourceType = LightningVirtualTypeId(resource);

    forRange (Property* prop, resourceType->GetProperties())
    {
      if (prop->HasAttribute(PropertyAttributes::cProperty))
        return resource;
    }

    return this;
  }
  else
  {
    return this;
  }
}

void ContentItem::BuildContentItem(bool useJob)
{
  ZoneScoped;
  ProfileScopeFunctionArgs(Filename);
  BuildOptions options(mLibrary);
  BuildContentItem(options);
}

void ContentItem::BuildListing(ResourceListing& listing)
{
}

void ContentItem::Serialize(Serializer& stream)
{
}

void ContentItem::Initialize(ContentLibrary* library)
{
  mLibrary = library;
  if (library)
    library->AddContentItem(this);
  OnInitialize();
}

ContentComponent* ContentItem::QueryComponentId(BoundType* typeId)
{
  return nullptr;
}

void ContentItem::OnInitialize()
{
}

// Resource Meta Operations
LightningDefineType(ContentItemMetaOperations, builder, type)
{
}

void ContentItemMetaOperations::ObjectModified(HandleParam object, bool intermediateChange)
{
  MetaOperations::ObjectModified(object, intermediateChange);

  if (!intermediateChange)
  {
    ContentItem* contentItem = object.Get<ContentItem*>(GetOptions::AssertOnNull);
    PL::gContentSystem->mModifiedContentItems.Insert(contentItem->Id);
  }
}

} // namespace Plasma
