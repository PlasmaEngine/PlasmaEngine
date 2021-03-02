// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
int PlasmaStartup::sReturnCode = 0;

int PlasmaStartup::Run()
{
  RunMainLoop(&PlasmaStartup::MainLoopFunction, this);
  return sReturnCode;
}

void PlasmaStartup::UserInitializeLibraries()
{
}

void PlasmaStartup::UserInitializeConfig(Cog* configCog)
{
}

void PlasmaStartup::UserInitialize()
{
}

void PlasmaStartup::UserStartup()
{
}

void PlasmaStartup::UserCreation()
{
}

void PlasmaStartup::UserShutdownLibraries()
{
}

void PlasmaStartup::UserShutdown()
{
}

void PlasmaStartup::Exit(int returnCode)
{
  PlasmaPrint("Exit %d\n", returnCode);
  mExit = true;
  sReturnCode = returnCode;
}

void PlasmaStartup::MainLoop()
{
  switch (mPhase)
  {
  case StartupPhase::Initialize:
    Initialize();
    NextPhase();
    break;
  case StartupPhase::UserInitialize:
    UserInitialize();
    NextPhase();
    break;
  case StartupPhase::Startup:
    Startup();
    NextPhase();
    break;
  case StartupPhase::UserStartup:
    UserStartup();
    NextPhase();
    break;
  case StartupPhase::ProcessJobs:
    // Handles changing to the next phase internally.
    ProcessJobs();
    break;
  case StartupPhase::JobsComplete:
    JobsComplete();
    NextPhase();
    break;
  case StartupPhase::UserCreation:
    UserCreation();
    NextPhase();
    break;
  case StartupPhase::EngineUpdate:
    // Handles changing to the next phase internally.
    EngineUpdate();
    break;
  case StartupPhase::UserShutdown:
    UserShutdown();
    NextPhase();
    break;
  case StartupPhase::Shutdown:
    Shutdown();
    break;
  }

  if (mExit)
  {
    // This exact string is required to be printed because it
    // is the only we know the application has exited when it
    // comes to running Emscripten (tests, builds, etc.)
    PlasmaPrint("Stopping main loop\n");
    StopMainLoop();
    delete this;
  }
}

void PlasmaStartup::MainLoopFunction(void* userData)
{
  PlasmaStartup* self = (PlasmaStartup*)userData;
  self->MainLoop();
}

void PlasmaStartup::Initialize()
{
  // Set the log and error handlers so debug printing and asserts will print to
  // the any debugger output (such as the Visual Studio Output Window).
  mDebuggerListener = new DebuggerListener();
  Plasma::Console::Add(mDebuggerListener);

  // Start the profiling system used to performance counters and timers.
  Profile::ProfileSystem::Initialize();
  mFileSystemInitializer = new FileSystemInitializer(&PopulateVirtualFileSystemWithZip);

  // Mirror console output to a log file.
  mFileListener = new FileListener();
  Plasma::Console::Add(mFileListener);

  CrashHandler::Enable();

  // CrashHandler::SetPreMemoryDumpCallback(Plasma::CrashPreMemoryDumpCallback,
  //                                       NULL);
  // CrashHandler::SetCustomMemoryCallback(Plasma::CrashCustomMemoryCallback,
  // NULL); CrashHandler::SetLoggingCallback(Plasma::CrashLoggingCallback,
  // mFileListener);
  // CrashHandler::SetSendCrashReportCallback(Plasma::SendCrashReport, NULL);
  // CrashHandler::SetCrashStartCallback(Plasma::CrashStartCallback, NULL);

  Environment* environment = Environment::GetInstance();
  environment->ParseCommandArgs(gCommandLineArguments);

  ZoneScoped;
  ProfileScopeFunction();

  if (Environment::GetValue<bool>("BeginTracing", false))
    Profile::ProfileSystem::Instance->BeginTracing();

  // Add stdout listener (requires engine initialization to get the Environment
  // object)
  if (!environment->GetParsedArgument("logStdOut").Empty())
  {
    mStdoutListener = new StdOutListener();
    Plasma::Console::Add(mStdoutListener);
  }

  CrashHandler::RestartOnCrash(Environment::GetValue<bool>("autorestart", false));

  CommonLibrary::Initialize();

  // Temporary location for registering handle managers
  // LightningRegisterSharedHandleManager(ReferenceCountedHandleManager);
  LightningRegisterSharedHandleManager(CogHandleManager);
  LightningRegisterSharedHandleManager(ComponentHandleManager);
  LightningRegisterSharedHandleManager(ResourceHandleManager);
  LightningRegisterSharedHandleManager(WidgetHandleManager);
  LightningRegisterSharedHandleManager(ContentItemHandleManager);

  RegisterCommonHandleManagers();

  PlasmaRegisterHandleManager(ContentComposition);

  // Graphics specific
  PlasmaRegisterThreadSafeReferenceCountedHandleManager(ThreadSafeReferenceCounted);
  PlasmaRegisterThreadSafeReferenceCountedHandleManager(GraphicsBlendSettings);
  PlasmaRegisterThreadSafeReferenceCountedHandleManager(GraphicsDepthSettings);

  // Setup the core Lightning library
  mLightningSetup = new LightningSetup(SetupFlags::DoNotShutdownMemory);

  // We need the calling state to be set so we can create Handles for Meta
  // Components
  Lightning::Module module;
  mState = module.Link();

#if !defined(PlasmaDebug) && !defined(PlasmaTargetOsEmscripten)
  mState->SetTimeout(5);
#endif

  ExecutableState::CallingState = mState;

  MetaDatabase::Initialize();

  // Add the core library to the meta database
  MetaDatabase::GetInstance()->AddNativeLibrary(Core::GetInstance().GetLibrary());

  // Initialize Plasma Libraries
  PlatformLibrary::Initialize();
  GeometryLibrary::Initialize();
  // Geometry doesn't know about the Meta Library, so it cannot add itself to
  // the MetaDatabase
  MetaDatabase::GetInstance()->AddNativeLibrary(GeometryLibrary::GetLibrary());
  MetaLibrary::Initialize();
  SerializationLibrary::Initialize();
  ContentMetaLibrary::Initialize();
  SpatialPartitionLibrary::Initialize();

  EngineLibrary::Initialize();
  GraphicsLibrary::Initialize();
  PhysicsLibrary::Initialize();
  NetworkingLibrary::Initialize();
  SoundLibrary::Initialize();

  WidgetLibrary::Initialize();
  GameplayLibrary::Initialize();
  EditorLibrary::Initialize();
  UiWidgetLibrary::Initialize();

  LightningScriptLibrary::Initialize();

  NativeBindingList::ValidateTypes();

  UserInitializeLibraries();

  LoadConfig(&InitializeConfigExternal, this);

  Tweakables::Load();

  Shortcuts::GetInstance()->Load(
      FilePath::Combine(PL::gEngine->GetConfigCog()->has(MainConfig)->DataDirectory, "Shortcuts.data"));

  // Load documentation for all native libraries
  DocumentationLibrary::GetInstance()->LoadDocumentation(
      FilePath::Combine(PL::gEngine->GetConfigCog()->has(MainConfig)->DataDirectory, "Documentation.data"));

  PlasmaPrint("Os: %s\n", Os::GetVersionString().c_str());
}

