// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{

DefineEvent(InstallStarted);
DefineEvent(InstallCompleted);
DefineEvent(UninstallStarted);
DefineEvent(UninstallCompleted);
DefineEvent(BuildListUpdated);
DefineEvent(TemplateListUpdated);
DefineEvent(NewBuildAvailable);

} // namespace Events

/// Sort versions such that newer builds are first. If the branches are
/// different then sort alphabetically.
struct VersionSorter
{
  // Return true if lhs is "newer" than rhs (first things should return true)
  bool operator()(PlasmaBuild* lhs, PlasmaBuild* rhs)
  {
    BuildId lhsBuildId = lhs->GetBuildId();
    BuildId rhsBuildId = rhs->GetBuildId();
    BuildUpdateState::Enum state = lhsBuildId.CheckForUpdate(rhsBuildId);

    // If the branches or application are different just sort alphabetically
    if (state == BuildUpdateState::DifferentApplication)
      return lhsBuildId.mApplication < rhsBuildId.mApplication;
    if (state == BuildUpdateState::DifferentBranch)
      return lhsBuildId.mBranch < rhsBuildId.mBranch;

    // Otherwise, return true if lhs > rhs (aka the update is to an older build)
    bool updatingToOlder = (state == BuildUpdateState::Older || state == BuildUpdateState::OlderBreaking);
    return updatingToOlder;
  }
};

/// Sort templates by their names.
struct TemplateSorter
{
  bool operator()(TemplateProject* lhs, TemplateProject* rhs)
  {
    // If the sort priority is equal then resort to comparing the names
    if (lhs->GetSortPriority() == rhs->GetSortPriority())
      return lhs->GetDisplayName() < rhs->GetDisplayName();

    // Otherwise just use the sort priority
    return lhs->GetSortPriority() < rhs->GetSortPriority();
  }
};

VersionSelector::VersionSelector(LauncherConfig* config)
{
  mConfig = config;
}

VersionSelector::~VersionSelector()
{
  DeleteObjectsInContainer(mVersions);
  DeleteObjectsInContainer(mOldVersions);
  DeleteObjectsInContainer(mTemplates);
  DeleteObjectsInContainer(mOldTemplates);
  DeleteObjectsInContainer(mReinstallTasks);
}

void VersionSelector::FindInstalledVersions()
{
  // The launcher may come with a pre-packaged build.
  MainConfig* mainConfig = PL::gEngine->GetConfigCog()->has(MainConfig);
  String includedBuilds = FilePath::Combine(mainConfig->SourceDirectory, "Build", "IncludedBuilds");

  if (DirectoryExists(includedBuilds))
    FindInstalledVersions(includedBuilds);

  // Recursively find builds from the root install location
  String rootInstallLocation = GetRootInstallLocation();
  FindInstalledVersions(rootInstallLocation);

  Sort(mVersions.All(), VersionSorter());

  Event e;
  DispatchEvent(Events::BuildListUpdated, &e);
}

void VersionSelector::FindInstalledVersions(StringParam searchPath)
{
  PlasmaPrint("Looking for builds in %s\n", searchPath.c_str());

  // Iterate over the install directory
  FileRange range(searchPath);
  for (; !range.Empty(); range.PopFront())
  {
    FileEntry entry = range.FrontEntry();
    String fullDirName = entry.GetFullPath();
    // We only care about folders
    if (DirectoryExists(fullDirName))
    {
      // Add checksum logic to validate installs here?

      // If this folder contains an exe (a build) then load the version,
      // otherwise recurse.
      String executablePath = FilePath::Combine(fullDirName, GetEditorExecutableFileName());
      if (FileExists(executablePath) == false)
        FindInstalledVersions(fullDirName);
      else
        LoadInstalledBuild(entry.mPath, entry.mFileName);
    }
  }
}

void VersionSelector::LoadInstalledBuild(StringParam directoryPath, StringParam buildFolder)
{
  String fullPath = FilePath::Combine(directoryPath, buildFolder);
  PlasmaPrint("Loading installed build %s\n", fullPath.c_str());

  // By default, parse the build's folder name to
  // attempt to get the build id (mostly for legacy)
  BuildId buildId;
  if (!buildId.ParseRequired(buildFolder))
    return;

  // If this build hasn't already been loaded then we need to create it
  // (we either haven't contacted the server yet or it's not on the server).
  PlasmaBuild* localBuild = mVersionMap.FindValue(buildId, nullptr);
  if (localBuild == nullptr)
  {
    localBuild = new PlasmaBuild();
    localBuild->SetBuildId(buildId);
    
    // Add this build by id and into an array (sorted by id)
    mVersionMap.Insert(buildId, localBuild);
    mVersions.PushBack(localBuild);
  }

  // If the build has a deprecated component then make sure to add the
  // deprecated tag
  if (localBuild->GetDeprecatedInfo(false) != nullptr)
  {
    PlasmaBuildContent* buildInfo = localBuild->GetBuildContent(true);
    buildInfo->AddTag(PlasmaBuild::mDeprecatedTag);
  }

  // Save the builds install location so we know where it is to run it
  localBuild->mInstallLocation = fullPath;
  localBuild->mInstallState = InstallState::Installed;
}

BackgroundTask* VersionSelector::GetServerListing()
{
  return GetReleaseListing(GetEditorFullName());
}

BackgroundTask* VersionSelector::GetLauncherListing()
{
  return GetReleaseListing(GetLauncherFullName());
}

