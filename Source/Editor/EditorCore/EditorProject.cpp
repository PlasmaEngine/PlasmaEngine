// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(LauncherDebuggerCommunicationCompleted);
DefineEvent(LauncherDebuggerCommunicationFailed);
} // namespace Events

void LoadProject(Editor* editor, Cog* projectCog, StringParam path, StringParam projectFile)
{
  PlasmaPrint("Loading project '%s'\n", projectFile.c_str());

  if (editor->mProject)
    UnloadProject(editor, editor->mProject);

  PL::gEditor->mProject = projectCog;

  HasOrAdd<EditorConfig>(PL::gEditor->mConfig)->EditingProject = projectFile;

  // Save what project was opened
  SaveConfig();

  ProjectSettings* project = projectCog->has(ProjectSettings);
  project->ContentFolder = FilePath::Combine(path, "Content");
  project->EditorContentFolder = FilePath::Combine(path, "EditorContent");
  project->ProjectFolder = path;
  project->ProjectFile = projectFile;

  // Set the Platform Title Bar.
  editor->mMainWindow->SetTitle(BuildString(GetOrganization(), " ", GetApplicationName(), " - ", project->ProjectName));

  // Set the store name based on the project name.
  ObjectStore::GetInstance()->SetStoreName(project->ProjectName);

  String projectFolder = project->ProjectFolder;

  // Load shared content libraries if present
  if (SharedContent* sharedContent = projectCog->has(SharedContent))
  {
    forRange (ContentLibraryReference libraryRef, sharedContent->ExtraContentLibraries.All())
    {
      String libraryName = libraryRef.mContentLibraryName;
      String contentFolder = FilePath::Combine(projectFolder, libraryName);
      Status loadContentLibrary;
      ContentLibrary* contentLibrary =
          PL::gContentSystem->LibraryFromDirectory(loadContentLibrary, libraryName, contentFolder);
      if (contentLibrary)
      {
        Status status;
        PL::gContentSystem->BuildLibrary(status, contentLibrary, true);
      }
      else
      {
        DoNotifyWarning("Missing Library",
                        String::Format("Failed to find shared content library %s", libraryName.c_str()));
      }
    }
  }

  EditorSettings* engineEditorSettings = HasOrAdd<EditorSettings>(PL::gEditor->mConfig);

  // Load content package of project
  Status loadContentLibrary;
  
  ContentLibrary* projectLibrary =
      PL::gContentSystem->LibraryFromDirectory(loadContentLibrary, project->ProjectName, project->ContentFolder);
  
  /// Store the library on the project
  project->ProjectContentLibrary = projectLibrary;

  Status status;
  PL::gContentSystem->BuildLibrary(status, projectLibrary, true);

  //Load Extra content libraries
  project->ExtraContentLibraries = Array<ContentLibrary*>();
  forRange (String library, project->ExtraLibraries.All())
  {    
    String libPath = FilePath::Combine(project->ProjectFolder, library);
      
    ContentLibrary* lib =
      PL::gContentSystem->LibraryFromDirectory(loadContentLibrary, library, libPath);
    project->ExtraContentLibraries.PushBack(lib);
  }

  forRange (ContentLibrary* library, project->ExtraContentLibraries.All())
  {
    Status s;
    PL::gContentSystem->BuildLibrary(s, library, true);
  }
  
  // Always select the first tool
  if (editor->Tools)
    editor->Tools->SelectToolIndex(0);

  PL::gEditor->ProjectLoaded();
}

void UnloadProject(Editor* editor, Cog* projectCog)
{
  ProjectSettings* project = projectCog->has(ProjectSettings);

  PlasmaPrint("Unloading project %s\n", project->ProjectName.c_str());

  ResourceSystem* resourceSystem = PL::gResources;

  // Send Event so other controls can clean up state
  Event event;
  editor->DispatchEvent(Events::UnloadProject, &event);

  // Clear Library View
  editor->mLibrary->View(nullptr, nullptr);

  // Clear Active Selection
  editor->mSelection->Clear();

  // Clear Undo history
  editor->mQueue->ClearAll();

  // Clear Property View
  editor->mMainPropertyView->HardReset();

  // Unload resources
  if (ResourceLibrary* projectLibrary = project->ProjectResourceLibrary)
    projectLibrary->Unload();

  forRange (ResourceLibrary* library, project->SharedResourceLibraries.All())
    library->Unload();

  // Send Out Editor Close

  // Destroy the edit game session
  editor->mEditGame.SafeDestroy();
  editor->mProject = nullptr;
  editor->mProjectLibrary = nullptr;

  projectCog->Destroy();

  // Always select the first tool
  if (editor->Tools)
    editor->Tools->SelectToolIndex(0);
}

