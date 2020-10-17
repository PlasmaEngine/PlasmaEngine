// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class Launcher
{
public:
  Launcher(OsWindow* window);

  /// Handles logic for making sure the newest Eula is accepted before opening
  /// the launcher itself.
  void Startup();

private:
  void OpenTweakablesWindow();

  OsWindow* mOsWindow;
  MainWindow* mMainWindow;
  Composite* mLauncherWindow;
};

// Global Access
namespace PL
{
extern Launcher* gLauncher;
}

} // namespace Plasma