BackgroundTask* VersionSelector::GetReleaseListing(StringParam applicationName)
{
  PlasmaPrint("Requesting launcher build list for %s from server.\n", applicationName.c_str());
  // Start the task to get the version listing
  GetVersionListingTaskJob* job = new GetVersionListingTaskJob(applicationName);
  job->mName = "Version List";
  return PL::gBackgroundTasks->Execute(job, job->mName);
}

void VersionSelector::UpdatePackageListing(GetVersionListingTaskJob* job)
{
  PlasmaPrint("Updating build list.\n");
  // clear out whether or not each version is on the server
  //(so that if one was removed then it will be updated)
  for (uint i = 0; i < mVersions.Size(); ++i)
    mVersions[i]->mOnServer = false;

  // Since versions are sorted by newest to oldest, the first one is the latest
  // version
  BuildId prevNewestBuildId;
  if (!mVersions.Empty())
    prevNewestBuildId = mVersions[0]->GetBuildId();

  // Have to serialize the job's data on the main thread
  job->PopulatePackageList();
  // Add each build (properly merging with local builds)
  for (uint i = 0; i < job->mPackages.Size(); ++i)
  {
    Cog* buildCog = job->mPackages[i];
    // This build must have a meta cog, otherwise the server screwed up
    PlasmaBuildContent* buildContent = buildCog->has(PlasmaBuildContent);
    if (buildContent == nullptr)
      continue;

    // Get the build id of the server's build and use it to find
    // if we have the same build locally installed
    BuildId serverBuildId = buildContent->GetBuildId();
    PlasmaBuild* localStandalone = mVersionMap.FindValue(serverBuildId, nullptr);
    // If this build wasn't locally installed, then we need to create a
    // PlasmaBuild for it
    if (localStandalone == nullptr)
    {
      localStandalone = new PlasmaBuild();
      localStandalone->mMetaCog = buildCog;
      // Since it didn't already exist that means it wasn't installed
      localStandalone->mInstallState = InstallState::NotInstalled;

      mVersions.PushBack(localStandalone);
      mVersionMap.Insert(serverBuildId, localStandalone);
    }
    // Otherwise, this build was already installed so we have to update the meta
    // info
    else
    {
      // Overwrite the local build's  meta with the server's
      Cog* oldMetaCog = localStandalone->mMetaCog;
      localStandalone->mMetaCog = buildCog;

      // If there local build had a meta then transfer certain local properties
      if (oldMetaCog != nullptr)
      {
        // If the local build had a user defined deprecated message then
        // transfer that over to the new meta
        PlasmaBuildDeprecated* oldDeprecatedInfo = oldMetaCog->has(PlasmaBuildDeprecated);
        if (oldDeprecatedInfo != nullptr && !oldDeprecatedInfo->mUserMessage.Empty())
        {
          PlasmaBuildDeprecated* deprecatedInfo = localStandalone->GetDeprecatedInfo(true);
          deprecatedInfo->mUserMessage = oldDeprecatedInfo->mUserMessage;
        }
      }
    }

    // Either way, mark this build as being on the server
    localStandalone->mOnServer = true;

    // If there is a deprecated component then make sure the build is tagged as
    // deprecated
    if (localStandalone->GetDeprecatedInfo(false) != nullptr)
      buildContent->AddTag(PlasmaBuild::mDeprecatedTag);
    // On the flip side, the server could have a build tagged as deprecated
    // without actually having the component (or no message). In this case add
    // the component and set a message if there wasn't one. If the tag says this
    // version is deprecated then mark that it is bad
    if (buildContent->ContainsTag(PlasmaBuild::mDeprecatedTag) == true)
    {
      PlasmaBuildDeprecated* deprecatedInfo = localStandalone->GetDeprecatedInfo(true);
      if (deprecatedInfo->mServerMessage.Empty())
        deprecatedInfo->mServerMessage = "Version was deprecated";
    }
  }

  // Now we know which versions are on the server so remove all of the templates
  // that are no longer on the server (and haven't been downloaded) and then
  // re-populate the hashmap
  mVersionMap.Clear();
  size_t i = 0;
  while (i < mVersions.Size())
  {
    PlasmaBuild* build = mVersions[i];
    if (!build->mOnServer && build->mInstallState == InstallState::NotInstalled)
    {
      // Because people were listening to the builds (they're event objects) we
      // can't safely delete them. Instead just move them to another list that
      // we'll clean-up on close (there shouldn't ever be too many of these
      // unless someone leaves the launcher running for months)
      mOldVersions.PushBack(build);
      mVersions[i] = mVersions.Back();
      mVersions.PopBack();
      continue;
    }

    mVersionMap.Insert(build->GetBuildId(), build);
    ++i;
  }

  Sort(mVersions.All(), VersionSorter());

  Event e;
  DispatchEvent(Events::BuildListUpdated, &e);

  // If there is a newer version available, it'll have a name different than the
  // one we cached before, in that case send out an event so someone knows (and
  // can update Ui or something)
  if (!mVersions.Empty() && prevNewestBuildId != mVersions[0]->GetBuildId())
  {
    Event toSend;
    DispatchEvent(Events::NewBuildAvailable, &toSend);
  }
}

