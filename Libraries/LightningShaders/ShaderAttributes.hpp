// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class ShaderIRAttributeParameter
{
public:
  ShaderIRAttributeParameter();
  ShaderIRAttributeParameter(Lightning::AttributeParameter& param, Lightning::SyntaxNode* node);

  String GetName() const;
  void SetName(StringParam name);

  Lightning::ConstantType::Enum GetType() const;

  String GetStringValue() const;
  void SetStringValue(StringParam stringValue);

  int GetIntValue() const;
  void SetIntValue(int intValue);

  float GetFloatValue() const;
  void SetFloatValue(float floatValue);

  Lightning::Type* GetTypeValue() const;
  void SetTypeValue(Lightning::Type* typeValue);

  Lightning::CodeLocation* GetLocation();
  void SetLocationNode(Lightning::SyntaxNode* node);

  // Return the internal lightning attribute parameter. Mostly exposed for ease of
  // binding.
  Lightning::AttributeParameter& GetLightningAttributeParameter();

private:
  Lightning::AttributeParameter mParameter;
  Lightning::SyntaxNode* mNode;
};

class ShaderIRAttribute
{
public:
  ShaderIRAttribute();
  ShaderIRAttribute(StringParam attributeName, Lightning::SyntaxNode* locationNode);

  ShaderIRAttributeParameter* FindFirstParameter(StringParam name);
  Lightning::CodeLocation* GetLocation();

  String mAttributeName;
  Array<ShaderIRAttributeParameter> mParameters;
  Lightning::SyntaxNode* mNode;

  // Was this attribute created from another (e.g. [Input] implies
  // [AppBuiltInInput]). Some errors are only valid if the attribute was
  // explicitly declared.
  bool mImplicitAttribute;
};

class ShaderIRAttributeList
{
public:
  typedef Array<ShaderIRAttribute>::range Range;

  class NamedRange
  {
  public:
    NamedRange();
    NamedRange(StringParam attributeToFind, const Range& range);

    ShaderIRAttribute* Front();
    bool Empty() const;
    void PopFront();

  private:
    void SkipAttributes();

    String mAttributeToFind;
    Range mRange;
  };

  ShaderIRAttribute* AddAttribute(StringParam attributeName, Lightning::AttributeNode* node);
  NamedRange FindAttributes(StringParam attributeName);
  ShaderIRAttribute* FindFirstAttribute(StringParam attributeName);
  Range All();
  size_t Size();

  ShaderIRAttribute* GetAtIndex(size_t index);
  ShaderIRAttribute* operator[](size_t index);

private:
  Array<ShaderIRAttribute> mAttributes;
};

} // namespace Plasma
