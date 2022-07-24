// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class GameOrEditorStartup : public PlasmaStartup
{
private:
  bool mPlayGame = false;
  Cog* mProjectCog = nullptr;
  String mProjectFile;
  String mNewProject;


  MainWindow* mMainUIWindow;

  void UserInitializeConfig(Cog* configCog) override;
  void UserInitialize() override;
  void UserStartup() override;
  void UserCreation() override;
    
  UniquePointer<FileListener> mFileListener;
};

class Editor2 : public EventObject
{
public:
    void OnUiRenderUpdate(Event* event);
};

} // namespace Plasma