void VersionSelector::AddCustomBuild(StringParam buildPath, bool shouldInstall)
{
  PlasmaPrint("Adding custom user build.\n");
  PlasmaBuild* localBuild = new PlasmaBuild();

  BuildId buildId;
  String buildName = FilePath::GetFileName(buildPath);
  if (!buildId.ParseRequired(buildName))
    return;

  localBuild->SetBuildId(buildId);
  localBuild->mInstallLocation = GetDefaultInstallLocation(buildId);

  // Since it didn't already exist that means it wasn't installed
  localBuild->mInstallState = InstallState::Installing;

  // If we already had the build installed then remove the old one
  // so we can replace it with the new one
  PlasmaBuild* oldBuild = mVersionMap.FindValue(localBuild->GetBuildId(), nullptr);
  if (oldBuild != nullptr)
  {
    mVersions.EraseValue(oldBuild);
    mVersionMap.Erase(localBuild->GetBuildId());
    mOldVersions.PushBack(oldBuild);
  }

  mVersions.PushBack(localBuild);
  mVersionMap.Insert(localBuild->GetBuildId(), localBuild);
  Sort(mVersions.All(), VersionSorter());

  Event e;
  DispatchEvent(Events::BuildListUpdated, &e);

  if (shouldInstall)
  {
    InstallBuildTaskJob* job = new InstallBuildTaskJob();
    job->LoadFromFile(buildPath);
    job->mInstallLocation = localBuild->mInstallLocation;

    // there are several different scenarios that want to install then do
    // something else, because of this just create the task and return it so
    // they can connect to whatever events they want to on the outside
    BackgroundTask* task = PL::gBackgroundTasks->CreateTask(job);
    task->mName = "Latest Install";

    Plasma::Connect(task, Events::BackgroundTaskUpdated, localBuild, &PlasmaBuild::ForwardEvent);
    Plasma::Connect(task, Events::BackgroundTaskCompleted, localBuild, &PlasmaBuild::ForwardEvent);
    Plasma::Connect(task, Events::BackgroundTaskCompleted, localBuild, &PlasmaBuild::InstallCompleted);
    Plasma::Connect(task, Events::BackgroundTaskFailed, localBuild, &PlasmaBuild::ForwardEvent);

    Event toSend;
    localBuild->DispatchEvent(Events::InstallStarted, &toSend);
    task->Execute();
  }
}

bool VersionSelector::InstallLocalTemplateProject(StringParam filePath)
{
  File file;
  file.Open(filePath, FileMode::Read, FileAccessPattern::Random);

  // Try to find a meta file in the zip. Make sure to open the zip to only read
  // the entries so that we don't have to decompress everything.
  Archive archive(ArchiveMode::Decompressing);
  archive.ReadZip(ArchiveReadFlags::Entries, file);

  // Try and extract the meta file from the archive (as well as any preview
  // images/icons if they exist)
  String sku = FilePath::GetFileNameWithoutExtension(filePath);
  String metaFileName = BuildString(sku, ".meta");
  Cog* metaCog = ExtractLocalTemplateMetaAndImages(archive, file, metaFileName, true);
  // If we failed to find a meta file then we need to parse the file name to get
  // the sku and build id
  if (metaCog == nullptr)
  {
    String fileName = FilePath::GetFileNameWithoutExtension(filePath);

    // Find a string that is _VersionNumber, this can include '.' ',' and '-' in
    // order to specify ranges The expected file name is SKU_UserId_BuildId
    // where _UserId is optional.
    Matches matches;
    Regex regex("(\\w+)(_[\\w\\d\\.\\,\\-]+)?_([\\w\\d\\.\\,\\-]+)");
    regex.Search(fileName, matches);

    // If we failed to find a match then notify the user
    if (matches.Empty())
      return false;

    // Create a cog for the meta
    metaCog = PL::gFactory->Create(PL::gEngine->GetEngineSpace(), CoreArchetypes::Empty, 0, nullptr);
    metaCog->ClearArchetype();

    // Create the plasma template component and set the sku and version id from
    // what we parsed
    PlasmaTemplate* plasmaTemplate = HasOrAdd<PlasmaTemplate>(metaCog);
    plasmaTemplate->mSKU = matches[1];
    plasmaTemplate->mVersionId = matches[3];
    // Just default the display name to the sku name
    plasmaTemplate->mDisplayName = plasmaTemplate->mSKU;
    // Parse the ids into a more usable run-time format
    plasmaTemplate->ParseVersionId();
  }
  file.Close();

  PlasmaTemplate* plasmaTemplate = metaCog->has(PlasmaTemplate);

  // Always create the template's output directory
  String fullTemplateName = plasmaTemplate->GetFullTemplateVersionName();
  String templateInstallDirectory = FilePath::Combine(mConfig->GetTemplateInstallPath(), fullTemplateName);
  CreateDirectoryAndParents(templateInstallDirectory);

  // Create the template project from our meta cog so that we can
  // cache information like where this is installed, icon images, etc...
  TemplateProject* currentProject = CreateTemplateProjectFromMeta(metaCog, templateInstallDirectory, String());
  // Make sure we set the local path of where this template is installed right
  // away as several other functions use this
  currentProject->mLocalPath = templateInstallDirectory;

  // Always copy over the ".plasmatemplate" that is being installed.
  //Remove misc. files like the images and meta later?
  String plasmaTemplateFilePathDest = currentProject->GetInstalledTemplatePath();
  CopyFile(plasmaTemplateFilePathDest, filePath);

  // Always save out the meta file
  String metaFilePath = FilePath::CombineWithExtension(templateInstallDirectory, fullTemplateName, ".meta");
  String metaContents = currentProject->SaveMetaFileToString();
  WriteStringRangeToFile(metaFilePath, metaContents);

  // Set the relevant information we loaded and then load the images
  currentProject->mMetaCog = metaCog;
  currentProject->mIsDownloaded = true;
  currentProject->LoadLocalImages();

  // Let everyone know that we've changed our template list
  Event e;
  GetDispatcher()->Dispatch(Events::TemplateListUpdated, &e);
  return true;
}

