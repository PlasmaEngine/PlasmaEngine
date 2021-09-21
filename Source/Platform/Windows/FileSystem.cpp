// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
const Rune cDirectorySeparatorRune = Rune('\\');
const char cDirectorySeparatorCstr[] = "\\";
bool cFileSystemCaseSensitive = false;

// There is no initialization or virtual file system on this platform
FileSystemInitializer::FileSystemInitializer(PopulateVirtualFileSystem callback, void* userData)
{
}

FileSystemInitializer::~FileSystemInitializer()
{
}

void AddVirtualFileSystemEntry(StringParam absolutePath, DataBlock* stealData, TimeType modifiedTime)
{
}

bool PersistFiles()
{
  return false;
}

String GetWorkingDirectory()
{
  char temp[MAX_PATH + 1];
  _getcwd(temp, MAX_PATH);
  PlasmaStrCat(temp, MAX_PATH + 1, cDirectorySeparatorCstr);
  return String(temp);
}

void SetWorkingDirectory(StringParam path)
{
  _chdir(path.c_str());
}

String GetUserLocalDirectory()
{
  wchar_t temp[MAX_PATH + 1];
  // Consider making an option to set which path is returned for people who want
  // to use networked drives with proper permissions setup such that they work,
  // same for the GetUserDocumentsDirectory function

  // SHGFP_TYPE_DEFAULT - Returns the default path for the requested folder
  // SHGFP_TYPE_CURRENT - Returns the redirect path for the requested folder
  SHGFP_TYPE pathType = SHGFP_TYPE_DEFAULT;

  // By calling get folder path with SHGFP_TYPE_DEFAULT it will return the path
  // %USERPROFILE%\AppData\Local even if the user has redirected it to a network
  // drive this avoids user permission errors caused by one drive or other
  // networked drives
  SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, pathType, temp);
  return Narrow(temp);
}

String GetUserDocumentsDirectory()
{
  wchar_t temp[MAX_PATH + 1];
  // SHGFP_TYPE_DEFAULT - Returns the default path for the requested folder
  // SHGFP_TYPE_CURRENT - Returns the redirect path for the requested folder
  SHGFP_TYPE pathType = SHGFP_TYPE_DEFAULT;

  // By calling get folder path with SHGFP_TYPE_DEFAULT it will return the path
  // %USERPROFILE%\Documents even if the user has redirected it to a network
  // drive this avoids user permission errors caused by one drive or other
  // networked drives
  SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, pathType, temp);
  return Narrow(temp);
}

String GetApplication()
{
  wchar_t temp[MAX_PATH + 1];
  GetModuleFileName(NULL, temp, MAX_PATH);
  return Narrow(temp);
}

String GetTemporaryDirectory()
{
  wchar_t tempPath[MAX_PATH];
  GetTempPath(MAX_PATH, tempPath);
  return Narrow(tempPath);
}

