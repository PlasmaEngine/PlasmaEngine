// MIT Licensed (see LICENSE.md).

#pragma once
#ifndef LIGHTNING_TRAITS_HPP
#  define LIGHTNING_TRAITS_HPP

namespace Lightning
{
template <typename T>
class PlasmaSharedTemplate StaticDereference
{
public:
  typedef T Type;
};

template <typename T>
class PlasmaSharedTemplate StaticDereference<T*>
{
public:
  typedef T Type;
};

// Tells us whether a type is a primitive (built in type)
template <typename T>
class PlasmaSharedTemplate IsPrimitive
{
public:
  static const bool Value = false;
  typedef void FalseType;
};

// Mark all the basic types that we know of as primtiive
template <>
class PlasmaShared IsPrimitive<bool>
{
public:
  static const bool Value = true;
};
template <>
class PlasmaShared IsPrimitive<float>
{
public:
  static const bool Value = true;
};
template <>
class PlasmaShared IsPrimitive<double>
{
public:
  static const bool Value = true;
};
template <>
class PlasmaShared IsPrimitive<char>
{
public:
  static const bool Value = true;
};
template <>
class PlasmaShared IsPrimitive<signed char>
{
public:
  static const bool Value = true;
};
template <>
class PlasmaShared IsPrimitive<unsigned char>
{
public:
  static const bool Value = true;
};
template <>
class PlasmaShared IsPrimitive<signed short>
{
public:
  static const bool Value = true;
};
template <>
class PlasmaShared IsPrimitive<unsigned short>
{
public:
  static const bool Value = true;
};
template <>
class PlasmaShared IsPrimitive<signed int>
{
public:
  static const bool Value = true;
};
template <>
class PlasmaShared IsPrimitive<unsigned int>
{
public:
  static const bool Value = true;
};
template <>
class PlasmaShared IsPrimitive<signed long>
{
public:
  static const bool Value = true;
};
template <>
class PlasmaShared IsPrimitive<unsigned long>
{
public:
  static const bool Value = true;
};
template <>
class PlasmaShared IsPrimitive<signed long long>
{
public:
  static const bool Value = true;
};
template <>
class PlasmaShared IsPrimitive<unsigned long long>
{
public:
  static const bool Value = true;
};
} // namespace Lightning

#endif
