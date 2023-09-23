// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Text.hpp"
#include "StringConversion.hpp"
#include "File.hpp"
#include "CharacterTraits.hpp"
#include "Core/Support/FileSupport.hpp"
#include "SerializationUtility.hpp"
#include "ToString.hpp"

namespace Plasma
{

TextSaver::TextSaver()
{
  mMode = SerializerMode::Saving;
  mSerializerType = SerializerType::Text;
  mVersion = DataVersion::Current;
}

TextSaver::~TextSaver()
{
  Close();
}

void TextSaver::Open(Status& status, cstr file, DataVersion::Enum version, FileMode::Enum fileMode)
{
  ErrorIf(fileMode != FileMode::Write && fileMode != FileMode::Append, "FileMode must be Write or Append.");

  Close();
  SetFlags();
  mFilename = file;
  mVersion = version;
  mWriteMode = fileMode;

  // Save out the file version if we're not in the legacy version
  // If we're appending, the version will be written out when the file is opened
  // if the file didn't exist
  if (fileMode == FileMode::Write && version != DataVersion::Legacy)
    SaveFileVersion();
}

bool TextSaver::OpenBuffer(DataVersion::Enum version, FileMode::Enum fileMode)
{
  ErrorIf(fileMode != FileMode::Write && fileMode != FileMode::Append, "FileMode must be Write or Append.");

  Close();
  SetFlags();
  mVersion = version;
  mWriteMode = fileMode;

  // Save out the file version if we're not in the legacy version
  if (fileMode == FileMode::Write && version != DataVersion::Legacy)
    SaveFileVersion();

  return true;
}

uint TextSaver::GetBufferSize()
{
  return mStream.GetSize();
}

String TextSaver::GetString()
{
  return mStream.ToString();
}

SerializerClass::Enum TextSaver::GetClass()
{
  return SerializerClass::TextSaver;
}

void TextSaver::ExtractInto(::byte* data, uint size)
{
  mStream.ExtractInto(data, size);
}

DataBlock TextSaver::ExtractAsDataBlock()
{
  uint size = mStream.GetSize();
  DataBlock block = AllocateBlock(size);
  mStream.ExtractInto(block.Data, block.Size);
  return block;
}

void TextSaver::Close()
{
  if (!mFilename.Empty())
  {
    File file;

    bool opened = file.Open(mFilename.c_str(), mWriteMode, FileAccessPattern::Sequential);
    ErrorIf(!opened, "Failed to open file for text output");
    if (opened)
    {
      // We only want the file version to save out once. We've deferred the
      // responsibility of saving out the file version to here, where we need
      // to check if there is anything in the file
      if (mWriteMode == FileMode::Append && file.Tell() == 0)
      {
        String data = mStream.ToString();
        mStream.Deallocate();
        SaveFileVersion();
        mStream.Append(data);
      }

      ByteBuffer::BlockRange blocks = mStream.Blocks();
      for (; !blocks.Empty(); blocks.PopFront())
      {
        file.Write(blocks.Front().Data, blocks.Front().Size);
      }
    }

    mFilename = String();
  }
  mStream.Deallocate();
}

bool TextSaver::InnerStart(cstr typeName, cstr fieldName, StructType structType, bool ignoreTabs)
{
  bool field = false;
  if (fieldName)
  {
    field = true;

    if (!ignoreTabs)
      Tabs();

    if (mVersion == DataVersion::Legacy)
    {
      if (typeName)
      {
        mStream << typeName;
        mStream << " ";
      }
    }
    else
    {
      mStream << "var ";
    }

    if (*fieldName == 'm')
      ++fieldName;
    mStream << fieldName;

    mStream << " = ";
  }

  if (mVersion == DataVersion::Legacy)
  {
    switch (structType)
    {
    case StructureType::Object:
      if (field)
        mStream << "\n";
      Tabs();
      mStream << "{\n";
      ++mDepth;
      break;
    case StructureType::Array:
      if (field)
        mStream << "\n";
      Tabs();
      mStream << "[\n";
      ++mDepth;
      break;
    case StructureType::BasicArray:
      mStream << "[";
      break;
    case StructureType::Value:
    default:
      if (!field)
        Tabs();
      break;
    }
  }
  else
  {
    switch (structType)
    {
    case StructureType::Object:
    case StructureType::Array:
      if (typeName)
      {
        if (fieldName == nullptr && !ignoreTabs)
          Tabs();
        mStream << typeName;
        if (fieldName == nullptr)
          mStream << "\n";
      }
      if (field)
        mStream << "\n";
      Tabs();
      mStream << "{\n";
      ++mDepth;
      break;
    case StructureType::BasicArray:
      if (typeName)
        mStream << typeName;
      mStream << "{";
      break;
    case StructureType::Value:
    default:
      if (!field)
        Tabs();
      break;
    }
  }

  return true;
}

void TextSaver::Tabs()
{
  for (uint i = 0; i < mDepth; ++i)
    mStream << "\t";
}

void TextSaver::InnerEnd(cstr typeName, StructType structType)
{
  if (mVersion == DataVersion::Legacy)
  {
    switch (structType)
    {
    case StructureType::Object:
      --mDepth;
      Tabs();
      mStream << "},\n";
      break;
    case StructureType::Array:
      --mDepth;
      Tabs();
      mStream << "],\n";
      break;
    case StructureType::BasicArray:
      mStream << "],\n";
      break;
    case StructureType::Value:
    default:
      mStream << ",\n";
      break;
    }
  }
  else
  {
    switch (structType)
    {
    case StructureType::Object:
      --mDepth;
      Tabs();
      mStream << "}\n";
      break;
    case StructureType::Array:
      --mDepth;
      Tabs();
      mStream << "}\n";
      break;
    case StructureType::BasicArray:
      mStream << "}\n";
      break;
    case StructureType::Value:
    default:
      mStream << "\n";
      break;
    }
  }
}

bool TextSaver::SimpleField(cstr typeName, cstr fieldName, StringRange& stringRange)
{
  Start(typeName, fieldName, StructureType::Value);

  // Create a temporary string range
  StringRange range = stringRange;

  // Loop through all characters in the string
  while (range.Empty() == false)
  {
    // If the current character is a quote....
    if (range.Front() == '"')
    {
      // Append two quotes
      mStream.Append("\"\"");
    }
    else
    {
      // Otherwise, just Append the character value
      mStream.Append(range.Front());
    }

    // Pop the front
    range.PopFront();
  }

  End(typeName, StructureType::Value);
  return true;
}

bool TextSaver::StringField(cstr typeName, cstr fieldName, StringRange& stringRange)
{
  Start(typeName, fieldName, StructureType::Value);
  // Start with a "
  mStream.Append("\"");

  StringRange range = stringRange;

  // Escape slashes and quotes. The data tree tokenizer needs an un-escaped
  // quote at the end to properly detect the end of the string property We're
  // only doing this for newer formats (we can't do older formats because the
  // loader would need to handle it).
  if (mVersion != DataVersion::Legacy)
  {
    while (range.Empty() == false)
    {
      if (range.Front() == '"' || range.Front() == '\\')
        mStream.Append("\\");

      mStream.Append(range.Front());

      range.PopFront();
    }
  }
  else
  {
    while (range.Empty() == false)
    {
      if (range.Front() == '"')
        mStream.Append("\"\"");
      else
        mStream.Append(range.Front());

      range.PopFront();
    }
  }

  // End with a "
  mStream.Append("\"");

  End(typeName, StructureType::Value);
  return true;
}

void TextSaver::StartPolymorphicInternal(const PolymorphicInfo& info)
{
  Tabs();

  bool additive = info.mFlags.IsSet(PolymorphicSaveFlags::LocallyAdded);
  bool orderOverride = info.mFlags.IsSet(PolymorphicSaveFlags::ChildOrderOverride);

  if (mVersion == DataVersion::Legacy)
  {
    // Signify that the node was added
    if (additive)
      mStream << "+ ";
    mStream << info.mTypeName;

    // Node id
    if (info.mUniqueNodeId != PolymorphicNode::cInvalidUniqueNodeId)
    {
      mStream << ":";
      mStream << ToString(info.mUniqueNodeId);
    }

    // Inherit id
    if (!info.mInheritanceId.Empty())
    {
      mStream << " (\"";
      mStream << info.mInheritanceId;
      mStream << "\")";
    }

    mStream << " = ";
  }
  else
  {
    ErrorIf(strcmp(info.mTypeName, "BoundType") == 0,
            "StartPolymorphic was passed a BoundType*"
            "instead of an object instance.");
    mStream << info.mTypeName;

    // Add a space to separate the type with attributes. This isn't really
    // needed, but makes things look more visually appealing in the file
    mStream << " ";

    // Save out Attributes
    if (info.mUniqueNodeId != PolymorphicNode::cInvalidUniqueNodeId)
      SaveAttribute(SerializationAttributes::Id, ToString(info.mUniqueNodeId));
    if (!info.mInheritanceId.Empty())
      SaveAttribute(SerializationAttributes::InheritId, info.mInheritanceId, true);
    if (additive)
      SaveAttribute(SerializationAttributes::LocallyAdded);
    if (orderOverride)
      SaveAttribute(SerializationAttributes::ChildOrderOverride);

    Handle object = info.mObject;
    if (object.StoredType)
    {
      if (MetaSerialization* metaSerialization = object.StoredType->HasInherited<MetaSerialization>())
        metaSerialization->AddCustomAttributes(object, this);
    }
  }

  mStream << "\n";

  Tabs();
  mStream << "{\n";
  ++mDepth;
}

void TextSaver::EndPolymorphic()
{
  End((cstr) nullptr, StructureType::Object);
}

void TextSaver::AddSubtractivePolymorphicNode(cstr typeName, Guid nodeId)
{
  if (mVersion == DataVersion::Legacy)
  {
    // Example:
    // -Model,
    Tabs();
    mStream << "- ";
    mStream << typeName;
    if (nodeId != PolymorphicNode::cInvalidUniqueNodeId)
    {
      mStream << ":";
      mStream << ToString(nodeId);
    }
    mStream << ",\n";
  }
  else
  {
    /* Example:

      Model [LocallyRemoved]
      {
      }

    */
    Tabs();
    mStream << typeName;

    // Space before attributes looks nice
    mStream << ' ';

    if (nodeId != PolymorphicNode::cInvalidUniqueNodeId)
      SaveAttribute("Id", ToString(nodeId));
    SaveAttribute("LocallyRemoved");
    mStream << "\n";

    // Open and close parenthesis
    Tabs();
    mStream << "{\n";
    Tabs();
    mStream << "}\n";
  }
}

bool TextSaver::GetPolymorphic(PolymorphicNode& node)
{
  ErrorIf(true, "Serializer is a saver.");
  return false;
}

template <typename type>
bool WriteArrayText(StringBuilder& os, type* data, uint numberOfElements)
{
  bool result = true;

  for (uint i = 0; i < numberOfElements; ++i)
  {
    type& value = data[i];

    // Make sure its not NAN/IND/INF
    result &= CorrectNonFiniteValues(value);

    os << data[i];
    if (i != numberOfElements - 1)
      os << ", ";
  }

  return result;
}

bool TextSaver::ArrayField(
    cstr typeName, cstr fieldName, ::byte* data, ArrayType arrayType, uint numberOfElements, uint sizeOftype)
{
  bool result = true;

  InnerStart(typeName, fieldName, StructureType::BasicArray);
  switch (arrayType)
  {
  case BasicArrayType::Float:
  {
    result = WriteArrayText<float>(mStream, (float*)data, numberOfElements);
  }
  break;

  case BasicArrayType::Integer:
  {
    result = WriteArrayText<int>(mStream, (int*)data, numberOfElements);
  }
  break;

  default:
  {
    ErrorIf(true, "Can not serialize type.");
    result = false;
  }
  break;
  }

  InnerEnd(typeName, StructureType::BasicArray);
  return result;
}

bool TextSaver::EnumField(cstr enumTypeName, cstr fieldName, uint& enumValue, BoundType* type)
{
  InnerStart(enumTypeName, fieldName, StructureType::Value);
  Array<String>& strings = type->EnumValueToStrings[(Integer)enumValue];

  if (strings.Empty())
  {
    mStream << enumValue;
    InnerEnd(enumTypeName, StructureType::Value);
    return true;
  }

  String stringValue = strings.Front();

  if (mVersion == DataVersion::Legacy)
  {
    mStream << stringValue;
  }
  else
  {
    mStream << enumTypeName;
    mStream << ".";
    mStream << stringValue;
  }
  InnerEnd(enumTypeName, StructureType::Value);
  return true;
}

void TextSaver::SaveAttribute(StringParam name, StringParam value, bool stringValue)
{
  mStream << "[";
  mStream << name;
  if (!value.Empty())
  {
    mStream << ":";
    if (stringValue)
    {
      mStream << "\"";
      mStream << value;
      mStream << "\"";
    }
    else
    {
      mStream << value;
    }
  }
  mStream << "]";
}

void TextSaver::SaveFileVersion()
{
  SaveAttribute("Version", ToString((uint)mVersion));
  mStream << "\n";
}

void TextSaver::SetFlags()
{
  mDepth = 0;
}

} // namespace Plasma
