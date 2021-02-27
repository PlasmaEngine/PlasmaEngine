// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

// This is probably a bad dependency, but there's not many
// other places that make sense for launching the background task
#include "Editor/EditorCore/SimpleBackgroundTasks.hpp"

namespace Plasma
{
LightningDefineType(LightningPluginSource, builder, type)
{
  LightningBindMethodProperty(OpenIde);
  LightningBindMethodProperty(OpenDirectory);
  LightningBindMethodProperty(CopyPluginDependencies);
  LightningBindMethodProperty(CompileDebug);
  LightningBindMethodProperty(CompileRelease);
  LightningBindMethodProperty(Clean);
  LightningBindMethodProperty(InstallIdeTools);
}

LightningPluginSource::LightningPluginSource()
{
  mCompileTask = nullptr;
  mOpenIdeAfterToolsInstallCounter = 0;
}

LightningPluginSource::~LightningPluginSource()
{
}

void LightningPluginSource::Serialize(Serializer& stream)
{
}

void LightningPluginSource::EditorInitialize()
{
  ConnectThisTo(PL::gEngine, Events::EngineUpdate, OnEngineUpdate);
}

String LightningPluginSource::GetContentDirectory()
{
  if (mContentItem == nullptr)
    return String();

  return mContentItem->mLibrary->SourcePath;
}

String LightningPluginSource::GetCodeDirectory()
{
  return FilePath::Combine(GetContentDirectory(), Name);
}

String LightningPluginSource::GetVersionsDirectory()
{
  return FilePath::Combine(GetCodeDirectory(), "Versions");
}

String LightningPluginSource::GetCurrentVersionDirectory()
{
  return FilePath::Combine(GetVersionsDirectory(), LightningPluginBuilder::GetSharedLibraryPlatformBuildName());
}

LightningPluginLibrary* LightningPluginSource::GetLibrary() const
{
  return LightningPluginLibraryManager::FindOrNull(Name);
}

void CopyGeneratedSource(StringParam destFileName, StringParam code)
{
  if (FileExists(destFileName))
  {
    Status status;
    if (CompareFileAndString(status, destFileName, code) == false)
      DeleteFile(destFileName);
    else
      return;
  }
  WriteStringRangeToFile(destFileName, code);
}

void LightningPluginSource::ForceCopyPluginDependencies()
{
  // Only do this code when we have content loaded
  if (mContentItem == nullptr)
    return;

  Cog* configCog = PL::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);

  // All plugin dependencies (library and headers) are located within our data
  // directory This is copied here automatically by post build events We place
  // it under a directory mangled by the platform name so that we can support
  // both Debug/Release builds (and possibly x86/x64 in the future) This ALSO
  // needs to partially match where we place the lib/all-to-one header in the
  // Data directory (does not include the version number since every version is
  // separated by the launcher) This is setup within the Plasma engine build
  // process (typically as a post build event)
  String platformName = LightningPluginBuilder::GetSharedLibraryPlatformName();
  String sourceVersionDir = FilePath::Combine(mainConfig->DataDirectory, "LightningCustomPluginShared", platformName);

  String destVersionDir = GetCurrentVersionDirectory();
  CreateDirectoryAndParents(destVersionDir);

  // Copy all the files from the source directory to the destination directory
  for (FileRange files(sourceVersionDir); !files.Empty(); files.PopFront())
  {
    String sourceFileName = files.Front();
    String sourceFile = FilePath::Combine(sourceVersionDir, sourceFileName);
    String destFile = FilePath::Combine(destVersionDir, sourceFileName);

    if (FileExists(destFile))
    {
      Status status;
      if (Plasma::CompareFile(status, sourceFile, destFile) == false)
        CopyFile(destFile, sourceFile);
    }
    else
    {
      CopyFile(destFile, sourceFile);
    }
  }

  // Create Core library source files
  Lightning::LibraryRef coreLibrary = Lightning::Core::GetInstance().GetLibrary();

  Lightning::NativeStubCode coreStubber;
  coreStubber.PrecompiledHeader = BuildString("\"", Name, "Precompiled.hpp\"");
  coreStubber.HppHeader = "// MIT Licensed (see LICENSE.md).";
  coreStubber.CppHeader = "// MIT Licensed (see LICENSE.md).";