TemplateProject* VersionSelector::CreateTemplateProjectFromMeta(StringParam metaFilePath, StringParam fullBuildId)
{
  // Load the meta file
  DataTreeLoader loader;
  Status status;
  loader.OpenFile(status, metaFilePath);
  Cog* metaCog = PL::gFactory->CreateFromStream(PL::gEngine->GetEngineSpace(), loader, 0, nullptr);
  return CreateTemplateProjectFromMeta(metaCog, FilePath::GetDirectoryPath(metaFilePath), fullBuildId);
}

TemplateProject* VersionSelector::CreateTemplateProjectFromMeta(Cog* metaCog,
                                                                StringParam localPath,
                                                                StringParam fullBuildId)
{
  // Make sure the meta file contained the necessary info
  if (metaCog == nullptr)
    return nullptr;
  PlasmaTemplate* plasmaTemplate = metaCog->has(PlasmaTemplate);
  if (plasmaTemplate == nullptr)
    return nullptr;

  PlasmaPrint("Found and parsed template: %s\n", localPath.c_str());

  // Check and see if we already had this template
  String key = plasmaTemplate->GetIdString();
  TemplateProject* currentProject = mTemplateMap.FindValue(key, nullptr);
  // If we didn't then create a new project and mark it as not existing on the
  // server
  if (currentProject == nullptr)
  {
    currentProject = new TemplateProject();
    currentProject->mMetaCog = metaCog;
    currentProject->mIsOnServer = false;

    mTemplateMap.Insert(key, currentProject);
    mTemplates.PushBack(currentProject);
  }

  // Either way, we need to set that we loaded this meta file
  // from a local path and then load the icons/preview images
  currentProject->mLocalPath = localPath;
  currentProject->mMetaCog = metaCog;
  currentProject->mIsDownloaded = true;
  currentProject->LoadLocalImages();
  return currentProject;
}

void VersionSelector::FindOnDiskTemplates(PlasmaBuild* selectedBuild)
{
  mOldTemplates.Insert(mOldTemplates.End(), mTemplates.All());
  mTemplates.Clear();
  mTemplateMap.Clear();
  if (selectedBuild)
  {
    String buildTemplates = FilePath::Combine(selectedBuild->mInstallLocation, "LauncherTemplates");
    FindOnDiskTemplatesRecursive(buildTemplates, FilePath::GetFileName(selectedBuild->mInstallLocation));
  }

  String packagedTemplates = FilePath::Combine(GetApplicationDirectory(), "Templates");
  FindOnDiskTemplatesRecursive(packagedTemplates);

  // Check the download directory (for user downloaded templates)
  String dir = mConfig->GetTemplateInstallPath();
  FindOnDiskTemplatesRecursive(dir);
}

void VersionSelector::FindOnDiskTemplatesRecursive(StringParam searchPath, StringParam fullBuildId)
{
  // Iterate over the install directory
  FileRange range(searchPath);
  for (; !range.Empty(); range.PopFront())
  {
    FileEntry entry = range.FrontEntry();
    String fullDirName = entry.GetFullPath();
    if (DirectoryExists(fullDirName))
      FindOnDiskTemplatesRecursive(fullDirName);
    else
    {
      String ext = FilePath::GetExtension(fullDirName);
      if (ext != "meta")
        continue;
      CreateTemplateProjectFromMeta(fullDirName, fullBuildId);
    }
  }
}

BackgroundTask* VersionSelector::GetTemplateListing()
{
  PlasmaPrint("Checking server for project templates.\n");
  GetTemplateListingTaskJob* job = new GetTemplateListingTaskJob(Urls::cApiLauncherTemplates);
  job->mName = "Template List";
  return PL::gBackgroundTasks->Execute(job, job->mName);
}

void VersionSelector::UpdateTemplateListing(GetTemplateListingTaskJob* templates)
{
  PlasmaPrint("Updating template project listing.\n");
  // Mark all templates as not being on the server (so we know which ones to get
  // rid of later)
  for (size_t i = 0; i < mTemplates.Size(); ++i)
    mTemplates[i]->mIsOnServer = false;

  // have to deserialize the job's data on the main thread
  templates->PopulateTemplateList();

  for (size_t i = 0; i < templates->mTemplates.Size(); ++i)
  {
    Cog* templateCog = templates->mTemplates[i];
    PlasmaTemplate* serverPlasmaTemplate = templateCog->has(PlasmaTemplate);
    if (serverPlasmaTemplate == nullptr)
      return;

    String key = serverPlasmaTemplate->GetIdString();
    TemplateProject* localTemplate = mTemplateMap.FindValue(key, nullptr);
    // If we didn't already have a copy of this template then
    // create a project for it and store its information
    if (localTemplate == nullptr)
    {
      localTemplate = new TemplateProject();
      mTemplates.PushBack(localTemplate);
      mTemplateMap.Insert(key, localTemplate);

      localTemplate->mMetaCog = templateCog;
      localTemplate->mIsOnServer = true;
      localTemplate->DownloadIcon(this);
      continue;
    }

    // Otherwise we already had a copy, but this copy could've been from a
    // previous call to the server or it could be a locally downloaded template.
    // If it is locally downloaded then keep that template's information instead
    // of the servers. Otherwise just update in place.
    if (localTemplate->mIsDownloaded)
    {
      PlasmaTemplate* localPlasmaTemplate = localTemplate->GetPlasmaTemplate(true);
      localTemplate->mIsOnServer = true;
      localTemplate->mIsDifferentFromServer = (serverPlasmaTemplate->mDate != localPlasmaTemplate->mDate);
      continue;
    }

    localTemplate->mMetaCog = templateCog;
    localTemplate->mIsOnServer = true;
    localTemplate->DownloadIcon(this);
  }

  // Now we know which versions are on the server so remove all of the templates
  // that are no longer on the server (and haven't been downloaded) and then
  // re-populate the hashmap
  mTemplateMap.Clear();
  size_t i = 0;
  while (i < mTemplates.Size())
  {
    TemplateProject* project = mTemplates[i];
    if (!project->mIsOnServer && !project->mIsDownloaded)
    {
      // Because people were listening to the templates (they're event objects)
      // we can't safely delete them. Instead just move them to another list
      // that we'll clean-up on close (there shouldn't ever be too many of these
      // unless someone leaves the launcher running for months)
      mOldTemplates.PushBack(mTemplates[i]);
      mTemplates[i] = mTemplates.Back();
      mTemplates.PopBack();
      continue;
    }

    String key = project->GetIdString();
    mTemplateMap.Insert(key, project);
    ++i;
  }

  Sort(mTemplates.All(), TemplateSorter());
}

