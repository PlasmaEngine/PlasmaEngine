// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

NativeType* LightningTypeToBasicNativeType(Lightning::Type* lightningType)
{
  // Empty type?
  if (lightningType == nullptr)
    return nullptr;

  // Boolean type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Bool>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Bool>::Type);

  // Integer type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Int32>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Int32>::Type);
  // DoubleInteger type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Int64>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Int64>::Type);

  // Byte type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Uint8>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Uint8>::Type);

  // Real type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Float>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Float>::Type);
  // DoubleReal type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Double>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Double>::Type);

  // Boolean2 type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::BoolVector2>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::BoolVector2>::Type);
  // Boolean3 type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::BoolVector3>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::BoolVector3>::Type);
  // Boolean4 type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::BoolVector4>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::BoolVector4>::Type);

  // Integer2 type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::IntVector2>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::IntVector2>::Type);
  // Integer3 type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::IntVector3>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::IntVector3>::Type);
  // Integer4 type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::IntVector4>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::IntVector4>::Type);

  // Real2 type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Vector2>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Vector2>::Type);
  // Real3 type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Vector3>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Vector3>::Type);
  // Real4 type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Vector4>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Vector4>::Type);

  // Quaternion type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Quaternion>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Quaternion>::Type);

  // Real3x3 type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Matrix3>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Matrix3>::Type);
  // Real4x4 type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Matrix4>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Matrix4>::Type);

  // String type?
  else if (lightningType == LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::String>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::String>::Type);

  // Other type?
  else
    return nullptr;
}

Lightning::Type* BasicNativeTypeToLightningType(NativeType* nativeType)
{
  // Get native type ID
  NativeTypeId nativeTypeId = nativeType ? nativeType->mTypeId : cInvalidNativeTypeId;

  // Get lightning type via native type ID
  return BasicNativeTypeToLightningType(nativeTypeId);
}
Lightning::Type* BasicNativeTypeToLightningType(NativeTypeId nativeTypeId)
{
  // Switch on native type ID
  switch (nativeTypeId)
  {
  // Other Types
  default:
    return nullptr;

  // Bool Type
  case BasicNativeType::Bool:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Bool>::Type); // Boolean type

  // Char Type
  case BasicNativeType::Char:
    return nullptr; // (Not bound to Lightning)

  // Fixed-Width Signed Integral Types
  case BasicNativeType::Int8:
    return nullptr; // (Not bound to Lightning)
  case BasicNativeType::Int16:
    return nullptr; // (Not bound to Lightning)
  case BasicNativeType::Int32:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Int32>::Type); // Integer type
  case BasicNativeType::Int64:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Int64>::Type); // DoubleInteger
                                                                               // type

  // Fixed-Width Unsigned Integral Types
  case BasicNativeType::Uint8:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Uint8>::Type); // Byte type
  case BasicNativeType::Uint16:
    return nullptr; // (Not bound to Lightning)
  case BasicNativeType::Uint32:
    return nullptr; // (Not bound to Lightning)
  case BasicNativeType::Uint64:
    return nullptr; // (Not bound to Lightning)

  // Floating Point Types
  case BasicNativeType::Float:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Float>::Type); // Real type
  case BasicNativeType::Double:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Double>::Type); // DoubleReal
                                                                                // type

  // Multi-Primitive Math Types
  case BasicNativeType::BoolVector2:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::BoolVector2>::Type); // Boolean2
                                                                                     // type
  case BasicNativeType::BoolVector3:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::BoolVector3>::Type); // Boolean3
                                                                                     // type
  case BasicNativeType::BoolVector4:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::BoolVector4>::Type); // Boolean4
                                                                                     // type
  case BasicNativeType::IntVector2:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::IntVector2>::Type); // Integer2
                                                                                    // type
  case BasicNativeType::IntVector3:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::IntVector3>::Type); // Integer3
                                                                                    // type
  case BasicNativeType::IntVector4:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::IntVector4>::Type); // Integer4
                                                                                    // type
  case BasicNativeType::Vector2:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Vector2>::Type); // Real2 type
  case BasicNativeType::Vector3:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Vector3>::Type); // Real3 type
  case BasicNativeType::Vector4:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Vector4>::Type); // Real4 type
  case BasicNativeType::Quaternion:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Quaternion>::Type); // Quaternion
                                                                                    // type
  case BasicNativeType::Matrix3:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Matrix3>::Type); // Real3x3
                                                                                 // type
  case BasicNativeType::Matrix4:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::Matrix4>::Type); // Real4x4
                                                                                 // type

  // String Type
  case BasicNativeType::String:
    return LightningTypeId(BasicNativeTypeFromEnum<BasicNativeType::String>::Type); // String type
  }
}

Variant ConvertBasicAnyToVariant(const Any& anyValue)
{
  // Any is empty?
  if (!anyValue.IsHoldingValue())
    return Variant();

  // Get basic native type from the any's stored type
  NativeType* nativeType = LightningTypeToBasicNativeType(anyValue.StoredType);
  if (!nativeType) // Unable? (The any's stored type is not a basic native
                   // type?)
    return Variant();

  // Get any's stored value data (may be null)
  const void* anyData = anyValue.Dereference();

  // Copy value to new variant
  // (If the data is null, this simply default constructs a value of the native
  // type)
  Variant result(nativeType, anyData);
  return result;
}
Any ConvertBasicVariantToAny(const Variant& variantValue)
{
  // Variant is empty?
  if (variantValue.IsEmpty())
    return Any();

  // Get lightning type from the variant's stored type
  Type* lightningType = BasicNativeTypeToLightningType(variantValue.GetNativeType());
  if (!lightningType) // Unable? (The variant's stored type is not a basic native
                  // type?)
    return Any();

  // Get variant's stored value data
  const void* variantData = variantValue.GetData();
  Assert(variantData);

  // This lightning type is meant to be passed by handle?
  if (lightningType->IsHandle())
  {
    // Create a handle containing our value data
    Handle handle((const byte*)variantData, Type::GetBoundType(lightningType));

    // Copy handle to new any
    Any result(handle);
    return result;
  }
  else
  {
    // Copy value to new any
    Any result((const byte*)variantData, lightningType);
    return result;
  }
}

} // namespace Plasma