  coreStubber.Generate(coreLibrary);

  String coreName = coreLibrary->Name;
  String coreHppFileName = FilePath::CombineWithExtension(destVersionDir, coreName, ".hpp");
  String coreCppFileName = FilePath::CombineWithExtension(destVersionDir, coreName, ".cpp");

  CopyGeneratedSource(coreHppFileName, coreStubber.Hpp);
  CopyGeneratedSource(coreCppFileName, coreStubber.Cpp);

  // Create PlasmaEngine library source files
  Lightning::NativeStubCode plasmaStubber;
  plasmaStubber.PrecompiledHeader = BuildString("\"", Name, "Precompiled.hpp\"");
  plasmaStubber.HppHeader = "// MIT Licensed (see LICENSE.md).";
  plasmaStubber.CppHeader = "// MIT Licensed (see LICENSE.md).";

  Lightning::BoundType* cogType = EngineLibrary::GetLibrary()->BoundTypes["Cog"];
  plasmaStubber.CustomClassHeaderDefines[cogType] = "PlasmaCogGetComponentTemplate;";

  plasmaStubber.HppMiddle = "#define PlasmaCogGetComponentTemplate                                     "
                          " \\\n"
                          "  template <typename ComponentType>                                     "
                          " \\\n"
                          "  ComponentType* GetComponent()                                         "
                          " \\\n"
                          "  {                                                                     "
                          " \\\n"
                          "    Lightning::String name = LightningTypeId(ComponentType)->Name;              "
                          " \\\n"
                          "    return static_cast<ComponentType*>                                  "
                          " \\\n"
                          "      (this->GetComponentByName(name).Dereference());                   "
                          " \\\n"
                          "  }                                                                     "
                          "   \n"
                          "#define has(ComponentType) GetComponent<ComponentType>()                "
                          "   \n";

  plasmaStubber.HppFooter = "namespace PlasmaEngine                                                    "
                          "   \n"
                          "{                                                                       "
                          "   \n"
                          "  template <typename ClassType>                                         "
                          "   \n"
                          "  void Connect                                                          "
                          "   \n"
                          "  (                                                                     "
                          "   \n"
                          "    PlasmaEngine::Object* sender,                                         "
                          "   \n"
                          "    const Lightning::String& eventName,                                     "
                          "   \n"
                          "    const Lightning::String& functionName,                                  "
                          "   \n"
                          "    ClassType* receiver                                                 "
                          "   \n"
                          "  )                                                                     "
                          "   \n"
                          "  {                                                                     "
                          "   \n"
                          "    Lightning::BoundType* type = LightningTypeId(ClassType);                    "
                          "   \n"
                          "    Lightning::FunctionArray* instanceFunctions =                           "
                          "   \n"
                          "      type->InstanceFunctions.FindPointer(functionName);                "
                          "   \n"
                          "    ReturnIf(instanceFunctions == nullptr || "
                          "instanceFunctions->Empty(),,  \n"
                          "      \"In %s making an event connection to %s we could \"              "
                          "   \n"
                          "      \"not find a function by the name of %s\",                        "
                          "   \n"
                          "      type->Name.c_str(), eventName.c_str(), functionName.c_str());     "
                          "   \n"
                          "                                                                        "
                          "   \n"
                          "    Lightning::Delegate delegate;                                           "
                          "   \n"
                          "    delegate.ThisHandle = Lightning::Handle((byte*)receiver, type);         "
                          "   \n"
                          "    delegate.BoundFunction = (*instanceFunctions)[0];                   "
                          "   \n"
                          "    PlasmaEngine::Plasma::Connect(sender, eventName, delegate);             "
                          "   \n"
                          "  }                                                                     "
                          "   \n"
                          "}                                                                       "
                          "   \n"
                          "#define PlasmaConnectThisTo(Sender, EventName, FunctionName)              "
                          " \\\n"
                          "  ::PlasmaEngine::Connect(Sender, EventName, FunctionName, this);         "
                          "   \n";

