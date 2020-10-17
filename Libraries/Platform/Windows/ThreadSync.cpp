// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

ThreadLock::ThreadLock()
{
  PlasmaConstructPrivateData(CRITICAL_SECTION);
  ::InitializeCriticalSection(self);
}

ThreadLock::~ThreadLock()
{
  PlasmaGetPrivateData(CRITICAL_SECTION);
  ::DeleteCriticalSection(self);
  PlasmaDestructPrivateData(CRITICAL_SECTION);
}

void ThreadLock::Lock()
{
  PlasmaGetPrivateData(CRITICAL_SECTION);
  ::EnterCriticalSection(self);
}

void ThreadLock::Unlock()
{
  PlasmaGetPrivateData(CRITICAL_SECTION);
  ::LeaveCriticalSection(self);
}

OsEvent::OsEvent()
{
  PlasmaConstructPrivateData(HANDLE);
}

OsEvent::~OsEvent()
{
  Close();
  PlasmaDestructPrivateData(HANDLE);
}

void OsEvent::Initialize(bool manualReset, bool startSignaled)
{
  PlasmaGetPrivateData(HANDLE);
  *self = CreateEvent(NULL, manualReset, startSignaled, NULL);
  CheckWin(*self != INVALID_HANDLE_VALUE, "Failed to create event.");
}

void OsEvent::Close()
{
  PlasmaGetPrivateData(HANDLE);
  if (*self != nullptr)
  {
    VerifyWin(CloseHandle(*self), "Failed to close event.");
  }
}

void OsEvent::Signal()
{
  PlasmaGetPrivateData(HANDLE);
  VerifyWin(SetEvent(*self), "Failed to Signal event.");
}

void OsEvent::Wait()
{
  PlasmaGetPrivateData(HANDLE);
  DWORD result = WaitForSingleObject(*self, INFINITE);
  if (result == WAIT_FAILED)
  {
    VerifyWin(0, "Failed to Signal event.");
  }
}

void OsEvent::Reset()
{
  PlasmaGetPrivateData(HANDLE);
  VerifyWin(ResetEvent(*self), "Failed to Reset event.");
}

OsHandle OsEvent::GetHandle()
{
  PlasmaGetPrivateData(HANDLE);
  return *self;
}

Semaphore::Semaphore()
{
  mHandle = CreateSemaphore(NULL, 0, MaxSemaphoreCount, NULL);
}

Semaphore::~Semaphore()
{
  VerifyWin(CloseHandle(mHandle), "Failed to close Semaphore handle");
}

void Semaphore::Increment()
{
  VerifyWin(ReleaseSemaphore(mHandle, 1, NULL), "Failed to increment semaphore");
}

void Semaphore::Decrement()
{
  WaitForSingleObject(mHandle, 0);
}

void Semaphore::Reset()
{
  VerifyWin(CloseHandle(mHandle), "Failed to close Semaphore handle");
  mHandle = CreateSemaphore(NULL, 0, MaxSemaphoreCount, NULL);
}

void Semaphore::WaitAndDecrement()
{
  OsInt result = WaitForSingleObject(mHandle, INFINITE);
  if (result != WAIT_OBJECT_0)
  {
  }
}

InterprocessMutex::InterprocessMutex()
{
  PlasmaConstructPrivateData(HANDLE);
}

InterprocessMutex::~InterprocessMutex()
{
  PlasmaGetPrivateData(HANDLE);
  CloseHandle(*self);

  PlasmaDestructPrivateData(HANDLE);
}

void InterprocessMutex::Initialize(Status& status, const char* mutexName, bool failIfAlreadyExists)
{
  PlasmaGetPrivateData(HANDLE);
  *self = CreateMutex(NULL, FALSE, Widen(mutexName).c_str());

  DWORD error = GetLastError();
  if (*self == nullptr)
    status.SetFailed("Mutex initialization error.", error);
  else if (failIfAlreadyExists && error == ERROR_ALREADY_EXISTS)
    status.SetFailed("The handle already existed", error);
  else
    status.Succeeded();
}

CountdownEvent::CountdownEvent() : mCount(0)
{
  mWaitEvent.Initialize(true, true);
}

void CountdownEvent::IncrementCount()
{
  mThreadLock.Lock();
  // If count is initially plasma, reset event
  if (mCount == 0)
    mWaitEvent.Reset();
  ++mCount;
  mThreadLock.Unlock();
}

void CountdownEvent::DecrementCount()
{
  mThreadLock.Lock();
  --mCount;
  // If count is now plasma, signal event
  if (mCount == 0)
    mWaitEvent.Signal();
  mThreadLock.Unlock();
}

void CountdownEvent::Wait()
{
  mWaitEvent.Wait();
}

} // namespace Plasma