BackgroundTask* VersionSelector::DownloadTemplateProject(TemplateProject* project)
{
  String templateUrl = project->GetTemplateUrl();
  PlasmaPrint("Downloading template project '%s'.\n", templateUrl.c_str());
  DownloadTemplateTaskJob* job = new DownloadTemplateTaskJob(templateUrl, project);

  PlasmaTemplate* plasmaTemplate = project->GetPlasmaTemplate(true);
  job->mTemplateInstallLocation =
      FilePath::Combine(mConfig->GetTemplateInstallPath(), plasmaTemplate->GetFullTemplateVersionName());
  job->mTemplateNameWithoutExtension = FilePath::GetFileNameWithoutExtension(project->GetLocalTemplateFileName());

  return PL::gBackgroundTasks->Execute(job, job->mName);
}

BackgroundTask* VersionSelector::CreateProjectFromTemplate(TemplateProject* project,
                                                           StringParam templateInstallPath,
                                                           StringParam projectInstallPath,
                                                           const BuildId& buildId,
                                                           const HashSet<String>& projectTags)
{
  String templateUrl = project->GetTemplateUrl();
  DownloadAndCreateTemplateTaskJob* job = new DownloadAndCreateTemplateTaskJob(templateUrl, project);

  job->mTemplateInstallLocation = templateInstallPath;
  job->mTemplateNameWithoutExtension = project->GetLocalTemplateFileName();
  job->mProjectInstallLocation = FilePath::GetDirectoryPath(projectInstallPath);
  job->mProjectName = FilePath::GetFileNameWithoutExtension(projectInstallPath);
  job->mBuildId = buildId;
  job->mName = "Template Download";
  job->mProjectTags = projectTags;

  return PL::gBackgroundTasks->Execute(job, job->mName);
}

BackgroundTask* VersionSelector::DownloadTemplateIcon(TemplateProject* project)
{
  String iconUrl = project->GetIconUrl();
  DownloadImageTaskJob* job = new DownloadImageTaskJob(iconUrl);
  job->mName = "Download Preview";

  return PL::gBackgroundTasks->Execute(job, job->mName);
}

BackgroundTask* VersionSelector::DownloadTemplatePreviews(TemplateProject* project)
{
  //Update to get the remaining preview icons later...
  return nullptr;
}

BackgroundTask* VersionSelector::DownloadDeveloperNotes()
{
  DownloadTaskJob* job = new DownloadTaskJob(Urls::cApiDevNotes, cCacheSeconds);
  job->mName = "Download DevUpdates";

  return PL::gBackgroundTasks->Execute(job, job->mName);
}

Cog* VersionSelector::ExtractLocalTemplateMetaAndImages(Archive& archive,
                                                        File& file,
                                                        StringParam metaFileName,
                                                        bool extractImages)
{
  // Find the meta file as a file with the same name as the sku
  ArchiveEntry* metaFileEntry = FindEntryFromArchive(archive, metaFileName);
  if (metaFileEntry == nullptr)
  {
    // Maybe find another meta file if this failed?
    return nullptr;
  }

  // Load the meta file's data into a string
  archive.DecompressEntry(*metaFileEntry, file);
  String metaContents((cstr)metaFileEntry->Full.Data, metaFileEntry->Full.Size);

  Status status;
  DataTreeLoader loader;
  loader.OpenBuffer(status, metaContents);
  Cog* metaCog = PL::gFactory->CreateFromStream(PL::gEngine->GetEngineSpace(), loader, 0, nullptr);
  // Make sure we successfully created the cog and that it has the required
  // component
  if (metaCog == nullptr)
    return nullptr;
  PlasmaTemplate* plasmaTemplate = metaCog->has(PlasmaTemplate);
  if (plasmaTemplate == nullptr)
    return nullptr;

  // If we want to display previews from a template we need to extract them from
  // the archive and put them in the template folder where we can easily load
  // them
  if (extractImages)
  {
    // Make sure the template's directory exists
    String templateInstallDirectory =
        FilePath::Combine(mConfig->GetTemplateInstallPath(), plasmaTemplate->GetFullTemplateVersionName());
    CreateDirectoryAndParents(templateInstallDirectory);

    // Find the icon file in the archive specified by the meta file
    ArchiveEntry* iconEntry = FindEntryFromArchive(archive, plasmaTemplate->mIconUrl);
    if (iconEntry != nullptr)
    {
      // Decompress the entry and save it out to the specified relative path
      archive.DecompressEntry(*iconEntry, file);
      String iconFilePathDest = FilePath::Combine(templateInstallDirectory, plasmaTemplate->mIconUrl);
      WriteToFile(iconFilePathDest.c_str(), iconEntry->Full.Data, iconEntry->Full.Size);
    }
  }
  return metaCog;
}