  // Emit all the attributes we have in Plasma as string constants
  LightningScriptManager* manager = LightningScriptManager::GetInstance();
  Lightning::StringBuilderExtended hppAttributes;
  hppAttributes.WriteLine("namespace PlasmaEngine");
  hppAttributes.WriteLine("{");
  Lightning::StringBuilderExtended cppAttributes;
  cppAttributes.WriteLine("namespace PlasmaEngine");
  cppAttributes.WriteLine("{");

  HashSet<String> attributes;
  AttributeExtensions* attributeExtensions = AttributeExtensions::GetInstance();
  attributes.Append(attributeExtensions->mClassExtensions.Keys());
  attributes.Append(attributeExtensions->mPropertyExtensions.Keys());
  attributes.Append(attributeExtensions->mFunctionExtensions.Keys());

  forRange (String& attribute, attributes.All())
  {
    hppAttributes.Write("  extern const Lightning::String ");
    hppAttributes.Write(attribute);
    hppAttributes.Write("Attribute;");
    hppAttributes.WriteLine();

    cppAttributes.Write("  const Lightning::String ");
    cppAttributes.Write(attribute);
    cppAttributes.Write("Attribute(\"");
    cppAttributes.Write(attribute);
    cppAttributes.Write("\");");
    cppAttributes.WriteLine();
  }

  hppAttributes.WriteLine("}");
  cppAttributes.WriteLine("}");
  plasmaStubber.HppFooter = BuildString(plasmaStubber.HppFooter, hppAttributes.ToString());
  plasmaStubber.CppFooter = BuildString(plasmaStubber.CppFooter, cppAttributes.ToString());

  // We need to make the Plasma engine header, but we want to
  // remove the Lightning Core library from the array on meta database
  Array<LibraryRef> nativeLibrariesWithoutCore = MetaDatabase::GetInstance()->mNativeLibraries;
  nativeLibrariesWithoutCore.EraseValueError(coreLibrary);

  String plasmaNamespace = plasmaStubber.Generate(nativeLibrariesWithoutCore);

  String plasmaHppFileName = FilePath::CombineWithExtension(destVersionDir, plasmaNamespace, ".hpp");
  String plasmaCppFileName = FilePath::CombineWithExtension(destVersionDir, plasmaNamespace, ".cpp");

  CopyGeneratedSource(plasmaHppFileName, plasmaStubber.Hpp);
  CopyGeneratedSource(plasmaCppFileName, plasmaStubber.Cpp);
}

void LightningPluginSource::CopyPluginDependencies()
{
  WriteCurrentVersionFile();
  ForceCopyPluginDependencies();
}

void LightningPluginSource::CopyPluginDependenciesOnce()
{
  // Only do this code when we have content loaded
  if (mContentItem == nullptr)
    return;

  // We only bother to copy plugin dependencies if plugins exist
  if (DirectoryExists(GetCodeDirectory()) == false)
    return;

  // We always write the current version, regardless of if we've already copied
  // the dependencies
  WriteCurrentVersionFile();

  // If we already have a directory for the current version, just skip this step
  if (DirectoryExists(GetCurrentVersionDirectory()))
    return;

  ForceCopyPluginDependencies();
}

void LightningPluginSource::WriteCurrentVersionFile()
{
  // Only do this code when we have content loaded
  if (mContentItem == nullptr)
    return;

#if defined(PlasmaTargetOsWindows)
  String revisionNumber = GetRevisionNumberString();
  String propsFile = BuildString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
                                 "<Project ToolsVersion=\"4.0\" "
                                 "xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n"
                                 "  <PropertyGroup>\r\n"
                                 "    <PlasmaVersion>",
                                 revisionNumber,
                                 "</PlasmaVersion>\r\n"
                                 "  </PropertyGroup>\r\n"
                                 "</Project>\r\n");

  String codeDir = GetCodeDirectory();
  CreateDirectoryAndParents(codeDir);
  String versionFile = FilePath::Combine(codeDir, BuildString(Name, "Version.props"));
  WriteStringRangeToFile(versionFile, propsFile);
#endif
}