bool OpenProjectFile(StringParam filename)
{
  // File check
  if (!FileExists(filename))
  {
    DoNotifyError("Failed to load project.", String::Format("Project file not found '%s'", filename.c_str()));
    return false;
  }

  // Load the project object
  Cog* projectCog =
      PL::gFactory->Create(PL::gEngine->GetEngineSpace(), filename, CreationFlags::ProxyComponentsExpected, nullptr);
  if (projectCog == nullptr)
  {
    DoNotifyError("Failed to load project.", "Project file invalid.");
    return false;
  }
  // Prevent components from being added or removed from the project cog
  projectCog->mFlags.SetFlag(CogFlags::ScriptComponentsLocked);

  ProjectSettings* project = projectCog->has(ProjectSettings);
  if (project == nullptr)
    return false;

  // Begin the loading project
  String projectFolder = FilePath::GetDirectoryPath(filename);
  LoadProject(PL::gEditor, projectCog, projectFolder, filename);
  return true;
}

// Project Commands

void RunLauncherOpenProject(StringParam eventName)
{
  // See if the composite is already open
  Widget* widget = PL::gEditor->mManager->FindWidget(LightningTypeId(LauncherOpenProjectComposite)->Name);
  LauncherOpenProjectComposite* launcherComposite = (LauncherOpenProjectComposite*)widget;
  // if not create it
  if (launcherComposite == nullptr)
  {
    launcherComposite = new LauncherOpenProjectComposite(PL::gEditor);
    PL::gEditor->AddManagedWidget(launcherComposite, DockArea::Floating, false);
  }
  // run the given event name (to communicate with the launcher)
  launcherComposite->SendEvent(eventName);
}

void OpenProject()
{
  RunLauncherOpenProject(Events::LauncherOpenRecentProjects);
}

void ReloadProject()
{
  Cog* projectCog = PL::gEditor->mProject;
  if (projectCog == nullptr)
    return;

  ProjectSettings* project = projectCog->has(ProjectSettings);
  OpenProjectFile(project->ProjectFile);
}

void NewProject()
{
  RunLauncherOpenProject(Events::LauncherNewProject);
}

void SaveProject()
{
  Editor* editor = PL::gEditor;
  editor->SaveAll(true);

  // Take a screen shot if it's enabled
  if (Cog* projectCog = editor->mProject)
  {
    ProjectSettings* project = projectCog->has(ProjectSettings);
    if (project->AutoTakeProjectScreenshot)
      editor->TakeProjectScreenshot();
  }
}

void BindProjectCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("OpenProject", BindCommandFunction(OpenProject));
  commands->AddCommand("ReloadProject", BindCommandFunction(ReloadProject));
  commands->AddCommand("NewProject", BindCommandFunction(NewProject));
  commands->AddCommand("SaveProject", BindCommandFunction(SaveProject));
}

LightningDefineType(LauncherOpenProjectComposite, builder, type)
{
}

LauncherOpenProjectComposite::LauncherOpenProjectComposite(Composite* parent) : Composite(parent)
{
  mSocket = nullptr;
  mName = LightningVirtualTypeId(this)->Name;
}

LauncherOpenProjectComposite::~LauncherOpenProjectComposite()
{
  delete mSocket;
}

void LauncherOpenProjectComposite::SendEvent(StringParam eventType)
{
  mEventType = eventType;
  // delete the old socket
  if (mSocket != nullptr)
    delete mSocket;
  mSocket = nullptr;

  CommunicateWithLauncher();
}

