// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// compresses the entire ProjectFolder into an archive at filePath
void ArchiveProjectFile(Cog* projectCog, StringParam filePath)
{
  ProjectSettings* project = projectCog->has(ProjectSettings);
  String projectDirectory = project->ProjectFolder;

  Status status;
  Archive projectArchive(ArchiveMode::Compressing);
  projectArchive.ArchiveDirectory(status, projectDirectory);
  projectArchive.WriteZipFile(filePath);
  Download(filePath);
}

class ArchiveProjectJob : public Job
{
public:
  String mFileName;
  Cog* mProject;

  void Execute()
  {
    ZoneScoped;
    SendBlockingTaskStart("Archiving");

    ArchiveProjectFile(mProject, mFileName);

    SendBlockingTaskFinish();
  }
};

// queues up an ArchiveProjectJob
void StartArchiveJob(StringParam filename, ProjectSettings* project)
{
  Cog* projectCog = project->mOwner;
  PL::gEditor->SaveAll(true);

  ArchiveProjectJob* job = new ArchiveProjectJob();
  job->mProject = projectCog;
  job->mFileName = filename;
  PL::gJobs->AddJob(job);
}

struct ArchiveProjectModal : public SafeId32EventObject
{
  typedef ArchiveProjectModal LightningSelf;

  ArchiveProjectModal(String title, ProjectSettings* project, String fileName, String directory)
  {
    mProject = project;

    const String CallBackEvent = "ArchiveCallback";
    CreateDirectoryAndParents(directory);

    FileDialogConfig* config = FileDialogConfig::Create();
    config->EventName = CallBackEvent;
    config->CallbackObject = this;
    config->Title = title;
    config->AddFilter("Zip", "*.zip");
    config->DefaultFileName = BuildString(fileName, ".zip");
    config->mDefaultSaveExtension = "zip";
    config->StartingDirectory = directory;

    ConnectThisTo(this, CallBackEvent, OnArchiveProjectFile);
    PL::gEngine->has(OsShell)->SaveFile(config);
  }

  void OnArchiveProjectFile(OsFileSelection* event)
  {
    if (event->Success)
      StartArchiveJob(event->Files[0], mProject);
    delete this;
  }

  ProjectSettings* mProject;
};

void ArchiveProject(ProjectSettings* project)
{
    new ArchiveProjectModal("Archive project", project, 
        project->ProjectName, GetUserDocumentsDirectory());
}

void BackupProject(ProjectSettings* project)
{
    String backupDirectory = FilePath::Combine(GetUserDocumentsApplicationDirectory(), "Backups");
    String timeStamp = GetTimeAndDateStamp();
    String fileName = BuildString(project->ProjectName, timeStamp);

    new ArchiveProjectModal("Backup project", project, 
        fileName, backupDirectory);
}

void ExportGame(ProjectSettings* project)
{
  Exporter* exporter = Exporter::GetInstance();
  exporter->ExportGameProject(project->mOwner);
}

void ExportContent(ProjectSettings* project)
{
  // Save all resources and build them so the
  // output directory is up to date
  Editor* editor = PL::gEditor;
  editor->SaveAll(true);

  Exporter* exporter = Exporter::GetInstance();
  exporter->mProjectCog = project->GetOwner();
  exporter->ExportContent(exporter->mDefaultTargets);
}

void ShowProjectFolder(ProjectSettings* project)
{
  Os::ShellOpenDirectory(project->ProjectFolder);
}

void ShowContentOutput(ProjectSettings* project)
{
  String outputPath = project->ProjectContentLibrary->GetOutputPath();
  Os::ShellOpenDirectory(outputPath);
}

void ExportAndPlayGame(ProjectSettings* project)
{
  Exporter* exporter = Exporter::GetInstance();
  exporter->ExportAndPlay(project->mOwner);
}

void BindArchiveCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("ArchiveProject", BindCommandFunction(ArchiveProject));
  commands->AddCommand("BackupProject", BindCommandFunction(BackupProject));

  commands->AddCommand("ExportGame", BindCommandFunction(ExportGame));
  commands->AddCommand("ExportAndPlayGame", BindCommandFunction(ExportAndPlayGame));
  commands->AddCommand("ExportContent", BindCommandFunction(ExportContent));

  commands->AddCommand("ShowProjectFolder", BindCommandFunction(ShowProjectFolder), true);
  commands->AddCommand("ShowContentOutput", BindCommandFunction(ShowContentOutput), true);
}

} // namespace Plasma
