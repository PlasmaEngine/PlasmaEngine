// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Lightning
{
LightningDefineType(FilePathClass, builder, type)
{
  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::CombineDirectories,
                      (String(*)(StringParam, StringParam)),
                      "CombineDirectories",
                      "dir0, dir1")
      ->Description = FilePathClass::CombineDirectoriesDocumentation();
  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::CombineDirectories,
                      (String(*)(StringParam, StringParam, StringParam)),
                      "CombineDirectories",
                      "dir0, dir1, dir2")
      ->Description = FilePathClass::CombineDirectoriesDocumentation();
  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::CombineDirectories,
                      (String(*)(StringParam, StringParam, StringParam, StringParam)),
                      "CombineDirectories",
                      "dir0, dir1, dir2, dir3")
      ->Description = FilePathClass::CombineDirectoriesDocumentation();
  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::CombineDirectories,
                      (String(*)(StringParam, StringParam, StringParam, StringParam, StringParam)),
                      "CombineDirectories",
                      "dir0, dir1, dir2, dir3, dir4")
      ->Description = FilePathClass::CombineDirectoriesDocumentation();

  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::CombineDirectoriesAndFile,
                      (String(*)(StringParam, StringParam)),
                      "CombineDirectoriesAndFile",
                      "dir0, fileName")
      ->Description = FilePathClass::CombineDirectoriesAndFileDocumentation();
  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::CombineDirectoriesAndFile,
                      (String(*)(StringParam, StringParam, StringParam)),
                      "CombineDirectoriesAndFile",
                      "dir0, dir1, fileName")
      ->Description = FilePathClass::CombineDirectoriesAndFileDocumentation();
  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::CombineDirectoriesAndFile,
                      (String(*)(StringParam, StringParam, StringParam, StringParam)),
                      "CombineDirectoriesAndFile",
                      "dir0, dir1, dir2, fileName")
      ->Description = FilePathClass::CombineDirectoriesAndFileDocumentation();
  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::CombineDirectoriesAndFile,
                      (String(*)(StringParam, StringParam, StringParam, StringParam, StringParam)),
                      "CombineDirectoriesAndFile",
                      "dir0, dir1, dir2, dir3, fileName")
      ->Description = FilePathClass::CombineDirectoriesAndFileDocumentation();

  //// Bind the array version of Combine
  // Array<Type*> arrayElement;
  // arrayElement.PushBack(LightningTypeId(String));
  // LibraryArray coreArray;
  // coreArray.PushBack(Core::GetInstance().GetBuilder()->BuiltLibrary);
  // BoundType* arrayOfStrings = builder.InstantiateTemplate("Array",
  // arrayElement, coreArray).Type; Function* combineFunction =
  // builder.AddBoundFunction(type, "Combine", &FilePathClass::Combine,
  // OneParameter(arrayOfStrings, "parts"), LightningTypeId(String),
  // FunctionOptions::Static); combineFunction->Description =
  // CombineDocumentation();

  LightningFullBindGetterSetter(builder,
                            type,
                            &FilePathClass::GetDirectorySeparator,
                            LightningNoOverload,
                            LightningNoSetter,
                            LightningNoOverload,
                            "DirectorySeparator")
      ->Description = FilePathClass::DirectorySeparatorDocumentation();

  LightningFullBindMethod(builder, type, &FilePathClass::ChangeExtension, LightningNoOverload, "ChangeExtension", LightningNoNames)
      ->Description = FilePathClass::ChangeExtensionDocumentation();
  LightningFullBindMethod(
      builder, type, &FilePathClass::GetExtensionWithDot, LightningNoOverload, "GetExtensionWithDot", LightningNoNames)
      ->Description = FilePathClass::GetExtensionWithDotDocumentation();
  LightningFullBindMethod(
      builder, type, &FilePathClass::GetExtensionWithoutDot, LightningNoOverload, "GetExtensionWithoutDot", LightningNoNames)
      ->Description = FilePathClass::GetExtensionWithoutDotDocumentation();
  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::GetFileNameWithExtension,
                      LightningNoOverload,
                      "GetFileNameWithExtension",
                      LightningNoNames)
      ->Description = FilePathClass::GetFileNameWithExtensionDocumentation();
  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::GetFileNameWithoutExtension,
                      LightningNoOverload,
                      "GetFileNameWithoutExtension",
                      LightningNoNames)
      ->Description = FilePathClass::GetFileNameWithoutExtensionDocumentation();

  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::AddTrailingDirectorySeparator,
                      LightningNoOverload,
                      "AddTrailingDirectorySeparator",
                      LightningNoNames)
      ->Description = FilePathClass::AddTrailingDirectorySeparatorDocumentation();
  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::RemoveTrailingDirectorySeparator,
                      LightningNoOverload,
                      "RemoveTrailingDirectorySeparator",
                      LightningNoNames)
      ->Description = FilePathClass::RemoveTrailingDirectorySeparatorDocumentation();

  LightningFullBindMethod(
      builder, type, &FilePathClass::GetDirectoryPath, LightningNoOverload, "GetDirectoryPath", LightningNoNames)
      ->Description = FilePathClass::GetDirectoryPathDocumentation();
  LightningFullBindMethod(
      builder, type, &FilePathClass::GetDirectoryName, LightningNoOverload, "GetDirectoryName", LightningNoNames)
      ->Description = FilePathClass::GetDirectoryNameDocumentation();
  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::GetCanonicalizedPathFromAbsolutePath,
                      LightningNoOverload,
                      "GetCanonicalizedPathFromAbsolutePath",
                      LightningNoNames)
      ->Description = FilePathClass::GetCanonicalizedPathFromAbsolutePathDocumentation();
  LightningFullBindMethod(builder,
                      type,
                      &FilePathClass::GetComparablePathFromAbsolutePath,
                      LightningNoOverload,
                      "GetComparablePathFromAbsolutePath",
                      LightningNoNames)
      ->Description = FilePathClass::GetComparablePathFromAbsolutePathDocumentation();
  LightningFullBindMethod(builder, type, &FilePathClass::IsRelative, LightningNoOverload, "IsRelative", LightningNoNames)
      ->Description = FilePathClass::IsRelativeDocumentation();

  LightningFullBindGetterSetter(builder,
                            type,
                            &FilePathClass::GetWorkingDirectory,
                            LightningNoOverload,
                            &FilePathClass::SetWorkingDirectory,
                            LightningNoOverload,
                            "WorkingDirectory")
      ->Description = FilePathClass::WorkingDirectoryDocumentation();
  LightningFullBindGetterSetter(builder,
                            type,
                            &FilePathClass::GetTemporaryDirectory,
                            LightningNoOverload,
                            LightningNoSetter,
                            LightningNoOverload,
                            "TemporaryDirectory")
      ->Description = FilePathClass::TemporaryDirectoryDocumentation();
  LightningFullBindGetterSetter(builder,
                            type,
                            &FilePathClass::GetUserLocalDirectory,
                            LightningNoOverload,
                            LightningNoSetter,
                            LightningNoOverload,
                            "UserLocalDirectory")
      ->Description = FilePathClass::UserLocalDirectoryDocumentation();

  LightningFullBindGetterSetter(builder,
                            type,
                            &FilePathClass::GetUserDocumentsDirectory,
                            LightningNoOverload,
                            LightningNoSetter,
                            LightningNoOverload,
                            "UserDocumentsDirectory")
      ->Description = FilePathClass::UserDocumentsDirectoryDocumentation();
  LightningFullBindGetterSetter(builder,
                            type,
                            &FilePathClass::GetExecutableDirectory,
                            LightningNoOverload,
                            LightningNoSetter,
                            LightningNoOverload,
                            "ExecutableDirectory")
      ->Description = FilePathClass::ExecutableDirectoryDocumentation();
  LightningFullBindGetterSetter(builder,
                            type,
                            &FilePathClass::GetExecutableFile,
                            LightningNoOverload,
                            LightningNoSetter,
                            LightningNoOverload,
                            "ExecutableFile")
      ->Description = FilePathClass::ExecutableFileDocumentation();
}