void LightningPluginSource::OpenDirectory()
{
  CopyPluginDependenciesOnce();

  // Now open the folder up that we just created for the plugin
  Os::ShellOpenDirectory(GetCodeDirectory());
}

bool LightningPluginSource::IsIdeInstalled()
{
  return Os::GetEnvironmentalVariable("VCToolsInstallDir").SizeInBytes() != 0;
}

void LightningPluginSource::OpenIde()
{
  if (mOpenIdeAfterToolsInstallCounter > 0)
    return;

  CopyPluginDependenciesOnce();
  String codeDir = GetCodeDirectory();

#if defined(PlasmaTargetOsWindows)

  if (CheckIdeAndInformUser() == false)
    return;

  String ideFile = FilePath::CombineWithExtension(codeDir, Name, ".bat");
  StringParam params(ideFile);
  Status status;
  if (Os::ShellOpenFile(params))
  {
    status = Status(StatusState::Success, "Sucsess");
  }
  else
  {
    status = Status(StatusState::Failure, "Failed");
  }
  DoNotifyStatus(status);

#else
  DoNotifyErrorNoAssert("Lightning Plugin", "No IDE was detected or supported on this platform");
#endif
}

void LightningPluginSource::OnEngineUpdate(UpdateEvent* event)
{
#if defined(PlasmaTargetOsWindows)
  if (mOpenIdeAfterToolsInstallCounter > 0)
  {
    // If the installer process is finally stopped
    static const String VSIXInstaller("VSIXInstaller.exe");
    if (FindProcess(VSIXInstaller).mProcessId == 0)
    {
      ++mOpenIdeAfterToolsInstallCounter;

      if (mOpenIdeAfterToolsInstallCounter > InstallCounter)
      {
        mOpenIdeAfterToolsInstallCounter = 0;
        OpenIde();
      }
    }
  }
#endif
}

void LightningPluginSource::InstallIdeTools()
{
  MarkAttemptedIdeToolsInstAll();

#if defined(PlasmaTargetOsWindows)
  String extensionPath = FilePath::Combine(PL::gContentSystem->ToolPath, "PlasmaLightningPlugins.vsix");
  Os::ShellOpenFile(extensionPath);
#else
  DoNotifyErrorNoAssert("Lightning Plugin", "No IDE Plugins were detected or supported on this platform");
#endif
}

LightningPluginConfig* LightningPluginSource::GetConfig()
{
  LightningPluginConfig* config = HasOrAdd<LightningPluginConfig>(PL::gEngine->GetConfigCog());
  return config;
}

void LightningPluginSource::MarkAttemptedIdeToolsInstAll()
{
  // This resource is NOT actually modified, but I want to signal to the engine
  // to save the config (refactor me)
  GetConfig()->mAttemptedIdeToolsInstall = true;
  ResourceModified();
}

void LightningPluginSource::CompileDebug()
{
  CompileConfiguration("Debug");
}

void LightningPluginSource::CompileRelease()
{
  CompileConfiguration("Release");
}

bool LightningPluginSource::CheckIdeAndInformUser()
{
  if (GetResourceTemplate())
    return false;

  if (IsIdeInstalled() == false)
  {
    DoNotifyWarning("Lightning Plugin",
                    "No IDE was detected, you must first install a C++ IDE for "
                    "your platform");

#if defined(PlasmaTargetOsWindows)
    Os::OpenUrl("https://www.visualstudio.com/"
                "post-download-vs?sku=community&clcid=0x409");
#endif
    return false;
  }
  return true;
}

void LightningPluginSource::Clean()
{
#if defined(PlasmaTargetOsWindows)
  String codeDir = GetCodeDirectory();
  String ideFile = FilePath::Combine(codeDir, BuildString(Name, "Clean.bat"));
  Os::ShellOpenFile(ideFile);
#endif
}