void PlasmaStartup::InitializeConfigExternal(Cog* configCog, void* userData)
{
  ((PlasmaStartup*)userData)->UserInitializeConfig(configCog);
}

OsShell* CreateOsShellSystem();
System* CreateSoundSystem();
System* CreateGraphicsSystem();
System* CreatePhysicsSystem();
System* CreateTimeSystem();

void PlasmaStartup::Startup()
{
  ZoneScoped;
  ProfileScopeFunction();
  Engine* engine = PL::gEngine;
  Cog* configCog = engine->GetConfigCog();

  // Create all core systems
  engine->AddSystem(CreateUnitTestSystem());
  engine->AddSystem(CreateOsShellSystem());
  engine->AddSystem(CreateTimeSystem());
  engine->AddSystem(CreatePhysicsSystem());
  engine->AddSystem(CreateSoundSystem());
  engine->AddSystem(CreateGraphicsSystem());

  SystemInitializer initializer;
  initializer.mEngine = engine;
  initializer.Config = configCog;

  // Initialize all systems.
  engine->Initialize(initializer);

  if (mLoadContent)
    LoadContentConfig();

  PlasmaPrint("Creating main window.\n");

  OsShell* osShell = engine->has(OsShell);

  IntVec2 size = mWindowSize;
  if (mWindowSize == IntVec2::cZero)
  {
    size = osShell->GetPrimaryMonitorSize();
  }
  WindowState::Enum state = mWindowState;

  String name = BuildString(GetOrganization(), " ", GetApplicationName());

  if (mWindowSettingsFromProjectCog)
  {
    WindowLaunchSettings* windowLaunch = mWindowSettingsFromProjectCog->has(WindowLaunchSettings);
    if (windowLaunch != nullptr)
    {
      size = windowLaunch->mWindowedResolution;
      if (windowLaunch->mLaunchFullscreen)
        state = WindowState::Fullscreen;
    }

    ProjectSettings* projectSettings = mWindowSettingsFromProjectCog->has(ProjectSettings);
    if (projectSettings != nullptr)
    {
      name = projectSettings->ProjectName;
    }
  }

  // On Emscripten, the window full screen can only be done by a user
  // action. Setting it on startup causes an abrupt change the first time
  // the user click or hits a button.
#if !defined(PlasmaTargetOsEmscripten)
  if (state == WindowState::Fullscreen)
    state = WindowState::Maximized;
#endif

  IntVec2 minSize = Math::Min(mMinimumWindowSize, size);
  IntVec2 monitorClientPos = IntVec2(0, 0);

  if (mWindowCentered)
  {
    IntRect monitorRect = osShell->GetPrimaryMonitorRectangle();
    monitorClientPos = monitorRect.Center(size);
  }

  OsWindow* mainWindow = osShell->CreateOsWindow(name, size, monitorClientPos, nullptr, mWindowStyle, state);
  mainWindow->SetMinClientSize(minSize);

  // Pass window handle to initialize the graphics api
  auto graphics = engine->has(GraphicsEngine);
  graphics->CreateRenderer(mainWindow);

  if (mUseSplashScreen)
    graphics->SetSplashscreenLoading();

  // Fix any issues related to Intel drivers (we call SetState twice on purpose to fix the driver issues).
  mainWindow->PlatformSpecificFixup();

  // Used for trapping the mouse.
  PL::gMouse->mActiveWindow = mainWindow;

  // Note that content and resources are loaded after CreateRenderer so that they may use the Renderer API to upload
  // textures, meshes, etc.
  mMainWindow = mainWindow;
}

