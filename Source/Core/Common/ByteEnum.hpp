// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

// Used to make enum members only take 1 byte
// Needed for non-C++11 support
template <typename EnumType>
class ByteEnum
{
public:
  ByteEnum()
  {
  }
  ByteEnum(EnumType value) : mValue(value)
  {
  }
  ByteEnum& operator=(EnumType value)
  {
    mValue = value;
    return *this;
  }

  operator EnumType() const
  {
    return (const EnumType)mValue;
  }

  ::byte mValue;
};

#define DeclareByteEnumGetSet(enumType, name)                                                                          \
  ByteEnum<enumType> m##name;                                                                                          \
  enumType Get##name()                                                                                                 \
  {                                                                                                                    \
    return m##name;                                                                                                    \
  }                                                                                                                    \
  void Set##name(enumType value)                                                                                       \
  {                                                                                                                    \
    m##name = value;                                                                                                   \
  }

} // namespace Plasma
