// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
void InitializeKeyboard();

void CommonLibrary::Initialize()
{
  Thread::MainThreadId = Thread::GetCurrentThreadId();

  // Start the memory system used for all systems and containers.
  Memory::Root::Initialize();

  WebRequest::Initialize();

  // Initialize platform socket library
  Plasma::Status socketLibraryInitStatus;
  Plasma::Socket::InitializeSocketLibrary(socketLibraryInitStatus);

  // Setup keyboard enumerations
  InitializeKeyboard();

  Git::Initialize();

  // This is printed to any log for debugging purposes.
  PlasmaPrint("Paths:\n"
         "  Application: %s\n"
         "      Working: %s\n"
         "    Documents: %s\n"
         "        Local: %s\n"
         "    Temporary: %s\n",
         GetApplication().c_str(),
         GetWorkingDirectory().c_str(),
         GetUserDocumentsDirectory().c_str(),
         GetUserLocalDirectory().c_str(),
         GetTemporaryDirectory().c_str());
}

void CommonLibrary::Shutdown()
{
  Git::Destroy();

  // For certain platforms (Emscripten) make sure that any
  // files saved in memory are persisted for the next run.
  PersistFiles();

  WebRequest::Shutdown();

  // Uninitialize platform socket library
  Plasma::Status socketLibraryUninitStatus;
  Plasma::Socket::UninitializeSocketLibrary(socketLibraryUninitStatus);

  Memory::Shutdown();
}

} // namespace Plasma