void LauncherOpenProjectComposite::FailedToOpenLauncher()
{
  String message = String::Format("Couldn't find '%s'. Please "
                                  "download the installer from %s.",
                                  GetLauncherExecutableFileName().c_str(),
                                  Urls::cUserLauncherDownloads);
  DoNotifyError("Launcher not found", message);
}

bool LauncherOpenProjectComposite::RunLauncherExe(StringParam exePath)
{
  String cmd;
  if (mEventType == Events::LauncherOpenRecentProjects)
    cmd = BuildString("-", LauncherStartupArguments::Names[LauncherStartupArguments::Projects]);
  else if (mEventType == Events::LauncherNewProject)
    cmd = BuildString("-", LauncherStartupArguments::Names[LauncherStartupArguments::New]);

  // If the debugger is attached tell the launcher to run in debugger mode
  if (Os::IsDebuggerAttached())
    cmd = BuildString(cmd, " -", LauncherStartupArguments::Names[LauncherStartupArguments::DebuggerMode]);

  return Os::ShellOpenApplication(exePath, cmd);
}

bool LauncherOpenProjectComposite::RunFromInstalledPath()
{
  String installExe = Os::GetInstalledExecutable(sPlasmaOrganization, sLauncherName, sLauncherGuid);
  if (installExe.Empty())
    return false;

  return RunLauncherExe(installExe);
}

void LauncherOpenProjectComposite::CommunicateWithLauncher()
{
  mSocket = new TcpSocket(Protocol::Events | Protocol::Chunks, "Launcher");

  ConnectThisTo(mSocket, Events::ConnectionCompleted, OnConnectionCompleted);
  ConnectThisTo(mSocket, Events::ConnectionFailed, OnConnectionFailed);
  mSocket->Connect("localhost", LauncherCommunicationEvent::DesiredPort);
}

void LauncherOpenProjectComposite::OnConnectionCompleted(Event* e)
{
  // send a message to the launcher telling it to show the new or recent
  // projects ui
  LauncherCommunicationEvent toSend;
  if (Os::IsDebuggerAttached())
    toSend.mExtraData = LauncherStartupArguments::Names[LauncherStartupArguments::DebuggerMode];
  mSocket->SendToAll(mEventType, &toSend);

  mSocket->Close();
}

String GetLauncherLocationFromLauncherConfig()
{
  auto launcherConfig = LoadRemoteConfig(sPlasmaOrganization, sLauncherName);
  if (launcherConfig == nullptr)
    return String();

  auto versionConfig = launcherConfig->has(LauncherConfig);
  if (versionConfig == nullptr)
    return String();

  if (versionConfig->mLauncherLocation.Empty() || !FileExists(versionConfig->mLauncherLocation))
    return String();

  String location = versionConfig->mLauncherLocation;
  launcherConfig->Destroy();
  return location;
}

void LauncherOpenProjectComposite::OnConnectionFailed(Event* e)
{
  // Try to run from the installed location, if not open the old dialog
  if (!RunFromInstalledPath())
  {
    String launcherPath = GetLauncherLocationFromLauncherConfig();
    if (!launcherPath.Empty() && RunLauncherExe(launcherPath))
      return;

    // otherwise the launcher doesn't seem to exist so run the old open/new
    // dialog
    FailedToOpenLauncher();
  }

  mSocket->Close();
}

LightningDefineType(LauncherSingletonCommunication, builder, type)
{
}

LauncherSingletonCommunication::LauncherSingletonCommunication()
{
  mTimesTryingToConnect = 0;

  PlasmaPrint("Sending launcher communication event with parameters:\n");
  AutoDeclare(argRange, Environment::GetInstance()->mParsedCommandLineArguments.All());
  for (; !argRange.Empty(); argRange.PopFront())
  {
    AutoDeclare(pair, argRange.Front());
    PlasmaPrint("\t(%s, %s)\n", pair.first.c_str(), pair.second.c_str());
  }

  mSocket = new TcpSocket(Protocol::Events | Protocol::Chunks, "Launcher");
  ConnectThisTo(mSocket, Events::ConnectionCompleted, OnConnectionCompleted);
  ConnectThisTo(mSocket, Events::ConnectionFailed, OnConnectionFailed);
  mSocket->Connect("localhost", LauncherCommunicationEvent::DesiredPort);

  // Running engine update will attempt to compile even though we haven't loaded
  // any projects. Since we're just communicating with an open launcher and then
  // immediately closing it should be safe to disable compilation.
  LightningManager::GetInstance()->mShouldAttemptCompile = false;

  // This happens before the engine even exists and the tcp socket only updates
  // during EngineUpdate so manually pump the engine a set number of times until
  // we succeed or fail to connect
  while (mTimesTryingToConnect < 100)
    PL::gEngine->Update();
}

