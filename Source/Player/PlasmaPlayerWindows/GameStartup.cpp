// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
void CreateGame(OsWindow* mainWindow, StringParam projectFile, Cog* projectCog);
void LoadGamePackages(StringParam projectFile, Cog* projectCog);

void GameStartup::UserInitializeConfig(Cog* configCog)
{
	HasOrAdd<EditorSettings>(configCog);
  HasOrAdd<ContentConfig>(configCog);
  HasOrAdd<TextEditorConfig>(configCog);
}

void GameStartup::UserInitialize()
{
  String projectFile = Environment::GetValue<String>("file");
  String newProject = Environment::GetValue<String>("newProject");

  // Check to see if there was a project file in the same directory.
  static const String cDefaultProjectFile("Project.plasmaproj");
  if (FileExists(cDefaultProjectFile))
  {
    projectFile = cDefaultProjectFile;
  }
  else 
  {
      FatalEngineError("No project file found");
      return Exit(1);
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
  Cog* projectCog = PL::gFactory->Create(PL::gEngine->GetEngineSpace(), projectFile, 0, nullptr);

  if (projectCog == nullptr)
  {
	  FatalEngineError("Failed load project '%s'", projectFile.c_str());
	  return Exit(1);
  }

  // Since we don't create a resiziable wigdet/close button, etc.
  // for the game, then we want the typical OS border to appear.
  mWindowStyle = (WindowStyleFlags::Enum)(mWindowStyle & ~WindowStyleFlags::ClientOnly);

  mLoadContent = false;
  mUseSplashScreen = true;
  mWindowSettingsFromProjectCog = projectCog;

  mProjectCog = projectCog;
  mProjectFile = projectFile;
  mNewProject = newProject;
}

void GameStartup::UserStartup()
{
  IntVec2 displaySize = PL::gEngine->Has<OsShell>()->GetPrimaryMonitorSize();
  mWindowSize = (displaySize / 5) * 4;
  mMinimumWindowSize = IntVec2(1024, 595);
  mWindowCentered = true;
  mWindowState = WindowState::Maximized;

  LoadGamePackages(mProjectFile, mProjectCog);
}

void GameStartup::UserCreation()
{
	CreateGame(mMainWindow, mProjectFile, mProjectCog);
}

} // namespace Plasma
