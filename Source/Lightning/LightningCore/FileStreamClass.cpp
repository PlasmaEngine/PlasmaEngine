// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Lightning
{
LightningDefineExternalBaseType(FileMode::Enum, TypeCopyMode::ValueType, builder, type)
{
  LightningFullBindEnum(builder, type, SpecialType::Flags);
  LightningFullBindEnumValue(builder, type, FileMode::Read, "Read");
  LightningFullBindEnumValue(builder, type, FileMode::Write, "Write");
  LightningFullBindEnumValue(builder, type, FileMode::Append, "Append");
  LightningFullBindEnumValue(builder, type, FileMode::ShareRead, "ShareRead");
  LightningFullBindEnumValue(builder, type, FileMode::ShareWrite, "ShareWrite");
  LightningFullBindEnumValue(builder, type, FileMode::ShareDelete, "ShareDelete");
  LightningFullBindEnumValue(builder, type, FileMode::Sequential, "Sequential");
}

LightningDefineType(FileStreamClass, builder, type)
{
  // Even though this is an interface, because it is native, it must have a
  // constructor that can be implemented
  LightningFullBindDestructor(builder, type, FileStreamClass);
  LightningFullBindConstructor(builder, type, FileStreamClass, "filePath, mode", StringParam, FileMode::Enum);
  LightningFullBindConstructor(builder, type, FileStreamClass, nullptr);
  LightningBindConstructor(const FileStreamClass&);

  LightningFullBindMethod(builder, type, &FileStreamClass::Close, LightningNoOverload, "Close", nullptr);
}

FileStreamClass::FileStreamClass(StringParam filePath, FileMode::Enum mode)
{
  Plasma::FileShare::Enum plasmaShare = (Plasma::FileShare::Enum)0;
  Plasma::FileMode::Enum plasmaMode = Plasma::FileMode::Read;
  Plasma::FileAccessPattern::Enum plasmaAccessPattern = Plasma::FileAccessPattern::Random;

  bool read = (mode & FileMode::Read) != 0;
  bool write = (mode & FileMode::Write) != 0;
  bool append = (mode & FileMode::Append) != 0;

  if (append && read)
  {
    ExecutableState::CallingState->ThrowException("Cannot Append and Read from the same FileStreamClass");
    return;
  }

  // Translate our mode flags into the Plasma enum
  if (append)
    plasmaMode = Plasma::FileMode::Append;
  else if (read && write)
    plasmaMode = Plasma::FileMode::ReadWrite;
  else if (write)
    plasmaMode = Plasma::FileMode::Write;
  else
    plasmaMode = Plasma::FileMode::Read;

  // Enable any optimizations
  if (mode & FileMode::Sequential)
    plasmaAccessPattern = Plasma::FileAccessPattern::Sequential;

  // We always have these capabilities
  this->Capabilities = (StreamCapabilities::Enum)(StreamCapabilities::GetCount | StreamCapabilities::Seek);

  if (mode & FileMode::ShareRead)
    plasmaShare = (Plasma::FileShare::Enum)(plasmaShare | Plasma::FileShare::Read);
  if (mode & FileMode::ShareWrite)
    plasmaShare = (Plasma::FileShare::Enum)(plasmaShare | Plasma::FileShare::Write);
  if (mode & FileMode::ShareDelete)
    plasmaShare = (Plasma::FileShare::Enum)(plasmaShare | Plasma::FileShare::Delete);

  // Setup stream capabilities based on flags passed in
  if (read)
    this->Capabilities = (StreamCapabilities::Enum)(this->Capabilities | StreamCapabilities::Read);
  if (write)
    this->Capabilities = (StreamCapabilities::Enum)(this->Capabilities | StreamCapabilities::Write);

  // Create the path if it doesn't exist and we're in a writing file mode (all
  // of these should create the file according to the windows standard but
  // currently ReadWrite might not actually do this)
  if (plasmaMode == Plasma::FileMode::Write || plasmaMode == Plasma::FileMode::ReadWrite || plasmaMode == Plasma::FileMode::Append)
  {
    String directoryPath = Plasma::FilePath::GetDirectoryPath(filePath);
    CreateDirectoryAndParents(directoryPath);
  }

  // Open the file and throw an exception if we fail
  Status status;
  this->InternalFile.Open(filePath, plasmaMode, plasmaAccessPattern, plasmaShare, &status);
  if (status.Failed())
  {
    String message = String::Format("Unable to open the file '%s': %s", filePath.c_str(), status.Message.c_str());
    ExecutableState::CallingState->ThrowException(message);
  }
}

FileStreamClass::FileStreamClass()
{
  Capabilities = (StreamCapabilities::Enum)0;
}

FileStreamClass::FileStreamClass(const FileStreamClass& stream)
{
  // Duplicate the file handle
  Status status;
  const_cast<FileStreamClass&>(stream).InternalFile.Duplicate(status, InternalFile);

  Capabilities = stream.Capabilities;
}

FileStreamClass::~FileStreamClass()
{
  if (this->InternalFile.IsOpen())
    Close();
}

StreamCapabilities::Enum FileStreamClass::GetCapabilities()
{
  return this->Capabilities;
}

DoubleInteger FileStreamClass::GetPosition()
{
  return this->InternalFile.Tell();
}

DoubleInteger FileStreamClass::GetCount()
{
  return this->InternalFile.CurrentFileSize();
}

bool FileStreamClass::Seek(DoubleInteger position, StreamOrigin::Enum origin)
{
  return this->InternalFile.Seek(position, (Plasma::SeekOrigin::Enum)origin);
}

Integer FileStreamClass::Write(ArrayClass<Byte>& data, Integer byteStart, Integer byteCount)
{
  IStreamClass::Write(data, byteStart, byteCount);
  if (ExecutableState::GetCallingReport().HasThrownExceptions())
    return 0;

  return (Integer)this->InternalFile.Write(data.NativeArray.Data() + byteStart, (size_t)byteCount);
}

Integer FileStreamClass::WriteByte(Byte byte)
{
  return (Integer)this->InternalFile.Write(&byte, 1);
}

Integer FileStreamClass::Read(ArrayClass<Byte>& data, Integer byteStart, Integer byteCount)
{
  Status status;
  IStreamClass::Read(data, byteStart, byteCount);
  if (ExecutableState::GetCallingReport().HasThrownExceptions())
    return 0;

  return (Integer)this->InternalFile.Read(status, data.NativeArray.Data() + byteStart, (size_t)byteCount);
}

Integer FileStreamClass::ReadByte()
{
  Status status;
  IStreamClass::ReadByte();
  if (ExecutableState::GetCallingReport().HasThrownExceptions())
    return 0;

  // Read a single byte
  Byte byte = 0;
  size_t bytesRead = this->InternalFile.Read(status, &byte, 1);

  // If we didn't read anything, then return -1 (we're returning an Integer)
  if (bytesRead == 0)
    return -1;

  // Otherwise, return just the byte
  return (Integer)byte;
}

void FileStreamClass::Flush()
{
  this->InternalFile.Flush();
}

void FileStreamClass::Close()
{
  this->InternalFile.Close();
}

} // namespace Lightning
