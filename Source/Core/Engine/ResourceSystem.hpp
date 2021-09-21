// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{
class ResourceManager;
class ResourceLoader;
class ResourceEvent;

namespace Events
{
DeclareEvent(ResourcesLoaded);
DeclareEvent(ResourcesUnloaded);
DeclareEvent(PackagedStarted);
DeclareEvent(PackagedFinished);
} // namespace Events

// How the content item is edited in the editor
DeclareEnum3(ContentEditMode,
             // Can not be edited.
             NoEdit,
             // Edit the resource directly (usually a DataResource)
             // The content item has a one to one relationship with a resource
             ResourceObject,
             // Edit the content item (usually multiple resources or binary)
             ContentItem);

/// ResourceSystem Contains all resource managers.
class ResourceSystem : public EventObject
{
public:
  LightningDeclareType(ResourceSystem, TypeCopyMode::ReferenceType);

  ResourceSystem();
  static void Initialize();

  // Add a resource manager
  void RegisterManager(ResourceManager* manager);

  ResourceManager* GetResourceManager(BoundType* resourceType);
  ResourceManager* GetResourceManager(StringParam resourceTypeName);

  // A loader loads a specific resource type for a manager.
  void AddLoader(StringParam loaderName, ResourceLoader* loader);

  // Load a resource package from a file.
  ResourceLibrary* LoadPackageFile(StringParam fileName);

  // Load a resource package
  ResourceLibrary* LoadPackage(Status& status, ResourcePackage* package);

  // Reload all resource in package into resource library.
  void ReloadPackage(ResourceLibrary* resourceLibrary, ResourcePackage* package);

  // Unload all resource libraries
  void UnloadAll();

  // Find resource with a resourceId
  Resource* GetResource(ResourceId resourceId);
  // Find resource with their name or name and id string
  Resource* GetResourceByName(StringParam resourceIdAndName);
  // Find a resource with a given type and name
  Resource* GetResourceByTypeAndName(StringParam resourceType, StringParam resourceName);
  // Find a resource library
  ResourceLibrary* GetResourceLibrary(StringParam name);

  void LoadIntoLibrary(Status& status, ResourceLibrary* resourceLibrary, ResourcePackage* resourcePackage, bool isNew);

  void OnResourcesLoaded(ResourceEvent* event);

  ResourceLibrary* GetResourceLibraryFromCurrentType(BoundType* currentType);

  void SetupDefaults();

  // Resources that were modified in the editor.
  HashSet<ResourceId> mModifiedResources;

  // Map of dependent resource library names to their respective resource libraries
  typedef OrderedHashMap<String, ResourceLibrary*> LoadedSetMap;
  LoadedSetMap LoadedDependencyLibraries;

  // Map of resource library names to resource libraries
  typedef OrderedHashMap<String, ResourceLibrary*> LoadedSetMap;
  LoadedSetMap LoadedResourceLibraries;

  // Map of resource type names to managers
  typedef HashMap<String, ResourceManager*> ManagerMapType;
  ManagerMapType Managers;

  // Map of ResourceId to loaded resources
  typedef HashMap<ResourceId, Resource*> ResourceIdMapType;
  ResourceIdMapType ResourceIdMap;

  // Map of document names to document resources. The text resources are a
  // separate resource list for editing a text files and find/replace.
  typedef HashMap<String, ResourceId> TextResourceMap;
  TextResourceMap TextResources;

  // private:
  HandleOf<Resource> LoadEntry(Status& status, ResourceEntry& entry);
  void ReloadEntry(Resource* resource, ResourceEntry& entry);

  // Map of resource type names to loaders
  typedef HashMap<String, ResourceLoader*> LoaderMapType;
  typedef LoaderMapType::range LoaderRange;
  LoaderMapType mLoaderMap;

  Array<ResourceManager*> mResourceManagers;
};

namespace PL
{
extern ResourceSystem* gResources;
}

} // namespace Plasma
