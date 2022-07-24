// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void CreateEditor2(OsWindow* mainWindow, StringParam projectFile, StringParam newProjectName);
void CreateGame(OsWindow* mainWindow, StringParam projectFile, Cog* projectCog);
void LoadGamePackages(StringParam projectFile, Cog* projectCog);

void GameOrEditorStartup::UserInitializeConfig(Cog* configCog)
{
  HasOrAdd<EditorSettings>(configCog);
  HasOrAdd<ContentConfig>(configCog);
  HasOrAdd<TextEditorConfig>(configCog);
}

void GameOrEditorStartup::UserInitialize()
{
  mFileListener = new FileListener();
  
  CrashHandler::Enable();

  CrashHandler::SetPreMemoryDumpCallback(Plasma::CrashPreMemoryDumpCallback,
      NULL);
  CrashHandler::SetCustomMemoryCallback(Plasma::CrashCustomMemoryCallback,
      NULL); CrashHandler::SetLoggingCallback(Plasma::CrashLoggingCallback,
      mFileListener);
  CrashHandler::SetSendCrashReportCallback(Plasma::SendCrashReport, NULL);
  CrashHandler::SetCrashStartCallback(Plasma::CrashStartCallback, NULL);


  String projectFile = Environment::GetValue<String>("file");
  bool playGame = Environment::GetValue<bool>("play", false);
  String newProject = Environment::GetValue<String>("newProject");

  // Check to see if there was a project file in the same directory.
  static const String cDefaultProjectFile("Project.plasmaproj");
  if (FileExists(cDefaultProjectFile))
  {
    projectFile = cDefaultProjectFile;
    playGame = true;
  }

  // If there was no specified project file (or it doesn't exist) and we're not
  // creating a new project, then use a fall-back project that we open from our
  // data directory. This project should be read-only, but is useful for testing
  // platforms before the full launcher pipeline is implemented. Note that if
  // the 'projectFile' does not exist, but is specified, we will not use the
  // fall-back.
  Cog* configCog = PL::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);
  EditorConfig* editorConfig = configCog->has(EditorConfig);
  if (mainConfig && projectFile.Empty() && newProject.Empty() &&
      (editorConfig == nullptr || editorConfig->EditingProject.Empty()))
  {
    projectFile = FilePath::Combine(mainConfig->DataDirectory, "Fallback", "Fallback.plasmaproj");
  }

  // The options defaults are already tailored to the Editor.
  // If we're playing the game, we need to load the project Cog.
  // We'll also potentially derive some window settings from the project.
  Cog* projectCog = nullptr;
  if (playGame)
  {
    projectCog = PL::gFactory->Create(PL::gEngine->GetEngineSpace(), projectFile, 0, nullptr);
    if (projectCog == nullptr)
    {
      FatalEngineError("Failed load project '%s'", projectFile.c_str());
      return Exit(1);
    }

    // Since we don't create a resiziable wigdet/close button, etc.
    // for the game, then we want the typical OS border to appear.
    mWindowStyle = (WindowStyleFlags::Enum)(mWindowStyle & ~WindowStyleFlags::ClientOnly);
  }

  mLoadContent = !playGame;
  mUseSplashScreen = playGame;
  mWindowSettingsFromProjectCog = projectCog;

  mPlayGame = playGame;
  mProjectCog = projectCog;
  mProjectFile = projectFile;
  mNewProject = newProject;
}

void GameOrEditorStartup::UserStartup()
{
  IntVec2 displaySize = PL::gEngine->Has<OsShell>()->GetPrimaryMonitorSize();
  mWindowSize = (displaySize / 5) * 4;
  mMinimumWindowSize = IntVec2(1024, 595);
  mWindowCentered = true;
  mWindowState = WindowState::Maximized;

  if (mPlayGame)
  {
    LoadGamePackages(mProjectFile, mProjectCog);
  }
  else
  {
    Array<String> coreLibs;

    coreLibs.PushBack("FragmentCore");
    coreLibs.PushBack("Loading");
    coreLibs.PushBack("PlasmaCore");
    coreLibs.PushBack("UiWidget");
    coreLibs.PushBack("EditorUi");
    coreLibs.PushBack("Editor");

    LoadCoreContent(coreLibs);
  }

  String cloneUrl = Environment::GetValue<String>("cloneUrl");
  if (!cloneUrl.Empty() && !FileExists(mProjectFile))
  {
    GitCloneJob* job = new GitCloneJob();
    job->mUrl = cloneUrl;
    job->mDirectory = FilePath::GetDirectoryPath(mProjectFile);
    PlasmaPrint("Cloning url '%s' to directory '%s'\n", job->mUrl.c_str(), job->mDirectory.c_str());
    PL::gJobs->AddJob(job);
  }
}

void GameOrEditorStartup::UserCreation()
{
    CreateEditor2(mMainWindow, mProjectFile, mNewProject);

    Event event;
    PL::gEngine->DispatchEvent(Events::NoProjectLoaded, &event);


    Editor2* editor = new Editor2();

    Connect(PL::gEngine->has(GraphicsEngine), "UiRenderUpdate", editor, &Editor2::OnUiRenderUpdate);
}

void ImGUIBasics() 
{
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    bool isOpen = true;
    ImGui::Begin("DockSpace Demo", &isOpen, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();

            if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
            if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
            if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
            if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
            if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
            ImGui::Separator();

            ImGui::EndMenu();
        }
 

        ImGui::EndMenuBar();
    }

    ImGui::End();

    bool showDemo = true;

    ImGui::ShowAboutWindow(&showDemo);
    ImGui::ShowMetricsWindow(&showDemo);
}

void Editor2::OnUiRenderUpdate(Event* event)
{
    PL::gEngine->has(GraphicsEngine)->AddImguiRender(ImGUIBasics);
}

} // namespace Plasma