ArchiveEntry* VersionSelector::FindEntryFromArchive(Archive& archive, StringParam fileName, bool ignorePath)
{
  // Find the specified file's entry
  for (Archive::range range = archive.GetEntries(); !range.Empty(); range.PopFront())
  {
    ArchiveEntry& entry = range.Front();
    String entryName = entry.Name;
    // Strip the file path from the entry if desired
    if (ignorePath)
      entryName = FilePath::GetFileName(entryName);

    if (entryName == fileName)
      return &entry;
  }
  return nullptr;
}

BackgroundTask* VersionSelector::InstallVersion(PlasmaBuild* standalone)
{
  if (standalone->mInstallState == InstallState::Installed || standalone->mInstallState == InstallState::Installing)
    return nullptr;

  PlasmaPrint("Installing build '%s'\n", standalone->GetDebugIdString().c_str());

  // mark the version as now being installed (maybe have this happen on the
  // outside in case something failed?)
  standalone->mInstallState = InstallState::Installing;

  // build the task to start the install
  BuildId buildId = standalone->GetBuildId();
  String downloadUrl = standalone->GetDownloadUrl();
  DownloadStandaloneTaskJob* job = new DownloadStandaloneTaskJob(downloadUrl);

  // Get the install path for this build
  standalone->mInstallLocation = GetDefaultInstallLocation(buildId);

  job->mInstallLocation = standalone->mInstallLocation;

  // there are several different scenarios that want to install then do
  // something else, because of this just create the task and return it so
  // they can connect to whatever events they want to on the outside
  BackgroundTask* task = PL::gBackgroundTasks->CreateTask(job);
  task->mName = "Latest Install";

  Plasma::Connect(task, Events::BackgroundTaskUpdated, standalone, &PlasmaBuild::ForwardEvent);
  Plasma::Connect(task, Events::BackgroundTaskCompleted, standalone, &PlasmaBuild::ForwardEvent);
  Plasma::Connect(task, Events::BackgroundTaskCompleted, standalone, &PlasmaBuild::InstallCompleted);
  Plasma::Connect(task, Events::BackgroundTaskFailed, standalone, &PlasmaBuild::ForwardEvent);
  ConnectThisTo(standalone, Events::InstallStarted, OnForwardEvent);
  ConnectThisTo(standalone, Events::InstallCompleted, OnForwardEvent);

  Event toSend;
  standalone->DispatchEvent(Events::InstallStarted, &toSend);
  task->Execute();

  return task;
}

BackgroundTask* VersionSelector::DeleteVersion(PlasmaBuild* standalone)
{
  PlasmaPrint("Uninstalling build '%s'\n", standalone->GetDebugIdString().c_str());
  standalone->mInstallState = InstallState::Uninstalling;

  // If this build wasn't on the server then we need to move it to the list
  // of unavailable builds (we don't know how to re-install it)
  if (standalone->mOnServer == false)
  {
    mVersions.EraseValue(standalone);
    mVersionMap.Erase(standalone->GetBuildId());

    mOldVersions.PushBack(standalone);
  }

  String installLocation = GetInstallLocation(standalone);
  // Delete the directory of the build. Also recursively delete empty
  // parent directories up to the root install location.
  DeleteDirectoryJob* job = new DeleteDirectoryJob(installLocation, GetRootInstallLocation(), true);

  BackgroundTask* task = PL::gBackgroundTasks->CreateTask(job);
  task->mName = "DeleteVersion";

  Plasma::Connect(task, Events::BackgroundTaskUpdated, standalone, &PlasmaBuild::ForwardEvent);
  Plasma::Connect(task, Events::BackgroundTaskCompleted, standalone, &PlasmaBuild::ForwardEvent);
  Plasma::Connect(task, Events::BackgroundTaskCompleted, standalone, &PlasmaBuild::UninstallCompleted);
  Plasma::Connect(task, Events::BackgroundTaskFailed, standalone, &PlasmaBuild::ForwardEvent);

  Event toSend;
  standalone->DispatchEvent(Events::UninstallStarted, &toSend);
  task->Execute();

  return task;
}

bool VersionSelector::CheckForRunningBuild(PlasmaBuild* build)
{
  String buildInstallPath = GetInstallExePath(build);

  Array<ProcessInfo> processes;
  GetProcesses(processes);
  // If we find any processes that match the exe path for the given build then
  // return true
  for (size_t i = 0; i < processes.Size(); ++i)
  {
    if (processes[i].mProcessPath == buildInstallPath)
    {
      return true;
    }
  }

  return false;
}

void VersionSelector::ForceCloseRunningBuilds(PlasmaBuild* build)
{
  String buildInstallPath = GetInstallExePath(build);

  Array<ProcessInfo> processes;
  GetProcesses(processes);
  // If we find any processes that match the exe path for the given build then
  // kill it
  for (size_t i = 0; i < processes.Size(); ++i)
  {
    if (processes[i].mProcessPath == buildInstallPath)
    {
      KillProcess(processes[i].mProcessId);
    }
  }
  Os::Sleep(100);
}

void VersionSelector::ForceUpdateAllBuilds()
{
  // Copy all of the builds into an array beforehand as uninstalling a build
  // could possibly remove it from the array we're iterating through otherwise
  Array<PlasmaBuild*> buildsToRemove = mVersions;
  for (size_t i = 0; i < buildsToRemove.Size(); ++i)
  {
    PlasmaBuild* build = buildsToRemove[i];
    if (build->mInstallState == InstallState::NotInstalled)
      continue;

    mReinstallTasks.PushBack(new ReinstallHelper(build, this));
  }
}

