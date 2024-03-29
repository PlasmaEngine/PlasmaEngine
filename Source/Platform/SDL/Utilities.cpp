// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Os
{

void Sleep(uint ms)
{
  SDL_Delay((Uint32)ms);
}

#if !defined(PlasmaPlatformNoSetTimerFrequency)
void SetTimerFrequency(uint ms)
{
  // Not supported by SDL.
}
#endif

const char* GetEnvironmentList(const char* defaultValue, const char* names[], size_t length)
{
  for (size_t i = 0; i < length; ++i)
  {
    const char* name = names[i];
    const char* result = getenv(name);

    if (result != nullptr && strlen(result) != 0)
      return result;
  }

  return defaultValue;
}

#if !defined(PlasmaPlatformNoUserName)
String UserName()
{
  // There is no portable way to get the user name
  const char* names[] = {"USER", "USERNAME", "LOGNAME", "COMPUTERNAME", "HOSTNAME"};
  return GetEnvironmentList("User", names, SDL_arraysize(names));
}
#endif

#if !defined(PlasmaPlatformNoComputerName)
String ComputerName()
{
  // There is no portable way to get the computer/host name
  const char* names[] = {"COMPUTERNAME", "HOSTNAME", "USER", "USERNAME", "LOGNAME"};
  return GetEnvironmentList("Computer", names, SDL_arraysize(names));
}
#endif

#if !defined(PlasmaPlatformNoIsDebuggerAttached)
bool IsDebuggerAttached()
{
  // SDL cannot detect whether a debugger is attached.
  return false;
}
#endif

PlasmaShared void DebuggerOutput(const char* message)
{
  printf("%s", message);
}

#if !defined(PlasmaPlatformNoGetMacAddress)
u64 GetMacAddress()
{
  // Not supported by SDL.
  return 0;
}
#endif

bool DebugBreak()
{
#if defined(PlasmaTargetOsEmscripten)
  emscripten_debugger();
#else
  SDL_TriggerBreakpoint();
#endif
  return true;
}

#if !defined(PlasmaPlatformNoEnableMemoryLeakChecking)
void EnableMemoryLeakChecking(int breakOnAllocation)
{
  // Not supported by SDL.
}
#endif

DeclareEnum4(ReturnCode, Continue, DebugBreak, Terminate, Ignore);

bool ErrorProcessHandler(ErrorSignaler::ErrorData& errorData)
{
  // Stores the resulting quote removed message from below
  String expression = errorData.Expression;

  // Check if no message was provided
  const char* errorMessage = errorData.Message ? errorData.Message : "No message";

  String message = BuildString(errorMessage, "\n");

  String callStack = GetCallStack();
  if (!callStack.Empty())
    message = BuildString(message, callStack, "\n");

  Console::Print(Filter::ErrorFilter, message.c_str());

  // Show a message box
  message = BuildString(message, "Would you like to continue?");

  const SDL_MessageBoxButtonData buttons[] = {
      {0, ReturnCode::Continue, "Continue"},
      {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, ReturnCode::DebugBreak, "DebugBreak"},
      {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, ReturnCode::Terminate, "Terminate"},
      {0, ReturnCode::Ignore, "Ignore"},
  };

  SDL_MessageBoxData data = {
      SDL_MESSAGEBOX_INFORMATION, nullptr, "Error", message.c_str(), SDL_arraysize(buttons), buttons, nullptr};

  int buttonId = -1;
  SDL_ShowMessageBox(&data, &buttonId);

  // Based on the exit code...
  switch (buttonId)
  {
  case ReturnCode::Continue:
    // No debug break, just continue
    return false;

  case ReturnCode::DebugBreak:
    // Returning true will cause a break point
    return true;

  case ReturnCode::Terminate:
    // Immediately kill the application
    abort();
    return false;

  case ReturnCode::Ignore:
    errorData.IgnoreFutureAssert = true;
    return false;

  default:
    // Force a break point, we have no idea what we got back
    return true;
  }
}

#if !defined(PlasmaPlatformNoWebRequest)
void WebRequest(Status& status,
                StringParam url,
                const Array<WebPostData>& postDatas,
                const Array<String>& additionalRequestHeaders,
                WebRequestHeadersFn onHeadersReceived,
                WebRequestDataFn onDataReceived,
                void* userData)
{
  status.SetFailed("WebRequest not supported by SDL");
}
#endif

bool ShellOpen(const char* path)
{
  static cstr cOpeners[] = {"xdg-open", "open", "start"};
  static const size_t cOpenersCount = sizeof(cOpeners) / sizeof(*cOpeners);

  bool result = false;
  for (size_t i = 0; i < cOpenersCount; ++i)
  {
    String commandLine = String::Format("%s \"%s\" &", cOpeners[i], path);
    if (system(commandLine.c_str()) == 0)
      result = true;
  }
  return result;
}

bool ShellOpenDirectory(StringParam directory)
{
  return ShellOpen(directory.c_str());
}

bool ShellOpenFile(StringParam file)
{
  return ShellOpen(file.c_str());
}

bool ShellEditFile(StringParam file)
{
  return ShellOpen(file.c_str());
}

#if !defined(PlasmaPlatformNoShellOpenApplication)
bool ShellOpenApplicationWithWorkingDirectory(StringParam file, StringParam parameters, StringParam workingDirectory)
{
    String commandLine = String::Format("\"%s\" %s &", file.c_str(), parameters.c_str());
    int result = system(commandLine.c_str());
    return result == 0;
}

bool ShellOpenApplication(StringParam file, StringParam parameters)
{
  String commandLine = String::Format("\"%s\" %s &", file.c_str(), parameters.c_str());
  int result = system(commandLine.c_str());
  return result == 0;
}
#endif

#if !defined(PlasmaPlatformNoSupportsDownloadingFiles)
bool SupportsDownloadingFiles()
{
  return false;
}
#endif

#if !defined(PlasmaPlatformNoDownloadFile)
void DownloadFile(cstr fileName, const DataBlock& data)
{
}
#endif

#if !defined(PlasmaPlatformNoOpenUrl)
void OpenUrl(cstr url)
{
  ShellOpen(url);
}
#endif

void MarkAsExecutable(cstr fileName)
{
#if defined(PlasmaTargetOsLinux) || defined(PlasmaTargetOsMac)
  system(String::Format("chmod +x \"%s\"", fileName).c_str());
#endif
}

unsigned int GetDoubleClickTimeMs()
{
  return 500;
}

String GetEnvironmentalVariable(StringParam variable)
{
  return getenv(variable.c_str());
}

#if !defined(PlasmaPlatformNoGetMemoryStatus)
void GetMemoryStatus(MemoryInfo& data)
{
  // Not supported by SDL.
  data.Commit = 0;
  data.Free = 0;
  data.Reserve = 0;
}
#endif

String GetVersionString()
{
  SDL_version version;
  SDL_GetVersion(&version);
  return String::Format("%s SDL %d.%d.%d", SDL_GetPlatform(), version.major, version.minor, version.patch);
}

String GetInstalledExecutable(StringParam organization, StringParam name, StringParam guid)
{
  return GetRelativeExecutable(organization, name);
}

} // namespace Os

u64 GenerateUniqueId64()
{
  if (gDeterministicMode)
  {
    static u64 deterministicId = 0;
    ++deterministicId;
    return deterministicId;
  }

  u64 result = SDL_GetPerformanceCounter() ^ SDL_GetTicks();

  static u64 highCount = 0;
  ++highCount;
  result += (highCount << 48) ^ Os::UserName().Hash();

  return result;
}
} // namespace Plasma
