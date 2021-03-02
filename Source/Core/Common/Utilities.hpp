// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{
/// System Memory Information
struct PlasmaShared MemoryInfo
{
  uint Reserve;
  uint Commit;
  uint Free;
};

namespace Os
{

// Sleep the current thread for ms milliseconds.
PlasmaShared void Sleep(uint ms);

// Set the Timer Frequency (How often the OS checks threads for sleep, etc)
PlasmaShared void SetTimerFrequency(uint ms);

// Get the user name for the current profile
PlasmaShared String UserName();

// Get the computer name
PlasmaShared String ComputerName();

// Get computer Mac Address of adapter 0
PlasmaShared u64 GetMacAddress();

// Check if a debugger is attached
PlasmaShared bool IsDebuggerAttached();

// Output a message to any attached debuggers
PlasmaShared void DebuggerOutput(const char* message);

// Debug break (only if a debugger is attached)
PlasmaShared bool DebugBreak();

// Attempts to enable memory leak checking (break on
PlasmaShared void EnableMemoryLeakChecking(int breakOnAllocation = -1);

// When a diagnostic error occurs, this is the default response
PlasmaShared bool ErrorProcessHandler(ErrorSignaler::ErrorData& errorData);

// Tells the shell to open or show a directory.
PlasmaShared bool ShellOpenDirectory(StringParam directory);

// Tells the shell to open or show a file.
PlasmaShared bool ShellOpenFile(StringParam file);

// Tells the shell to edit a file.
PlasmaShared bool ShellEditFile(StringParam file);

// Open the application with parameters.
PlasmaShared bool ShellOpenApplication(StringParam file, StringParam parameters = String());

// On browser based platforms, we can't access the user's file-system so we need to download files instead.
PlasmaShared bool SupportsDownloadingFiles();

// Download a single file to the user's file system (on supported browser platforms).
PlasmaShared void DownloadFile(cstr fileName, const DataBlock& data);

// Open's a url in a browser or tab.
PlasmaShared void OpenUrl(cstr url);

// Mark a file as executable.
PlasmaShared void MarkAsExecutable(cstr fileName);

// Get the time in milliseconds for a double click.
PlasmaShared unsigned int GetDoubleClickTimeMs();

// Get the memory status of the Os.
PlasmaShared void GetMemoryStatus(MemoryInfo& memoryInfo);

// Get an Environmental variable
PlasmaShared String GetEnvironmentalVariable(StringParam variable);

// Get a string describing the current operating system version.
PlasmaShared String GetVersionString();

// Get the path to an installed executable (may use similar logic to discover the executable name as in
// BuildVersion.cpp).
PlasmaShared String GetInstalledExecutable(StringParam organization, StringParam name, StringParam guid);
} // namespace Os

// Generate a 64 bit unique Id. Uses system timer and mac
// address to generate the id.
PlasmaShared u64 GenerateUniqueId64();

// Waits for expression to evaluate to true, checking approximately every
// pollPeriod (in milliseconds)
#define WaitUntil(expression, pollPeriod)                                                                              \
  do                                                                                                                   \
  {                                                                                                                    \
    while (!(expression))                                                                                              \
    {                                                                                                                  \
      Os::Sleep(pollPeriod);                                                                                           \
    }                                                                                                                  \
  } while (gConditionalFalseConstant)

} // namespace Plasma