String FilePathClass::CombineDirectories(StringParam dir0, StringParam dir1)
{
  return AddTrailingDirectorySeparator(Plasma::FilePath::Combine(dir0, dir1));
}

String FilePathClass::CombineDirectories(StringParam dir0, StringParam dir1, StringParam dir2)
{
  return AddTrailingDirectorySeparator(Plasma::FilePath::Combine(dir0, dir1, dir2));
}

String FilePathClass::CombineDirectories(StringParam dir0, StringParam dir1, StringParam dir2, StringParam dir3)
{
  return AddTrailingDirectorySeparator(Plasma::FilePath::Combine(dir0, dir1, dir2, dir3));
}

String FilePathClass::CombineDirectories(
    StringParam dir0, StringParam dir1, StringParam dir2, StringParam dir3, StringParam dir4)
{
  return AddTrailingDirectorySeparator(Plasma::FilePath::Combine(dir0, dir1, dir2, dir3, dir4));
}

String FilePathClass::CombineDirectoriesAndFile(StringParam dir0, StringParam fileName)
{
  return Plasma::FilePath::Combine(dir0, fileName);
}

String FilePathClass::CombineDirectoriesAndFile(StringParam dir0, StringParam dir1, StringParam fileName)
{
  return Plasma::FilePath::Combine(dir0, dir1, fileName);
}

