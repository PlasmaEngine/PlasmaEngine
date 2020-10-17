// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{
/// Thread Lock
/// Safe to lock multiple times from the same thread
class PlasmaShared ThreadLock
{
public:
  ThreadLock();
  ~ThreadLock();
  void Lock();
  void Unlock();

private:
  PlasmaDeclarePrivateData(ThreadLock, 48);
};

// Wrapper around an unnamed event.
class PlasmaShared OsEvent
{
public:
  OsEvent();
  ~OsEvent();
  void Initialize(bool manualReset = false, bool startSignaled = false);
  void Close();
  void Signal();
  void Reset();
  void Wait();
  OsHandle GetHandle();

private:
  PlasmaDeclarePrivateData(OsEvent, 8);
};

const int MaxSemaphoreCount = 0x0FFFFFFF;

// Semaphore class. Multithreaded counter / gatekeeper.
class PlasmaShared Semaphore
{
public:
  Semaphore();
  ~Semaphore();
  void Increment();
  void Decrement();
  void Reset();
  void WaitAndDecrement();

private:
  OsHandle mHandle;

  PlasmaDeclarePrivateData(Semaphore, 8);
};

/// Not fully implemented as it's currently only needed for interprocess
/// communication
class PlasmaShared InterprocessMutex
{
public:
  InterprocessMutex();
  ~InterprocessMutex();

  void Initialize(Status& status, const char* mutexName, bool failIfAlreadyExists = false);

private:
  PlasmaDeclarePrivateData(InterprocessMutex, 8);
};

class CountdownEvent
{
public:
  CountdownEvent();

  void IncrementCount();
  void DecrementCount();
  void Wait();

private:
  OsEvent mWaitEvent;
  ThreadLock mThreadLock;
  int mCount;
};

} // namespace Plasma
