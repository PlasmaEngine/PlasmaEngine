// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void ForceCompileAllShaders(Editor* editor)
{
  PL::gEngine->has(GraphicsEngine)->ForceCompileAllShaders();
}

void BindGraphicsCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("ForceCompileAllShaders", BindCommandFunction(ForceCompileAllShaders));
}

} // namespace Plasma
