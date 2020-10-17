// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Lightning
{
LightningDefineExternalBaseType(Plasma::ProcessStartInfo, TypeCopyMode::ValueType, builder, type)
{
  LightningFullBindConstructor(builder, type, Plasma::ProcessStartInfo, nullptr)->Description =
      LightningDocumentString("Class used to set up parameters before launching a process.");
  LightningFullBindDestructor(builder, type, Plasma::ProcessStartInfo);

  LightningBindField(mApplicationName)->Description =
      LightningDocumentString("Name of the application to execute. No quoting of "
                          "this string is necessary.");
  LightningBindField(mArguments)->Description = LightningDocumentString("Arguments to pass to the application.");
  LightningBindField(mWorkingDirectory)->Description = LightningDocumentString("The working directory for the process to start "
                                                                       "with. No quoting of this string is necessary.");
  LightningBindField(mShowWindow)->Description =
      LightningDocumentString("Whether or not the window of the launched application should be shown.");
  LightningBindField(mSearchPath)->Description =
      LightningDocumentString("Whether or not we should search the path for the application.");
  LightningBindField(mRedirectStandardOutput)->Description =
      LightningDocumentString("Whether or not we should redirect the Standard "
                          "Output of the process for capturing.");
  LightningBindField(mRedirectStandardError)->Description =
      LightningDocumentString("Whether or not we should redirect the Standard "
                          "Error of the process for capturing.");
  LightningBindField(mRedirectStandardInput)->Description =
      LightningDocumentString("Whether or not we should redirect the Standard "
                          "Input of the process for writing.");
}

LightningDefineType(ProcessClass, builder, type)
{
  LightningFullBindConstructor(builder, type, ProcessClass, nullptr)->Description =
      LightningDocumentString("Process class used for managing external processes and redirecting "
                          "their stdio. "
                          "Used to launch and monitor various external programs.");
  LightningFullBindDestructor(builder, type, ProcessClass);

  LightningFullBindMethod(
      builder, type, &ProcessClass::Start, (void (ProcessClass::*)(Plasma::ProcessStartInfo&)), "Start", "startInfo")
      ->Description = LightningDocumentString("Begins the execution of another process using the given parameters. ");

  LightningFullBindMethod(builder, type, &ProcessClass::WaitForClose, (int (Process::*)()), "WaitForClose", LightningNoNames)
      ->Description = LightningDocumentString("Waits for a process to close, this will block until "
                                          "the process closes.");
  LightningFullBindMethod(builder, type, &ProcessClass::IsRunning, LightningNoOverload, "IsRunning", LightningNoNames)
      ->Description = LightningDocumentString("Returns true if the process is still running, false otherwise.");
  LightningFullBindMethod(builder, type, &ProcessClass::Close, LightningNoOverload, "Close", LightningNoNames)->Description =
      LightningDocumentString("Closes the wrapper around the process, does not "
                          "close the process launched.");
  LightningFullBindMethod(builder, type, &ProcessClass::Terminate, LightningNoOverload, "Terminate", LightningNoNames)
      ->Description = LightningDocumentString("Attempts to manually shut down the process. This is not safe for the "
                                          "other process or what it's handling.");

  LightningBindGetter(StandardError)->Description =
      LightningDocumentString("The stream where standard error is re-directed to. "
                          "Null if not re-directed");
  LightningBindGetter(StandardInput)->Description =
      LightningDocumentString("The stream where standard input is re-directed to. "
                          "Null if not re-directed");
  LightningBindGetter(StandardOutput)->Description =
      LightningDocumentString("The stream where standard output is re-directed to. "
                          "Null if not re-directed");
}

ProcessClass::ProcessClass()
{
  mRedirectStandardOutput = false;
  mRedirectStandardInput = false;
  mRedirectStandardError = false;
}

ProcessClass::~ProcessClass()
{
  CloseStreams();
  Close();
}

void ProcessClass::Start(Plasma::ProcessStartInfo& info)
{
  // Close if we were already open
  CloseStreams();
  Close();

  Status status;
  Process::Start(status, info);

  // If we failed to open the process then throw an exception and return
  if (status.Failed())
  {
    ExecutableState::CallingState->ThrowException(status.Message);
    return;
  }

  if (info.mRedirectStandardError)
  {
    mStandardError = LightningAllocate(FileStreamClass);
    FileStreamClass* standardError = mStandardError;
    Process::OpenStandardError(standardError->InternalFile);
    standardError->Capabilities = StreamCapabilities::Read;
  }

  if (info.mRedirectStandardInput)
  {
    mStandardInput = LightningAllocate(FileStreamClass);
    FileStreamClass* standardInput = mStandardInput;
    Process::OpenStandardIn(standardInput->InternalFile);
    standardInput->Capabilities = StreamCapabilities::Write;
  }

  if (info.mRedirectStandardOutput)
  {
    mStandardOutput = LightningAllocate(FileStreamClass);
    FileStreamClass* standardOutput = mStandardOutput;
    Process::OpenStandardOut(standardOutput->InternalFile);
    standardOutput->Capabilities = StreamCapabilities::Read;
  }
}

void ProcessClass::Close()
{
  Process::Close();
}

void ProcessClass::CloseStreams()
{
  // Clear out the stream handles
  mStandardError = nullptr;
  mStandardInput = nullptr;
  mStandardOutput = nullptr;
}

HandleOf<FileStreamClass> ProcessClass::GetStandardError()
{
  return mStandardError;
}

HandleOf<FileStreamClass> ProcessClass::GetStandardInput()
{
  return mStandardInput;
}

HandleOf<FileStreamClass> ProcessClass::GetStandardOutput()
{
  return mStandardOutput;
}

} // namespace Lightning