BackgroundTask* VersionSelector::DownloadPatchLauncherUpdate(StringParam url)
{
  PlasmaPrint("Downloading launcher patch update.\n");

  String majorVersionIdStr = ToString(GetMajorVersion());
  String launcherFolderName =
      FilePath::Combine(GetUserLocalDirectory(), BuildString("PlasmaLauncher_", majorVersionIdStr, ".0"));
  DownloadLauncherPatchInstallerJob* job = new DownloadLauncherPatchInstallerJob(url, launcherFolderName);
  job->mName = "Download Patch Installer";

  return PL::gBackgroundTasks->Execute(job, job->mName);
}

BackgroundTask* VersionSelector::DownloadMajorLauncherUpdate(StringParam url)
{
  PlasmaPrint("Downloading launcher major update.\n");

  DownloadLauncherMajorInstallerJob* job = new DownloadLauncherMajorInstallerJob(url);
  job->mName = "Download Major Installer";

  return PL::gBackgroundTasks->Execute(job, job->mName);
}

PlasmaBuild* VersionSelector::FindExactVersion(CachedProject* cachedProject)
{
  // if no project file was selected then don't do anything
  if (cachedProject == nullptr)
    return nullptr;

  // Find a build for this project given its build id (full match). If we only
  // display the preferred platform then always try to find the current
  // launcher's platform build.
  BuildId buildId = cachedProject->GetBuildId();
  buildId.SetToThisPlatform();
  // Allows us to push debug builds for testing and builds without configs
  buildId.mConfig = "Release"; 

  // If we find a matching build then return it
  PlasmaBuild* matchingBuild = mVersionMap.FindValue(buildId, nullptr);
  if (matchingBuild != nullptr)
    return matchingBuild;

  // Otherwise there's a bit of a legacy problem here. Builds previously didn't
  // store the changeset as part of it's id. Now that they do, old projects may
  // not have this field saved. If they don't then the hash of BuildId won't
  // resolve properly even though the build may exist. Do a linear search
  // instead to fix these legacy projects. After loading them they should
  // properly store the changeset and resolve quickly next time.
  for (size_t i = 0; i < mVersions.Size(); ++i)
  {
    PlasmaBuild* build = mVersions[i];
    if (build->GetBuildId().CompareBuilds(buildId))
      return build;
  }
  return nullptr;
}

PlasmaBuild* VersionSelector::GetLatestBuild()
{
  HashSet<String> rejectionTags;
  if (mConfig->mShowDevelopmentBuilds == false)
    rejectionTags.Insert("Develop");
  HashSet<String> legacyTags;

  PlasmaBuildTagPolicy policy(PL::gEngine->GetConfigCog()->has(LauncherConfig));
  Array<PlasmaBuild*> results;
  TagSet resultTags;
  FilterDataSetWithTags(legacyTags, rejectionTags, String(), mVersions, results, resultTags, policy);

  // We need to find the first build that satisfies the user tags (filtered results are still sorted)
  forRange (PlasmaBuild* build, results)
  {
    // Ignore builds that aren't on the server
    if (build->mOnServer)
      return build;
  }

  return nullptr;
}

size_t VersionSelector::GetInstalledBuildsCount() const
{
  size_t count = 0;
  forRange (PlasmaBuild* build, mVersions.All())
  {
    if (build->mInstallState == InstallState::Installed)
      ++count;
  }

  return count;
}

void VersionSelector::OnForwardEvent(Event* e)
{
  DispatchEvent(e->EventId, e);
}

void VersionSelector::FindVersionsWithTags(
    TagSet& activeTags, TagSet& rejectionTags, StringParam activeSearch, Array<PlasmaBuild*>& results, TagSet& resultTags)
{
  PlasmaBuildTagPolicy policy;
  FilterDataSetWithTags(activeTags, rejectionTags, activeSearch, mVersions, results, resultTags, policy);
}

void VersionSelector::FindTemplateWithTags(const BuildId& buildId,
                                           TagSet& activeTags,
                                           TagSet& rejectionTags,
                                           StringParam activeSearch,
                                           Array<TemplateProject*>& results,
                                           TagSet& resultTags)
{
  TemplatePackageTagPolicy policy;
  policy.mBuildId = buildId;
  FilterDataSetWithTags(activeTags, rejectionTags, activeSearch, mTemplates, results, resultTags, policy);

  // There's a chance that two projects with the same SKU could match the given
  // build. This typically happens with an installed template (such as the ones
  // shipped with the launcher's installer). In this case we want to only
  // display the build that has the best matching range.
  HashMap<String, TemplateProject*> uniqueTemplates;
  for (size_t i = 0; i < results.Size(); ++i)
  {
    TemplateProject* currentProject = results[i];
    PlasmaTemplate* currentTemplate = currentProject->GetPlasmaTemplate(false);
    // If we haven't seen a project with this SKU already then add this template
    // project as the best result for now
    if (!uniqueTemplates.ContainsKey(currentTemplate->mSKU))
    {
      uniqueTemplates.Insert(currentTemplate->mSKU, currentProject);
      continue;
    }

    // Log a warning
    PlasmaPrint("Project template conflict of SKU '%s'. Picking latest version.\n", currentTemplate->mSKU.c_str());
    // Get the previous template project for this SKU and determine which one is
    // a more exact range
    PlasmaTemplate* previousTemplate = uniqueTemplates[currentTemplate->mSKU]->GetPlasmaTemplate(false);
    if (currentTemplate->IsMoreExactRangeThan(buildId, previousTemplate))
      uniqueTemplates[currentTemplate->mSKU] = currentProject;
  }

  results.Clear();
  results.Insert(results.End(), uniqueTemplates.Values());
}