void LightningPluginSource::CompileConfiguration(StringParam configuration)
{
  // Don't allow compilation more than once at a time
  if (mCompileTask != nullptr && mCompileTask->IsCompleted() == false)
    return;

  if (CheckIdeAndInformUser() == false)
    return;

  CopyPluginDependenciesOnce();
  String codeDir = GetCodeDirectory();
  String taskName = BuildString("Compiling ", configuration, " ", Name);

#if defined(PlasmaTargetOsWindows)
  String configurationBatchFileName = BuildString(Name, "Build", configuration);
  String configurationBatchFilePath = FilePath::CombineWithExtension(codeDir, configurationBatchFileName, ".bat");
  String process = BuildString("cmd /C \"", configurationBatchFilePath, "\"");

  ExecuteProcessTaskJob* job = new ExecuteProcessTaskJob(process);
  mCompileTask = PL::gBackgroundTasks->Execute(job, taskName);
  mCompileTask->mActivateOnCompleted = true;
  // Listen for the task completion events
  ConnectThisTo(mCompileTask, Events::BackgroundTaskCompleted, OnCompilationCompleted);
  ConnectThisTo(mCompileTask, Events::BackgroundTaskFailed, OnCompilationCompleted);

  // We can't get progress, so we'll have to estimate the time to complete
  mCompileTask->mIndeterminate = true;
  mCompileTask->mEstimatedTotalDuration = 15.0f;

  // Other parts of the engine may want to know when a plugin is currently
  // compiling We decrement this above in the 'CompletedCompilation' callback
  LightningPluginSourceManager* manager = LightningPluginSourceManager::GetInstance();
  ++manager->mCompilingPluginCount;
#else
  DoNotifyErrorNoAssert("Lightning Plugin", "Cannot automatically compile the plugin for this platform");
#endif
}

void LightningPluginSource::OnCompilationCompleted(BackgroundTaskEvent* e)
{
  LightningPluginSourceManager* manager = LightningPluginSourceManager::GetInstance();
  --manager->mCompilingPluginCount;

  ExecuteProcessTaskJob* job = (ExecuteProcessTaskJob*)e->mTask->GetFinishedJob();
  // Check for failure
  if (e->State == BackgroundTaskState::Failed || job->mExitCode != 0)
  {
    String msg = String::Format("Plugin '%s' failed to compile", Name.c_str());
    DoNotifyError("Plugin Compilation Failed", msg);
  }
}

HandleOf<Resource> LightningPluginSourceLoader::LoadFromFile(ResourceEntry& entry)
{
  LightningPluginSource* plugin = new LightningPluginSource();
  LightningPluginSourceManager::GetInstance()->AddResource(entry, plugin);
  return plugin;
}

void LightningPluginSourceLoader::ReloadFromFile(Resource* resource, ResourceEntry& entry)
{
}

ImplementResourceManager(LightningPluginSourceManager, LightningPluginSource);

LightningPluginSourceManager::LightningPluginSourceManager(BoundType* resourceType) : ResourceManager(resourceType)
{
  mCategory = "Code";
  mCanDuplicate = false;
  mCanReload = true;
  mCanAddFile = false;
  mPreview = false;
  mSearchable = true;
  mExtension = DataResourceExtension;
  mNoFallbackNeeded = true;
  mCanCreateNew = true;
  mCompilingPluginCount = 0;

  AddLoader("LightningPluginSource", new LightningPluginSourceLoader());

  ConnectThisTo(this, Events::ResourceAdded, OnResourceEvent);
  ConnectThisTo(this, Events::ResourceRemoved, OnResourceEvent);
  ConnectThisTo(this, Events::ResourceModified, OnResourceEvent);
}

LightningPluginSourceManager::~LightningPluginSourceManager()
{
}

void LightningPluginSourceManager::ValidateNewName(Status& status, StringParam name, BoundType* optionalType)
{
  LightningDocumentResource::ValidateNewScriptName(status, name);
}

void LightningPluginSourceManager::ValidateRawName(Status& status, StringParam name, BoundType* optionalType)
{
  LightningDocumentResource::ValidateRawScriptName(status, name);
}

bool LightningPluginSourceManager::IsCompilingPlugins()
{
  return mCompilingPluginCount > 0;
}