String
FilePathClass::CombineDirectoriesAndFile(StringParam dir0, StringParam dir1, StringParam dir2, StringParam fileName)
{
  return Plasma::FilePath::Combine(dir0, dir1, dir2, fileName);
}

String FilePathClass::CombineDirectoriesAndFile(
    StringParam dir0, StringParam dir1, StringParam dir2, StringParam dir3, StringParam fileName)
{
  return Plasma::FilePath::Combine(dir0, dir1, dir2, dir3, fileName);
}

String FilePathClass::GetDirectorySeparator()
{
  static String DirectorySeparator(Plasma::cDirectorySeparatorCstr);
  return DirectorySeparator;
}

String FilePathClass::ChangeExtension(StringParam path, StringParam extension)
{
  // First we get the last dot in the path (it may not exist)
  StringRange pathLastDot = path.FindLastOf('.');
  StringRange pathRangeWithoutExtension = path.All();

  // If the last dot exists, then adjust the string range to not include it at
  // all
  /*if (0 != StringRange::InvalidIndex) {}*/
  // pathRangeWithoutExtension.end = pathRangeWithoutExtension.begin +
  // pathLastDot;

  // Check if the first character is a .'. so we know whether the user passed in
  // '.jpeg' vs just 'jpeg'
  StringRange extensionRangeWithoutLeadingDot = extension.All();
  if (extension.Empty() == false && extension.Front() == '.')
    extensionRangeWithoutLeadingDot.IncrementByRune();

  // Concatenate the path and extension together (both should not have the dot,
  // so add it in ourselves)
  return BuildString(pathRangeWithoutExtension, ".", extensionRangeWithoutLeadingDot);
}

String FilePathClass::GetExtensionWithDot(StringParam path)
{
  // This gets the extension without the dot '.'
  // If there is no extension, our function is defined to return nothing
  StringRange extension = Plasma::FilePath::GetExtension(path);
  if (extension.Empty())
    return extension;

  // Prepend the dot and return the extension
  return BuildString(".", extension);
}

String FilePathClass::GetExtensionWithoutDot(StringParam path)
{
  // This gets the extension without the dot '.' always
  return Plasma::FilePath::GetExtension(path);
}

String FilePathClass::GetFileNameWithExtension(StringParam path)
{
  return Plasma::FilePath::GetFileName(path);
}

String FilePathClass::GetFileNameWithoutExtension(StringParam path)
{
  return Plasma::FilePath::GetFileNameWithoutExtension(path);
}

String FilePathClass::AddTrailingDirectorySeparator(StringParam path)
{
  // First check if the path already ends in a directory separator, if not add
  // it in
  StringRange directorySeparator(Plasma::cDirectorySeparatorCstr);
  if (path.EndsWith(directorySeparator))
    return path;
  else
    return BuildString(path, Plasma::cDirectorySeparatorCstr);
}

