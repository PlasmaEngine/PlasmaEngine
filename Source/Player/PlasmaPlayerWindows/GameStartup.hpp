// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class GameStartup : public PlasmaStartup
{
private:
  Cog* mProjectCog = nullptr;
  String mProjectFile;
  String mNewProject;

  void UserInitializeConfig(Cog* configCog) override;
  void UserInitialize() override;
  void UserStartup() override;
  void UserCreation() override;
};

} // namespace Plasma
