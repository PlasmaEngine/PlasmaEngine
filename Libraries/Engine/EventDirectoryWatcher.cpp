// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
namespace Events
{
DefineEvent(FileModified);
DefineEvent(FileCreated);
DefineEvent(FileDeleted);
DefineEvent(FileRenamed);
} // namespace Events

LightningDefineType(EventDirectoryWatcher, builder, type)
{
  type->HandleManager = LightningManagerId(PointerManager);
}

LightningDefineType(FileEditEvent, builder, type)
{
}

EventDirectoryWatcher::EventDirectoryWatcher(StringParam directory) :
    mWatcher(directory.c_str(),
             DirectoryWatcher::CallBackCreator<EventDirectoryWatcher, &EventDirectoryWatcher::FileCallBack>,
             this)
{
}

OsInt EventDirectoryWatcher::FileCallBack(DirectoryWatcher::FileOperationInfo& info)
{
  FileEditEvent* event = new FileEditEvent();
  event->FileName = info.FileName;
  event->OldFileName = info.OldFileName;
  event->TimeStamp = Time::Clock();

  String eventName = Events::FileModified;
  if (info.Operation == DirectoryWatcher::Added)
    eventName = Events::FileCreated;
  if (info.Operation == DirectoryWatcher::Removed)
    eventName = Events::FileDeleted;
  if (info.Operation == DirectoryWatcher::Renamed)
    eventName = Events::FileRenamed;

  PL::gDispatch->DispatchOn(this, this->GetDispatcher(), eventName, event);
  return 0;
}

} // namespace Plasma
