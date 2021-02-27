// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

/// Drop a resource on a object
bool DropOnObject(MetaDropEvent* event, Cog* droppedOn);
bool EditorDrop(MetaDropEvent* event, Viewport* viewport, Space* space, Cog* droppedOn);
} // namespace Plasma
