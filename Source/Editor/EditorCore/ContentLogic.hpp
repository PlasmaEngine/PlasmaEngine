// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class Editor;
class ResourcePackage;
class Viewport;
class Mouse;
class MouseEvent;

bool LoadContentLibrary(StringParam name);

void LoadContentConfig();
void LoadCoreContent(Array<String>& coreLibs);

} // namespace Plasma