bool FileExists(StringParam filePath)
{
  DWORD attributes = GetFileAttributes(Widen(filePath).c_str());
  return (attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool FileWritable(StringParam filePath)
{
  DWORD attributes = GetFileAttributes(Widen(filePath).c_str());
  if (attributes == INVALID_FILE_ATTRIBUTES)
    return false;
  else
    return !(attributes & FILE_ATTRIBUTE_READONLY);
}

bool DirectoryExists(StringParam filePath)
{
  DWORD attributes = GetFileAttributes(Widen(filePath).c_str());
  if (attributes == INVALID_FILE_ATTRIBUTES) {
      return false;
  }
  return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

String CanonicalizePath(StringParam directoryPath)
{
  wchar_t buffer[MAX_PATH];
  if (PathCanonicalize(buffer, Widen(directoryPath).c_str()))
    return Narrow(buffer);
  else
    return directoryPath;
}

bool PathIsRooted(StringParam directoryPath)
{
  WString widePath = Widen(directoryPath);
  bool isRelative = PathIsRelative(widePath.c_str());
  return !isRelative;
}

void CreateDirectory(StringParam dest)
{
  BOOL success = ::CreateDirectoryW(Widen(dest).c_str(), NULL);
  if (!success)
  {
    DWORD lastErr = GetLastError();
    if (lastErr != ERROR_ALREADY_EXISTS)
      VerifyWin(success, "Failed to create the directory '%s'.", dest.c_str());
  }
}

void CreateDirectoryAndParents(StringParam directory)
{
  char directoryPath[MAX_PATH];
  PlasmaStrCpy(directoryPath, MAX_PATH, directory.c_str());
  uint size = directory.SizeInBytes();

  for (uint c = 0; c < size; ++c)
  {
    // When there is a directory separator
    if (directoryPath[c] == cDirectorySeparatorRune.value)
    {
      if (c > 0 && directoryPath[c - 1] == ':')
        continue;
      // Null terminate
      directoryPath[c] = '\0';
      // Create directory
      CreateDirectory(directoryPath);
      // remove null terminator
      directoryPath[c] = cDirectorySeparatorRune.value;
    }
  }

  // directories no longer have a trailing '/' so we have to
  // explicitly create the final directory
  CreateDirectory(directory);
}

bool CopyFileInternal(StringParam dest, StringParam source)
{
  BOOL success = ::CopyFileW(Widen(source).c_str(), Widen(dest).c_str(), FALSE);
  VerifyWin(success, "Failed to copy file. %s to %s.", source.c_str(), dest.c_str());
  return success != 0;
}

bool MoveFileInternal(StringParam dest, StringParam source)
{
  BOOL success = ::MoveFileEx(Widen(source).c_str(), Widen(dest).c_str(), MOVEFILE_REPLACE_EXISTING);
  VerifyWin(success, "Failed to move file. %s to %s.", source.c_str(), dest.c_str());
  return success != 0;
}

bool DeleteFileInternal(StringParam file)
{
  BOOL success = ::DeleteFileW(Widen(file).c_str());
  VerifyWin(success, "Failed to delete file %s.", file.c_str());
  return success != 0;
}

bool DeleteDirectory(StringParam directory)
{
  if (!DirectoryExists(directory))
    return false;

  // RemoveDirectoryW requires the directory to be empty, so we must delete
  // everything in it
  DeleteDirectoryContents(directory);

  // this is the only part that needs to be updated platform specific
  BOOL success = ::RemoveDirectoryW(Widen(directory).c_str());
  VerifyWin(success, "Failed to delete directory %s.", directory.c_str());
  return success != 0;
}

TimeType SystemTimeToTimeType(SYSTEMTIME& systemTime)
{
  // Build a TimeType using mktime and systemTime
  tm newTime;
  memset(&newTime, 0, sizeof(tm));
  // tm_year is based at 1900
  newTime.tm_year = systemTime.wYear - 1900;
  // tm_mday is plasma based
  newTime.tm_mon = systemTime.wMonth - 1;
  newTime.tm_mday = systemTime.wDay;
  newTime.tm_hour = systemTime.wHour;
  newTime.tm_min = systemTime.wMinute;
  newTime.tm_sec = systemTime.wSecond;
  return mktime(&newTime);
}

TimeType GetFileModifiedTime(StringParam filename)
{
  WIN32_FILE_ATTRIBUTE_DATA fileInfo;
  BOOL success = GetFileAttributesEx(Widen(filename).c_str(), GetFileExInfoStandard, (void*)&fileInfo);
  CheckWin(success, "Failed to get GetFileAttributesEx.");

  // Convert to system time so the time can be parsed
  SYSTEMTIME modifiedSystemTime;
  FileTimeToSystemTime(&fileInfo.ftLastWriteTime, &modifiedSystemTime);

  // Convert and return
  return SystemTimeToTimeType(modifiedSystemTime);
}

bool SetFileToCurrentTime(StringParam filename)
{
  // Need a file handle to do file time operations
  StackHandle sourceFile;
  sourceFile = CreateFile(Widen(filename).c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

  FILETIME fileTime;
  SYSTEMTIME systemTime;
  BOOL result;

  // Gets the current system time
  GetSystemTime(&systemTime);
  // Converts the current system time to file time format
  SystemTimeToFileTime(&systemTime, &fileTime);

  // Sets last-write time of the file
  // to the converted current system time
  result = SetFileTime(sourceFile, (LPFILETIME)NULL, (LPFILETIME)NULL, &fileTime);

  return result != 0;
}

u64 GetFileSize(StringParam fileName)
{
  WIN32_FILE_ATTRIBUTE_DATA fileInfo;
  BOOL success = GetFileAttributesEx(Widen(fileName).c_str(), GetFileExInfoStandard, (void*)&fileInfo);
  CheckWin(success, "Failed to get GetFileAttributesEx.");
  if (!success)
    return 0;
  return ((u64)fileInfo.nFileSizeHigh * ((u64)MAXDWORD + (u64)1)) + (u64)fileInfo.nFileSizeLow;
}

struct FileRangePrivateData
{
  cstr mCurrent;
  HANDLE mHandle;
  WIN32_FIND_DATA mFindData;
};

FileRange::FileRange(StringParam filePath)
{
  PlasmaConstructPrivateData(FileRangePrivateData);
  memset(self, 0, sizeof(*self));
  mPath = filePath;
  if (mPath.Empty())
  {
    Error("Cannot create a file range from an empty directory/path string "
          "(working directory as empty string not supported)");
    self->mHandle = NULL;
    return;
  }

  // Copy String into temporary
  wchar_t path[MAX_PATH];
  WString wpath = Widen(mPath);
  wcsncpy_s(path, MAX_PATH, wpath.c_str(), wpath.Size());

  // Check for trailing slash and add if not there
  if (path[wpath.Size() - 1] != '\\')
    PlasmaStrCatW(path, MAX_PATH, L"\\");

  // Add the wildcard to get all files in directory
  PlasmaStrCatW(path, MAX_PATH, L"*");

  // Begin Windows file iteration
  self->mHandle = FindFirstFile(path, &self->mFindData);

  if (self->mHandle == INVALID_HANDLE_VALUE)
  {
    self->mHandle = NULL;
  }
  else
  {
    // Skip rid of "." and ".." directory results.
    if (wcscmp(self->mFindData.cFileName, L".") == 0)
      this->PopFront();
    if (self->mHandle && wcscmp(self->mFindData.cFileName, L"..") == 0)
      this->PopFront();
  }
}

FileRange::~FileRange()
{
  PlasmaGetPrivateData(FileRangePrivateData);

  if (self->mHandle)
    FindClose(self->mHandle);

  PlasmaDestructPrivateData(FileRangePrivateData);
}

bool FileRange::Empty()
{
  PlasmaGetPrivateData(FileRangePrivateData);
  return self->mHandle == NULL;
}

String FileRange::Front()
{
  PlasmaGetPrivateData(FileRangePrivateData);
  return Narrow(self->mFindData.cFileName);
}

FileEntry FileRange::FrontEntry()
{
  PlasmaGetPrivateData(FileRangePrivateData);

  LARGE_INTEGER largeInt;
  largeInt.LowPart = self->mFindData.nFileSizeLow;
  largeInt.HighPart = self->mFindData.nFileSizeHigh;

  FileEntry entry;
  entry.mFileName = Narrow(self->mFindData.cFileName);
  entry.mSize = largeInt.QuadPart;
  entry.mPath = mPath;
  return entry;
}

void FileRange::PopFront()
{
  PlasmaGetPrivateData(FileRangePrivateData);
  BOOL hasNext = FindNextFile(self->mHandle, &self->mFindData);

  if (!hasNext)
  {
    // No more files
    FindClose(self->mHandle);
    self->mHandle = NULL;
  }
}

String UniqueFileId(StringParam fullpath)
{
#ifdef FILE_NAME_NORMALIZED
  StackHandle fileHandle;

  fileHandle = CreateFile(
      Widen(fullpath).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

  if (fileHandle == INVALID_HANDLE_VALUE)
  {
    // no file just return path.
    fileHandle = cInvalidHandle;
    return fullpath;
  }

  wchar_t fixedFullPath[MAX_PATH + 1] = {0};
  DWORD size = GetFinalPathNameByHandle(fileHandle, fixedFullPath, MAX_PATH, FILE_NAME_NORMALIZED);

  if (size == 0)
    return fullpath;

  return Narrow(fixedFullPath);
#else
  return fullpath;
#endif
}

} // namespace Plasma
