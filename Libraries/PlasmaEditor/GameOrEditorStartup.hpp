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

  void UserInitializeConfig(Cog* configCog) override;
  void UserInitialize() override;
  void UserStartup() override;
  void UserCreation() override;
};

} // namespace Plasma