void PlasmaStartup::ProcessJobs()
{
  PL::gJobs->RunJobsTimeSliced();

  if (PL::gJobs->AreAllJobsCompleted())
  {
    NextPhase();
  }
  else if (ThreadingEnabled)
  {
    // This should be a proper wait, not a spin wait with a sleep...
    Os::Sleep(10);
  }

  PL::gDispatch->DispatchEvents();
}

void PlasmaStartup::JobsComplete()
{
  PL::gResources->SetupDefaults();
}

void PlasmaStartup::EngineUpdate()
{
  PL::gEngine->Update();
  if (PL::gEngine->mEngineActive)
    return;

  NextPhase();
}

void PlasmaStartup::Shutdown()
{
  {
    ZoneScoped;
    ProfileScopeFunction();
    PL::gEngine->Shutdown();

    UserShutdownLibraries();

    Core::GetInstance().GetLibrary()->ClearComponents();

    // Shutdown in reverse order
    LightningScriptLibrary::Shutdown();

    UiWidgetLibrary::Shutdown();
    EditorLibrary::Shutdown();
    GameplayLibrary::Shutdown();
    WidgetLibrary::Shutdown();

    SoundLibrary::Shutdown();
    NetworkingLibrary::Shutdown();
    PhysicsLibrary::Shutdown();
    GraphicsLibrary::Shutdown();
    EngineLibrary::Shutdown();

    SpatialPartitionLibrary::Shutdown();
    ContentMetaLibrary::Shutdown();
    SerializationLibrary::Shutdown();
    MetaLibrary::Shutdown();
    GeometryLibrary::Shutdown();
    PlatformLibrary::Shutdown();

    // ClearLibrary
    LightningScriptLibrary::GetInstance().ClearLibrary();

    UiWidgetLibrary::GetInstance().ClearLibrary();
    EditorLibrary::GetInstance().ClearLibrary();
    GameplayLibrary::GetInstance().ClearLibrary();
    WidgetLibrary::GetInstance().ClearLibrary();

    SoundLibrary::GetInstance().ClearLibrary();
    NetworkingLibrary::GetInstance().ClearLibrary();
    PhysicsLibrary::GetInstance().ClearLibrary();
    GraphicsLibrary::GetInstance().ClearLibrary();
    EngineLibrary::GetInstance().ClearLibrary();

    SpatialPartitionLibrary::GetInstance().ClearLibrary();
    ContentMetaLibrary::GetInstance().ClearLibrary();
    SerializationLibrary::GetInstance().ClearLibrary();
    MetaLibrary::GetInstance().ClearLibrary();
    GeometryLibrary::GetInstance().ClearLibrary();

    // Destroy
    LightningScriptLibrary::Destroy();

    UiWidgetLibrary::Destroy();
    EditorLibrary::Destroy();
    GameplayLibrary::Destroy();
    WidgetLibrary::Destroy();

    SoundLibrary::Destroy();
    NetworkingLibrary::Destroy();
    PhysicsLibrary::Destroy();
    GraphicsLibrary::Destroy();
    EngineLibrary::Destroy();

    SpatialPartitionLibrary::Destroy();
    ContentMetaLibrary::Destroy();
    SerializationLibrary::Destroy();
    MetaLibrary::Destroy();
    GeometryLibrary::Destroy();

    LightningManager::Destroy();
    MetaDatabase::Destroy();

    delete mState;
    delete mLightningSetup;

    CommonLibrary::Shutdown();

    PlasmaPrint("Terminated\n");

    mExit = true;
  }

  Profile::ProfileSystem::Shutdown();
}

void PlasmaStartup::NextPhase()
{
  PlasmaPrint("Completed phase: %s\n", StartupPhase::Names[mPhase]);
  mPhase = (StartupPhase::Enum)(mPhase + 1);
  PlasmaPrint("Next phase: %s\n", StartupPhase::Names[mPhase]);
}

} // namespace Plasma