String FilePathClass::RemoveTrailingDirectorySeparator(StringParam path)
{
  // First check if the path already ends in a directory separator, if it does
  // then remove it
  StringRange directorySeparator(Plasma::cDirectorySeparatorCstr);
  if (path.EndsWith(directorySeparator))
  {
    StringRange pathRange = path.All();
    //--pathRange.end; TODO DANE this is not properly supported
    return pathRange;
  }
  else
  {
    return path;
  }
}

String FilePathClass::GetDirectoryPath(StringParam path)
{
  // This function does not include the trailing directory separator
  StringRange directoryPath = Plasma::FilePath::GetDirectoryPath(path);

  // Increment the end to include the path separator (if its within the range of
  // the original path we passed in)
  if (directoryPath.Begin() >= path.Begin() && directoryPath.End() < path.End())
    ++directoryPath.End();

  return directoryPath;
}

String FilePathClass::GetDirectoryName(StringParam path)
{
  return Plasma::FilePath::GetDirectoryName(path);
}

String FilePathClass::GetCanonicalizedPathFromAbsolutePath(StringParam absolutePath)
{
  // We want to check if it has an ending separator because our
  // FilePathClass::Normalize removes it (but we want it to be added back in)
  bool hasEndingSeparator = false;
  if (absolutePath.SizeInBytes() > 0 && (absolutePath.Back() == '\\' || absolutePath.Back() == '/'))
    hasEndingSeparator = true;

  // Our path normalizationg changes all slashes to be the OS slashes, and
  // removes redudant slashes
  String normalized = Plasma::FilePath::Normalize(absolutePath);
  if (hasEndingSeparator)
    normalized = BuildString(normalized, Plasma::cDirectorySeparatorCstr);

  // Let the operating system specific behavior canonicalize the path
  String canonicalized = Plasma::CanonicalizePath(normalized);
  return canonicalized;
}

String FilePathClass::GetComparablePathFromAbsolutePath(StringParam path)
{
  // First do path normaliziation and canonicalization
  String comparablePath = GetCanonicalizedPathFromAbsolutePath(path);

  // If the current file system is case insensative, then technically
  // "Player.png" should compare the same as "player.PNG" To make strings
  // properly comparable, we make them all lowercase
  if (!Plasma::cFileSystemCaseSensitive)
    comparablePath = comparablePath.ToLower();
  return comparablePath;
}

bool FilePathClass::IsRelative(StringParam path)
{
  // Return that empty paths are relative (it doesn't really matter what we do
  // here)
  if (path.Empty())
    return true;

  // If we have the unix 'root directory', then its not relative
  if (path.Front() == '/')
    return false;

  // Look for a windows network or UNC path
  if (path.StartsWith("\\\\"))
    return false;

  // If we find a ':' character then we assume its rooted because that is
  // otherwise an invalid character This may fail on older operating systems
  // (we'll have to let this run wild and see what people say)
  if (!path.FindFirstOf(':').Empty())
    return false;

  // Otherwise, assume its relative if we got here!
  return true;
}

String FilePathClass::GetWorkingDirectory()
{
  return AddTrailingDirectorySeparator(Plasma::GetWorkingDirectory());
}

void FilePathClass::SetWorkingDirectory(StringParam path)
{
  Plasma::SetWorkingDirectory(path);
}

String FilePathClass::GetTemporaryDirectory()
{
  return AddTrailingDirectorySeparator(Plasma::GetTemporaryDirectory());
}

String FilePathClass::GetUserLocalDirectory()
{
  return AddTrailingDirectorySeparator(Plasma::GetUserLocalDirectory());
}

String FilePathClass::GetUserDocumentsDirectory()
{
  return AddTrailingDirectorySeparator(Plasma::GetUserDocumentsDirectory());
}

String FilePathClass::GetExecutableDirectory()
{
  return AddTrailingDirectorySeparator(Plasma::GetApplicationDirectory());
}

String FilePathClass::GetExecutableFile()
{
  return Plasma::GetApplication();
}
} // namespace Lightning