void LightningPluginSourceManager::OnResourceEvent(ResourceEvent* event)
{
  LightningPluginSource* resource = Type::DebugOnlyDynamicCast<LightningPluginSource*>(event->EventResource);

  // If the resource was added (may be at load time, or may be the first time of
  // adding in the editor) Only do this if its in the editor with a content item
  if (event->EventId == Events::ResourceAdded && resource->mContentItem != nullptr)
  {
    resource->EditorInitialize();

    // The shared library that we build (dll/so) should be the same name as our
    // source
    String extension = LightningPluginBuilder::GetSharedLibraryExtension(true);
    String sharedLibraryPath =
        FilePath::CombineWithExtension(resource->mContentItem->mLibrary->SourcePath, resource->Name, extension);

    // If the shared library already exists, just make sure the plugin
    // dependencies are up to date
    if (FileExists(sharedLibraryPath) && GetFileSize(sharedLibraryPath) != 0)
      resource->CopyPluginDependenciesOnce();
    // Otherwise, attempt to build it once in release so we can get a working
    // plugin immediately
    else
      resource->CompileRelease();
  }

  if (LightningPluginLibrary* libraryResource = resource->GetLibrary())
  {
    ResourceLibrary* library = resource->mResourceLibrary;
    library->mSwapPlugins[libraryResource].mCompileStatus = LightningCompileStatus::Modified;
    library->PluginsModified();
  }
}

LightningDefineType(LightningPluginLibrary, builder, type)
{
}

String LightningPluginLibrary::GetSharedLibraryPath() const
{
  // If for some reason the 'SharedLibraryPath' was not set on loading, then try
  // and use the resource library location
  if (SharedLibraryPath.Empty())
    return FilePath::CombineWithExtension(
        mResourceLibrary->Location, Name, LightningPluginBuilder::GetSharedLibraryExtension(true));

  return SharedLibraryPath;
}

Resource* LightningPluginLibrary::GetOriginResource() const
{
  Resource* lightningPluginOrigin = GetSource();
  if (lightningPluginOrigin == nullptr)
    lightningPluginOrigin = const_cast<LightningPluginLibrary*>(this);
  return lightningPluginOrigin;
}

LightningPluginSource* LightningPluginLibrary::GetSource() const
{
  return LightningPluginSourceManager::FindOrNull(Name);
}

LightningPluginLibrary::LightningPluginLibrary()
{
}

LightningPluginLibrary::~LightningPluginLibrary()
{
}

HandleOf<Resource> LightningPluginLibraryLoader::LoadFromFile(ResourceEntry& entry)
{
  LightningPluginLibrary* plugin = new LightningPluginLibrary();
  plugin->SharedLibraryPath = entry.FullPath;
  LightningPluginLibraryManager::GetInstance()->AddResource(entry, plugin);
  return plugin;
}

ImplementResourceManager(LightningPluginLibraryManager, LightningPluginLibrary);

LightningPluginLibraryManager::LightningPluginLibraryManager(BoundType* resourceType) : ResourceManager(resourceType)
{
  mCanDuplicate = false;
  mCanReload = true;
  mCanAddFile = false;
  mPreview = false;
  mSearchable = true;
  mExtension = LightningPluginBuilder::GetSharedLibraryExtension(false);
  mNoFallbackNeeded = true;
  mCanCreateNew = false;

  AddLoader("LightningPluginLibrary", new LightningPluginLibraryLoader());

  ConnectThisTo(this, Events::ResourceAdded, OnResourceEvent);
  ConnectThisTo(this, Events::ResourceRemoved, OnResourceEvent);
  ConnectThisTo(this, Events::ResourceModified, OnResourceEvent);
}

LightningPluginLibraryManager::~LightningPluginLibraryManager()
{
}

void LightningPluginLibraryManager::OnResourceEvent(ResourceEvent* event)
{
  LightningPluginLibrary* libraryResource = Type::DebugOnlyDynamicCast<LightningPluginLibrary*>(event->EventResource);
  ResourceLibrary* library = libraryResource->mResourceLibrary;

  if (event->RemoveMode != RemoveMode::None)
  {
    library->mSwapPlugins.Erase(libraryResource);
  }
  else
  {
    library->mSwapPlugins[libraryResource].mCompileStatus = LightningCompileStatus::Modified;
  }
  library->PluginsModified();
}

} // namespace Plasma
