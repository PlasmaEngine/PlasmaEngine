// MIT Licensed (see LICENSE.md).
#pragma once

#include "LightningScript.hpp"

namespace Plasma
{
class BackgroundTask;
class UpdateEvent;
class LightningPluginConfig;
class LightningPluginLibrary;
class BackgroundTaskEvent;

class LightningPluginSource : public DataResource
{
public:
  LightningDeclareType(LightningPluginSource, TypeCopyMode::ReferenceType);

  LightningPluginSource();
  ~LightningPluginSource();

  // Resource interface
  void Serialize(Serializer& stream) override;
  void EditorInitialize();

  void OnEngineUpdate(UpdateEvent* event);

  String GetContentDirectory();
  String GetCodeDirectory();
  String GetVersionsDirectory();
  String GetCurrentVersionDirectory();
  LightningPluginLibrary* GetLibrary() const;

  // Copies the library and header files to the plugin shared directory
  // We pretty much litter this function in every call to open the plugin (ide,
  // folder, upon loading, etc)
  void ForceCopyPluginDependencies();
  void CopyPluginDependencies();
  void CopyPluginDependenciesOnce();
  void WriteCurrentVersionFile();

  void OpenDirectory();
  void OpenIde();
  void CompileDebug();
  void CompileRelease();
  void CompileConfiguration(StringParam configuration);
  void Clean();
  void InstallIdeTools();

  void OnCompilationCompleted(BackgroundTaskEvent* e);

  bool CheckIdeAndInformUser();
  bool IsIdeInstalled();
  void MarkAttemptedIdeToolsInstAll();

  LightningPluginConfig* GetConfig();

  // Internals
  BackgroundTask* mCompileTask;

  // This counter starts at 0, and if the user attempts to open the IDE
  // without installing the IDE tools, then we launch the IDE tools installer
  // We have to wait for that installer to close, however the installer
  // seems to run itself multiple times (perhaps elevating permissions)
  // So we need a counter to be sure the installer is finished
  static const int InstallCounter = 20;
  int mOpenIdeAfterToolsInstallCounter;
};

class LightningPluginSourceLoader : public ResourceLoader
{
public:
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override;
  void ReloadFromFile(Resource* resource, ResourceEntry& entry) override;
};

class LightningPluginSourceManager : public ResourceManager
{
public:
  DeclareResourceManager(LightningPluginSourceManager, LightningPluginSource);

  LightningPluginSourceManager(BoundType* resourceType);
  ~LightningPluginSourceManager();

  // ResourceManager Interface
  void ValidateNewName(Status& status, StringParam name, BoundType* optionalType) override;
  void ValidateRawName(Status& status, StringParam name, BoundType* optionalType) override;

  void OnResourceEvent(ResourceEvent* event);

  bool IsCompilingPlugins();

  // Internal
  // How many plugins we're currently compiling
  size_t mCompilingPluginCount;
};

class LightningPluginLibrary : public LightningLibraryResource
{
public:
  LightningDeclareType(LightningPluginLibrary, TypeCopyMode::ReferenceType);

  LightningPluginLibrary();
  ~LightningPluginLibrary();

  String SharedLibraryPath;
  String GetSharedLibraryPath() const override;
  Resource* GetOriginResource() const override;

  LightningPluginSource* GetSource() const;
};

class LightningPluginLibraryLoader : public ResourceLoader
{
public:
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override;
};

class LightningPluginLibraryManager : public ResourceManager
{
public:
  DeclareResourceManager(LightningPluginLibraryManager, LightningPluginLibrary);

  LightningPluginLibraryManager(BoundType* resourceType);
  ~LightningPluginLibraryManager();

  void OnResourceEvent(ResourceEvent* event);
};

} // namespace Plasma
