// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#if defined(PlasmaTargetOsMacOS)
#  include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#elif __has_include(<experimental/filesystem>)
#  include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#  include <filesystem>
namespace fs = std::filesystem;
#endif

namespace Plasma
{
const Rune cDirectorySeparatorRune = Rune(fs::path::preferred_separator);
const char cDirectorySeparatorCstr[] = {(char)fs::path::preferred_separator, '\0'};
bool cFileSystemCaseSensitive = true;

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

bool BrowseDirectory(StringParam path)
{
    return false;
}

String GetWorkingDirectory()
{
  std::error_code error;
  return fs::current_path(error).u8string().c_str();
}

void SetWorkingDirectory(StringParam path)
{
  std::error_code error;
  fs::current_path(fs::path(path.c_str()), error);
}

#if !defined(PlasmaPlatformNoGetUserLocalDirectory)
String GetUserLocalDirectory()
{
  std::error_code error;
  return fs::temp_directory_path(error).append("LocalDirectory").u8string().c_str();
}
#endif

#if !defined(PlasmaPlatformNoGetUserDocumentsDirectory)
String GetUserDocumentsDirectory()
{
  std::error_code error;
  return fs::temp_directory_path(error).append("DocumentsDirectory").u8string().c_str();
}
#endif

#if !defined(PlasmaPlatformNoGetApplication)
String GetApplication()
{
  // The first entry in the command line arguments should be our executable.
  ReturnIf(
      gCommandLineArguments.Empty(), "/Main.app", "The command line arguments should not be empty, were they set?");
  return gCommandLineArguments.Front();
}
#endif

String GetTemporaryDirectory()
{
  std::error_code error;
  return fs::temp_directory_path(error).u8string().c_str();
}

bool FileExists(StringParam filePath)
{
  std::error_code error;
  auto type = fs::status(filePath.c_str(), error).type();
  return type != fs::file_type::none && type != fs::file_type::not_found && type != fs::file_type::directory &&
         type != fs::file_type::unknown;
}

bool FileWritable(StringParam filePath)
{
  // Not the greatest way to check for writable...
  // We can use permissions, but we still don't know if we have the ability to
  // write to a file.
  FILE* file = fopen(filePath.c_str(), "a");
  if (file)
  {
    fclose(file);
    return true;
  }

  return false;
}

bool DirectoryExists(StringParam filePath)
{
  std::error_code error;
  return fs::status(filePath.c_str(), error).type() == fs::file_type::directory;
}

String CanonicalizePath(StringParam directoryPath)
{
  std::error_code error;
  return fs::canonical(directoryPath.c_str(), error).u8string().c_str();
}

bool PathIsRooted(StringParam directoryPath)
{
  return fs::path(directoryPath.c_str()).has_root_directory();
}

void CreateDirectory(StringParam dest)
{
  std::error_code error;
  fs::create_directory(dest.c_str(), error);
}

void CreateDirectoryAndParents(StringParam directory)
{
  std::error_code error;
  fs::create_directories(directory.c_str(), error);
}

bool CopyFileInternal(StringParam dest, StringParam source)
{
  std::error_code error;
  fs::copy_file(source.c_str(), dest.c_str(), fs::copy_options::overwrite_existing, error);
  return !(bool)error;
}

bool MoveFileInternal(StringParam dest, StringParam source)
{
  std::error_code error;
  fs::rename(source.c_str(), dest.c_str(), error);
  return !(bool)error;
}

bool DeleteFileInternal(StringParam file)
{
  std::error_code error;
  fs::remove(file.c_str(), error);
  return !(bool)error;
}

bool DeleteDirectory(StringParam directory)
{
  std::error_code error;
  fs::remove_all(directory.c_str(), error);
  return !(bool)error;
}

TimeType GetFileModifiedTime(StringParam filename)
{
  std::error_code error;
  fs::file_time_type time = fs::last_write_time(filename.c_str(), error);
  auto convertedTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(time - fs::file_time_type::clock::now()
              + std::chrono::system_clock::now());
  return std::chrono::system_clock::to_time_t(convertedTime);
}

bool SetFileToCurrentTime(StringParam filename)
{
  // Not the greatest way to set the last write time...
  FILE* file = fopen(filename.c_str(), "a");
  if (file)
  {
    fclose(file);
    return true;
  }
  return false;
}

u64 GetFileSize(StringParam fileName)
{
  std::error_code error;
  return (u64)fs::file_size(fileName.c_str(), error);
}

struct FileRangePrivateData
{
  fs::directory_iterator mIterator;
  fs::directory_iterator mEnd;
};

FileRange::FileRange(StringParam filePath)
{
  PlasmaConstructPrivateData(FileRangePrivateData);
  mPath = filePath;
  if (mPath.Empty())
  {
    Error("Cannot create a file range from an empty directory/path string "
          "(working directory as empty string not supported)");
    return;
  }

  std::error_code error;
  fs::directory_iterator begin = fs::directory_iterator(filePath.c_str(), error);
  self->mIterator = begin;
  self->mEnd = fs::end(begin);
}

FileRange::~FileRange()
{
  PlasmaDestructPrivateData(FileRangePrivateData);
}

bool FileRange::Empty()
{
  PlasmaGetPrivateData(FileRangePrivateData);
  return self->mIterator == self->mEnd;
}

String FileRange::Front()
{
  PlasmaGetPrivateData(FileRangePrivateData);
  return self->mIterator->path().filename().u8string().c_str();
}

FileEntry FileRange::FrontEntry()
{
  PlasmaGetPrivateData(FileRangePrivateData);

  std::error_code error;
  FileEntry entry;
  entry.mFileName = Front();
  entry.mSize = (u64)fs::file_size(self->mIterator->path(), error);
  entry.mPath = mPath;
  return entry;
}

void FileRange::PopFront()
{
  PlasmaGetPrivateData(FileRangePrivateData);
  ++self->mIterator;
}

String UniqueFileId(StringParam fullpath)
{
  return CanonicalizePath(FilePath::Normalize(fullpath));
}

} // namespace Plasma