WarningLevel::Enum VersionSelector::CheckVersionForProject(PlasmaBuild* standalone,
                                                           CachedProject* cachedProject,
                                                           String& warningString,
                                                           bool warnForUpgrading)
{
  // If the build was marked bad for any reason then it is a sever warning level
  if (standalone->IsBad() == true)
  {
    warningString = BuildString("Are you sure? ", standalone->GetDeprecatedString());
    return WarningLevel::Severe;
  }

  BuildId buildId = standalone->GetBuildId();
  BuildId projectId = cachedProject->GetBuildId();
  BuildUpdateState::Enum updateState = projectId.CheckForUpdate(buildId);

  if (updateState == BuildUpdateState::DifferentApplication)
  {
    warningString = "Changing to a different application. Are you sure?";
    return WarningLevel::Severe;
  }
  // If we cross build branches
  else if (updateState == BuildUpdateState::DifferentBranch)
  {
    warningString = "Changing to a different branch. Are you sure?";
    return WarningLevel::Basic;
  }
  // If we try to go to an older version (breaking changes older as well)
  else if (updateState == BuildUpdateState::Older || updateState == BuildUpdateState::OlderBreaking)
  {
    warningString = "Reverting to an old version. Are you sure?";
    return WarningLevel::Basic;
  }
  // If we try to update past a major version
  else if (updateState == BuildUpdateState::NewerBreaking)
  {
    warningString = "Upgrading past breaking changes. Are you sure?";
    return WarningLevel::Basic;
  }
  // If we update but no breaking changes should be included.
  else if (updateState == BuildUpdateState::Newer)
  {
    if (warnForUpgrading)
    {
      warningString = "Going to a newer version. This may include breaking "
                      "changes. Are you sure?";
      return WarningLevel::Basic;
    }
  }

  return WarningLevel::None;
}

void VersionSelector::RunProject(PlasmaBuild* standalone, StringParam projectPath)
{
  // get the location to plasma
  String executablePath = GetInstallExePath(standalone);
  String workingDirectory = GetInstallLocation(standalone);

  PlasmaPrint("Running project '%s' with build '%s'\n", projectPath.c_str(), executablePath.c_str());

  // call plasma with our project file
  String commandLine = String::Format("-file \"%s\"", projectPath.c_str());
  Os::ShellOpenApplicationWithWorkingDirectory(executablePath, commandLine, workingDirectory);
}

void VersionSelector::RunProject(PlasmaBuild* standalone, CachedProject* cachedProject)
{
  String projectFilePath = cachedProject->GetProjectPath();
  RunProject(standalone, projectFilePath);
}

void VersionSelector::RunNewProject(PlasmaBuild* standalone, StringParam projectName, Cog* configCog)
{
  // get the location to plasma
  String executablePath = GetInstallExePath(standalone);

  PlasmaPrint("Running project '%s' with build '%s'\n", projectName.c_str(), executablePath.c_str());

  String commandLine = "-newProject";
  // if a project name was specified then set the name of the new project to run
  if (projectName.Empty() == false)
    commandLine = String::Format("-newProject \"%s\"", projectName.c_str());

  Os::ShellOpenApplication(executablePath, commandLine);
}

void VersionSelector::MarkVersionValid(PlasmaBuild* build)
{
  PlasmaBuildDeprecated* deprecatedInfo = build->GetDeprecatedInfo(false);
  if (deprecatedInfo == nullptr)
    return;

  deprecatedInfo->GetOwner()->RemoveComponent(deprecatedInfo);
}

void VersionSelector::MarkVersionInvalid(PlasmaBuild* build, StringParam userDescription)
{
  PlasmaBuildDeprecated* deprecatedInfo = build->GetDeprecatedInfo(true);
  deprecatedInfo->mUserMessage = userDescription;
}

String VersionSelector::GetRootInstallLocation()
{
  return mConfig->GetBuildsInstallPath();
}

String VersionSelector::GetInstallExePath(PlasmaBuild* build)
{
  // get the location to plasma
  String installLocation = GetInstallLocation(build);
  return FilePath::Combine(installLocation, GetEditorExecutableFileName());
}

String VersionSelector::GetInstallLocation(PlasmaBuild* standalone)
{
  return standalone->mInstallLocation;
}

String VersionSelector::GetDefaultInstallLocation(const BuildId& buildId)
{
  return FilePath::Combine(GetRootInstallLocation(), buildId.GetFullId());
}

String VersionSelector::GetMarkedBadPath(PlasmaBuild* standalone)
{
  String markedBadPath = FilePath::CombineWithExtension(GetInstallLocation(standalone), "VersionMarkedBad", ".txt");
  return markedBadPath;
}

ReinstallHelper::ReinstallHelper(PlasmaBuild* build, VersionSelector* versionSelector)
{
  mBuild = build;
  mVersionSelector = versionSelector;

  mVersionSelector->ForceCloseRunningBuilds(build);
  BackgroundTask* task = mVersionSelector->DeleteVersion(build);
  ConnectThisTo(task, Events::BackgroundTaskCompleted, OnReinstall);
}

void ReinstallHelper::OnReinstall(Event* e)
{
  mVersionSelector->InstallVersion(mBuild);
}

} // namespace Plasma
