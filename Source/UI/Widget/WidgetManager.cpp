// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(WidgetManager, builder, type)
{
}

WidgetManager::WidgetManager()
{
  PL::gWidgetManager = this;
  IdCounter = 0;
  mWidgetActionSpace = new ActionSpace();
  ConnectThisTo(PL::gEngine, Events::EngineUpdate, OnEngineUpdate);
  ConnectThisTo(PL::gEngine, Events::EngineDebuggerUpdate, OnEngineUpdate);
  ConnectThisTo(PL::gEngine, Events::EngineShutdown, OnShutdown);
}

WidgetManager::~WidgetManager()
{
  SafeDelete(mWidgetActionSpace);
}

void WidgetManager::OnEngineUpdate(UpdateEvent* event)
{
  mWidgetActionSpace->UpdateActions(event, ActionExecuteMode::FrameUpdate);
  mWidgetActionSpace->UpdateActions(event, ActionExecuteMode::LogicUpdate);
  DispatchEvent(Events::WidgetUpdate, event);
  CleanUp();
}

void WidgetManager::OnShutdown(Event* event)
{
  forRange (Widget* widget, Widgets.Values())
  {
    if (widget->mParent == NULL)
      widget->Destroy();
  }
  CleanUp();
}

void WidgetManager::CleanUp()
{
  while (!DestroyList.Empty())
  {
    // Use temporary list so that widgets
    // that delete widget in their destructor
    // will not create problems
    Array<Widget*> templist;
    templist.Swap(DestroyList);

    forRange (Widget* widget, templist.All())
    {
      delete widget;
    }

    templist.Clear();

    // Put it back not
    if (DestroyList.Empty())
      templist.Swap(DestroyList);
  }
}

namespace PL
{
WidgetManager* gWidgetManager = nullptr;
}

} // namespace Plasma
