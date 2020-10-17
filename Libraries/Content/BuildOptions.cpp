// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
BuildOptions::BuildOptions(ContentLibrary* library)
{
  ToolPath = PL::gContentSystem->ToolPath;

  SourcePath = library->SourcePath;
  OutputPath = library->GetOutputPath();
}
} // namespace Plasma
