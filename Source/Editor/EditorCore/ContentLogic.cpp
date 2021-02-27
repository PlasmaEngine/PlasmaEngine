// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void LoadContentConfig()
{
  InitializeContentSystem();

  ContentSystem* contentSystem = PL::gContentSystem;

  Cog* configCog = PL::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);

  Array<String>& librarySearchPaths = contentSystem->LibrarySearchPaths;
  ContentConfig* contentConfig = configCog->has(ContentConfig);

  String sourceDirectory = mainConfig->SourceDirectory;
  ErrorIf(sourceDirectory.Empty(), "Expected a source directory");

  if (contentConfig)
    librarySearchPaths.InsertAt(0, contentConfig->LibraryDirectories.All());

  librarySearchPaths.PushBack(FilePath::Combine(sourceDirectory, "Resources"));

  contentSystem->ToolPath = FilePath::Combine(sourceDirectory, "Tools");

  contentSystem->mHistoryEnabled = contentConfig->HistoryEnabled;

  // To avoid conflicts of assets of different versions(especially when the
  // version selector goes live) set the content folder to a unique directory
  // based upon the version number
  String revisionChangesetName = BuildString("Version-", GetRevisionNumberString(), "-", GetChangeSetString());
  contentSystem->ContentOutputPath =
      FilePath::Combine(GetUserDocumentsApplicationDirectory(), "ContentOutput", revisionChangesetName);
  contentSystem->PrebuiltContentPath =
      FilePath::Combine(sourceDirectory, "Build", "PrebuiltContent", revisionChangesetName);
  PlasmaPrint("Content output directory '%s'\n", contentSystem->ContentOutputPath.c_str());
}

bool LoadContentLibrary(StringParam name)
{
  ZoneScoped;
  ProfileScopeFunctionArgs(name);
  ContentLibrary* library = PL::gContentSystem->Libraries.FindValue(name, nullptr);
  if (!library)
  {
    FatalEngineError("Failed to find core content library %s.\n", name.c_str());
    return false;
  }

  Status status;
  HandleOf<ResourcePackage> packageHandle = PL::gContentSystem->BuildLibrary(status, library, false);
  ResourcePackage* package = packageHandle;

  if (!status)
    return false;

  forRange (ResourceEntry& entry, package->Resources.All())
  {
    if (entry.mLibrarySource)
    {
      if (ContentEditorOptions* options = entry.mLibrarySource->has(ContentEditorOptions))
        entry.mLibrarySource->ShowInEditor = options->mShowInEditor;
      else
        entry.mLibrarySource->ShowInEditor = false;
    }
  }

  PL::gResources->LoadPackage(status, package);
  if (!status)
  {
    FatalEngineError("Failed to load core content library for editor. Resources"
                     " need to be in the working directory.");
  }

  return (bool)status;
}

void LoadCoreContent(Array<String>& coreLibs)
{
  ZoneScoped;
  ProfileScopeFunction();
  PL::gContentSystem->EnumerateLibraries();

  PlasmaPrint("Loading Content...\n");

  LoadContentLibrary("FragmentCore");
  LoadContentLibrary("Loading");

  forRange (String libraryName, coreLibs.All())
  {
    LoadContentLibrary(libraryName);
  }
}

} // namespace Plasma
