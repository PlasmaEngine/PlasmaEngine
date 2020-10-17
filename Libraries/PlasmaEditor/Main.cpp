// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

using namespace Plasma;

extern "C" int main(int argc, char* argv[])
{
  CommandLineToStringArray(gCommandLineArguments, argv, argc);
  SetupApplication(1, sPlasmaOrganization, sEditorGuid, sEditorName);

  return (new GameOrEditorStartup())->Run();
}
