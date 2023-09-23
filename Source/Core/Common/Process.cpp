// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
ProcessStartInfo::ProcessStartInfo() :
    mShowWindow(false),
    mSearchPath(true),
    mRedirectStandardOutput(false),
    mRedirectStandardError(false),
    mRedirectStandardInput(false)
{
}

ProcessInfo::ProcessInfo()
{
  mProcessId = 0;
}

PlasmaShared ProcessInfo FindProcess(StringParam processName)
{
  Array<ProcessInfo> processes;
  GetProcesses(processes);

  for (size_t i = 0; i < processes.Size(); ++i)
  {
    if (processes[i].mProcessName == processName)
      return processes[i];
  }

  return ProcessInfo();
}

void Process::Start(
    Status& status, StringRange commandLine, bool redirectOut, bool redirectError, bool redirectIn, bool showWindow)
{
  ProcessStartInfo info;
  info.mArguments = commandLine;
  info.mShowWindow = showWindow;
  info.mRedirectStandardOutput = redirectOut;
  info.mRedirectStandardError = redirectError;
  info.mRedirectStandardInput = redirectIn;
  info.mSearchPath = false;

  Start(status, info);
}

SimpleProcess::~SimpleProcess()
{
  if (IsRunning())
  {
    Cancel();
    WaitForClose();
  }
}

void SimpleProcess::ExecProcess(StringParam debugName, StringParam commandLine, TextStream* stream, bool showWindow)
{
  mCancel = false;
  mStream = stream;
  mDebugName = debugName;

  Status status;
  status.CallbackOnFailure = Status::AssertCallback;
  Start(status, commandLine, true);

  OpenStandardOut(mStandardOut);

  mReadThread.Initialize(ReadThreadEntryPoint, this, debugName);
}

OsInt SimpleProcess::ReadThreadEntryPoint(void* data)
{
  SimpleProcess* self = (SimpleProcess*)data;

  // Read until we hit the end, or an error, or the user cancels
  Status status;

  for (;;)
  {
    if (self->mCancel)
      break;

    const size_t BufferSize = 4096;

    // We add 1 for an extra null terminator at the end
    // (technically the null terminator could go anywhere since we place it at
    // the end of the amount we read)
    ::byte buffer[BufferSize + 1];

    size_t amountRead = self->mStandardOut.Read(status, buffer, BufferSize);

    // Terminate this buffer and write it out
    buffer[amountRead] = '\0';
    if (self->mStream != nullptr)
      self->mStream->Write((cstr)buffer);

    if (status.Failed())
      break;
  }
  return 0;
}

int SimpleProcess::WaitForClose()
{
  int result = Process::WaitForClose();
  Close();
  mStandardOut.Close();
  mReadThread.WaitForCompletion();
  return result;
}

void SimpleProcess::Cancel()
{
  mCancel = true;
  mStandardOut.Close();
}

} // namespace Plasma
