// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

static String gEngineStartTime = GetTimeAndDateStamp();

// Listen to console output and output to a log file.
class FileListener : public ConsoleListener
{
public:
  File* logFile;
  bool mCanOpen;
  String mOverridenFilePath;
  String mBaseLogFileName;

  FileListener()
  {
    logFile = nullptr;
    mCanOpen = true;
    mBaseLogFileName = FilePath::GetFileNameWithoutExtension(GetApplication());
  }

  ~FileListener()
  {
    SafeDelete(logFile);
  }

  void Print(FilterType filterType, cstr message) override
  {
    // Do not log user messages
    if (filterType & Filter::UserFilter)
      return;

    if (logFile == nullptr && mCanOpen)
    {
      logFile = new File();
      String logFilePath = GetLogFilePath();
      bool opened =
          logFile->Open(logFilePath.c_str(), FileMode::Append, FileAccessPattern::Sequential, FileShare::Read);

      if (!opened)
      {
        logFile = nullptr;
        mCanOpen = false;
      }
    }

    if (logFile != nullptr)
      logFile->Write((::byte*)message, strlen(message));
  }

  String GetLogFilePath()
  {
    // if we have an overridden path then write to that instead
    if (!mOverridenFilePath.Empty())
      return mOverridenFilePath;

    String logFileName = BuildString(mBaseLogFileName, gEngineStartTime, ".txt");
    String logFilePath = FilePath::Combine(GetTemporaryDirectory(), logFileName);
    return logFilePath;
  }

  // Note: this is expected to be called before any file opens.
  // This will not change to another file if one was already opened.
  void OverrideLogFile(StringParam filePath)
  {
    mOverridenFilePath = filePath;
  }

  void Flush() override
  {
    if (logFile)
    {
      logFile->Flush();
      logFile->Close();
      logFile = nullptr;
    }
  }

  void Close()
  {
    Flush();
    mCanOpen = false;
  }
};

} // namespace Plasma