LauncherSingletonCommunication::~LauncherSingletonCommunication()
{
  delete mSocket;
}

void LauncherSingletonCommunication::OnConnectionCompleted(Event* e)
{
  // stop trying to connect now
  mTimesTryingToConnect = 100;
  mStatus.Succeeded();

  LauncherCommunicationEvent myEvent;
  myEvent.LoadFromCommandArguments(Environment::GetInstance()->mParsedCommandLineArguments);

  // If no arguments were provided then just tell the open launcher to display
  // recent projects
  if (myEvent.EventId.Empty())
    myEvent.EventId = Events::LauncherOpenRecentProjects;
  mSocket->SendToAll(myEvent.EventId, &myEvent);
}

void LauncherSingletonCommunication::OnConnectionFailed(Event* e)
{
  // stop trying to connect now
  mTimesTryingToConnect = 100;
  mStatus.SetFailed("Failed to connect");
}

LightningDefineType(LauncherDebuggerCommunication, builder, type)
{
}

LauncherDebuggerCommunication::LauncherDebuggerCommunication()
{
  mSocket = nullptr;
}

LauncherDebuggerCommunication::~LauncherDebuggerCommunication()
{
}

void LauncherDebuggerCommunication::SendOpenProject(StringParam projectFile)
{
  SendOpenProject(projectFile, LauncherCommunicationEvent::DebuggerDesiredPort);
}

void LauncherDebuggerCommunication::SendOpenProject(StringParam projectFile, int port)
{
  mProjectFile = projectFile;
  mSocket = new TcpSocket(Protocol::Events | Protocol::Chunks, "Launcher");
  mSocket->mDataVersion = DataVersion::Legacy;

  ConnectThisTo(mSocket, Events::ConnectionCompleted, OnConnectionCompleted);
  ConnectThisTo(mSocket, Events::ConnectionFailed, OnConnectionFailed);
  mSocket->Connect("localhost", port);
}

void LauncherDebuggerCommunication::OnConnectionCompleted(Event* e)
{
  LauncherCommunicationEvent toSend;
  toSend.mProjectFile = mProjectFile;
  mSocket->SendToAll(Events::LauncherOpenProject, &toSend);
  DispatchEvent(Events::LauncherDebuggerCommunicationCompleted, &toSend);

  mSocket->Close();
}

void LauncherDebuggerCommunication::OnConnectionFailed(Event* e)
{
  LauncherCommunicationEvent toSend;
  toSend.mProjectFile = mProjectFile;
  DispatchEvent(Events::LauncherDebuggerCommunicationFailed, &toSend);
}

LightningDefineType(SimpleDebuggerListener, builder, type)
{
}

SimpleDebuggerListener::SimpleDebuggerListener()
{
  mListener = new TcpSocket(Protocol::Events | Protocol::Chunks, "IPC-Listener");
  // Listen for the launcher telling us to open a project
  ConnectThisTo(mListener, Events::LauncherOpenProject, OnLauncherOpenProject);
  mListener->Listen(
      LauncherCommunicationEvent::DebuggerDesiredPort, TcpSocket::MaxPossibleConnections, TcpSocketBind::Loopback);
}

SimpleDebuggerListener::~SimpleDebuggerListener()
{
  delete mListener;
}

void SimpleDebuggerListener::OnLauncherOpenProject(LauncherCommunicationEvent* e)
{
  OpenProjectFile(e->mProjectFile);
}

} // namespace Plasma
