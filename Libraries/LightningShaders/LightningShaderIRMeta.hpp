// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class LightningShaderIRLibrary;
class ShaderIRTypeMeta;

DeclareEnum5(FragmentType, Vertex, Pixel, Geometry, Compute, None);

/// A typed string for looking up shader fields. This is a specific type to help
/// avoid ambiguities on HashMap<String, ShaderField*> where it's hard to know
/// what the string's value is. This type implies that the hashmap is not based
/// upon the field's name, but rather it's key (name:string).
struct ShaderFieldKey
{
  ShaderFieldKey(){};
  ShaderFieldKey(StringParam fieldName, StringParam fieldType);

  void Set(StringParam fieldName, StringParam fieldType);

  size_t Hash() const;
  bool operator==(const ShaderFieldKey& other) const;
  operator String() const;

  String mKey;
};

/// Base meta type of all shader meta objects. Currently only contains
/// attributes.
class ShaderIRMeta
{
public:
  bool ContainsAttribute(StringParam attributeName);
  ShaderIRAttributeList mAttributes;
};

/// Represents a field defined in lightning.
class ShaderIRFieldMeta : public ShaderIRMeta
{
public:
  ShaderIRFieldMeta();

  /// Make a key for this field. Used for hashing.
  ShaderFieldKey MakeFieldKey() const;
  /// Make a key for this field via the given attribute (for the field name).
  /// Used for hashing.
  ShaderFieldKey MakeFieldKey(ShaderIRAttribute* attribute) const;

  /// Given an attribute, return the field name for this variable. This returns
  /// the name in the attribute if it exists, otherwise the regular variable
  /// name.
  String GetFieldAttributeName(ShaderIRAttribute* attribute) const;

  /// Creates a clone of the current field meta. The given library will take
  /// ownership of the cloned field.
  ShaderIRFieldMeta* Clone(LightningShaderIRLibrary* owningLibrary) const;

  /// The name of the field in lightning.
  String mLightningName;
  /// The type of the field in lightning.
  Lightning::BoundType* mLightningType;
  /// The type that owns this field.
  ShaderIRTypeMeta* mOwner;

  /// The property that this field comes from.
  /// Useful for further reflection (such as finding the field's location).
  Lightning::Property* mLightningProperty;
  /// The default value of this field after evaluating lightning.
  /// Used for reflection to extract the default value that a user sets.
  Lightning::Any mDefaultValueVariant;
};

/// Represents a function defined in lightning.
class ShaderIRFunctionMeta : public ShaderIRMeta
{
public:
  ShaderIRFunctionMeta();

  /// The name of the lightning function.
  String mLightningName;
  /// The lightning function this comes from. Useful for more complicated
  /// reflection.
  Lightning::Function* mLightningFunction;
};

/// Represents a type defined in lightning.
class ShaderIRTypeMeta : public ShaderIRMeta
{
public:
  ShaderIRTypeMeta();

  /// Internal function used by shader translation to create a field meta
  /// object.
  ShaderIRFieldMeta* CreateField(LightningShaderIRLibrary* library);
  /// Find a field meta object by name.
  ShaderIRFieldMeta* FindField(StringParam fieldName);

  /// Internal function used by shader translation to create a function meta
  /// object.
  ShaderIRFunctionMeta* CreateFunction(LightningShaderIRLibrary* library);

  /// The name of the lightning type.
  String mLightningName;
  /// The lightning type this comes from. Useful for more complicated reflection.
  Lightning::BoundType* mLightningType;

  /// All of the owned fields that this type owns.
  Array<ShaderIRFieldMeta*> mFields;
  /// All of the owned functions that this type owns.
  Array<ShaderIRFunctionMeta*> mFunctions;

  /// The kind of shader fragment type that this represents.
  FragmentType::Enum mFragmentType;

  /// Any user data that cant simply be represented by a pointer
  /// Data can be written to the buffer and will be properly destructed
  /// when this object is destroyed (must be read in the order it's written)
  mutable Lightning::DestructibleBuffer mComplexUserData;
};

} // namespace Plasma
