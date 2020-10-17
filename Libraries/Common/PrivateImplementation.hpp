// MIT Licensed (see LICENSE.md).
#pragma once

// Make sure the private data is of a type that is aligned for proper native
// alignment (most likely pointer reading alignment). Still take the size in
// bytes and round up to the nearest whole value.
#define PlasmaDeclarePrivateDataBytes(SizeInBytes) MaxAlignmentType mPrivateData[PlasmaAlignCount(SizeInBytes)];

#define PlasmaDeclarePrivateData(Type, SizeInBytes)                                                                      \
  PlasmaDeclarePrivateDataBytes(SizeInBytes);                                                                            \
  Type(const Type& right)                                                                                              \
  {                                                                                                                    \
  }                                                                                                                    \
  Type& operator=(const Type& right)                                                                                   \
  {                                                                                                                    \
    return *this;                                                                                                      \
  }

// Gets the private object of a given pointer (doesn't assume self) and uses the
// provided variable name.
#define PlasmaGetObjectPrivateData(Type, pointer, name) Type* name = (Type*)(pointer)->mPrivateData;

#define PlasmaGetPrivateData(Type) Type* self = (Type*)mPrivateData;

#define PlasmaAssertPrivateDataSize(Type)                                                                                \
  static_assert(sizeof(Type) <= sizeof(mPrivateData),                                                                  \
                "Increase the size of the private data because the private "                                           \
                "type is too big");

// For completely pod private data, it's easier just to clear it out (no
// destructor)
#define PlasmaMemClearPrivateData(Type)                                                                                  \
  memset(mPrivateData, 0, sizeof(mPrivateData));                                                                       \
  PlasmaAssertPrivateDataSize(Type);                                                                                     \
  PlasmaGetPrivateData(Type);

#define PlasmaConstructPrivateData(Type, ...)                                                                            \
  Type* self = new (mPrivateData) Type();                                                                              \
  PlasmaAssertPrivateDataSize(Type);

#define PlasmaDestructPrivateData(Type, ...) ((Type*)mPrivateData)->~Type();
