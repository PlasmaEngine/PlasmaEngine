// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

ShaderIRAttributeParameter::ShaderIRAttributeParameter()
{
  mNode = nullptr;
}

ShaderIRAttributeParameter::ShaderIRAttributeParameter(Lightning::AttributeParameter& param, Lightning::SyntaxNode* node)
{
  mParameter = param;
  mNode = node;
}

String ShaderIRAttributeParameter::GetName() const
{
  return mParameter.Name;
}

void ShaderIRAttributeParameter::SetName(StringParam name)
{
  mParameter.Name = name;
}

Lightning::ConstantType::Enum ShaderIRAttributeParameter::GetType() const
{
  return mParameter.Type;
}

String ShaderIRAttributeParameter::GetStringValue() const
{
  return mParameter.StringValue;
}

void ShaderIRAttributeParameter::SetStringValue(StringParam stringValue)
{
  mParameter.StringValue = stringValue;
  mParameter.Type = Lightning::ConstantType::String;
}

int ShaderIRAttributeParameter::GetIntValue() const
{
  return (int)mParameter.IntegerValue;
}

void ShaderIRAttributeParameter::SetIntValue(int intValue)
{
  mParameter.IntegerValue = intValue;
  mParameter.Type = Lightning::ConstantType::Integer;
}

float ShaderIRAttributeParameter::GetFloatValue() const
{
  return (float)mParameter.RealValue;
}

void ShaderIRAttributeParameter::SetFloatValue(float floatValue)
{
  mParameter.RealValue = floatValue;
  mParameter.Type = Lightning::ConstantType::Real;
}

Lightning::Type* ShaderIRAttributeParameter::GetTypeValue() const
{
  return mParameter.TypeValue;
}

void ShaderIRAttributeParameter::SetTypeValue(Lightning::Type* typeValue)
{
  mParameter.TypeValue = typeValue;
  mParameter.Type = Lightning::ConstantType::Type;
}

Lightning::CodeLocation* ShaderIRAttributeParameter::GetLocation()
{
  if (mNode == nullptr)
    return nullptr;
  return &mNode->Location;
}

void ShaderIRAttributeParameter::SetLocationNode(Lightning::SyntaxNode* node)
{
  mNode = node;
}

Lightning::AttributeParameter& ShaderIRAttributeParameter::GetLightningAttributeParameter()
{
  return mParameter;
}

ShaderIRAttribute::ShaderIRAttribute()
{
  mNode = nullptr;
  mImplicitAttribute = false;
}

ShaderIRAttribute::ShaderIRAttribute(StringParam attributeName, Lightning::SyntaxNode* locationNode)
{
  mNode = locationNode;
  mAttributeName = attributeName;
  mImplicitAttribute = false;
}

ShaderIRAttributeParameter* ShaderIRAttribute::FindFirstParameter(StringParam name)
{
  for (size_t i = 0; i < mParameters.Size(); ++i)
  {
    if (mParameters[i].GetName() == name)
      return &mParameters[i];
  }
  return nullptr;
}

Lightning::CodeLocation* ShaderIRAttribute::GetLocation()
{
  if (mNode == nullptr)
    return nullptr;
  return &mNode->Location;
}

ShaderIRAttributeList::NamedRange::NamedRange()
{
}

ShaderIRAttributeList::NamedRange::NamedRange(StringParam attributeToFind, const Range& range)
{
  mRange = range;
  mAttributeToFind = attributeToFind;
  SkipAttributes();
}

ShaderIRAttribute* ShaderIRAttributeList::NamedRange::Front()
{
  return &mRange.Front();
}

bool ShaderIRAttributeList::NamedRange::Empty() const
{
  return mRange.Empty();
}

void ShaderIRAttributeList::NamedRange::PopFront()
{
  mRange.PopFront();
  SkipAttributes();
}

void ShaderIRAttributeList::NamedRange::SkipAttributes()
{
  while (!mRange.Empty())
  {
    if (mRange.Front().mAttributeName == mAttributeToFind)
      break;
    mRange.PopFront();
  }
}

ShaderIRAttribute* ShaderIRAttributeList::AddAttribute(StringParam attributeName, Lightning::AttributeNode* node)
{
  ShaderIRAttribute* attribute = &mAttributes.PushBack();
  attribute->mAttributeName = attributeName;
  attribute->mNode = node;
  return attribute;
}

ShaderIRAttributeList::NamedRange ShaderIRAttributeList::FindAttributes(StringParam attributeName)
{
  return NamedRange(attributeName, mAttributes.All());
}

ShaderIRAttribute* ShaderIRAttributeList::FindFirstAttribute(StringParam attributeName)
{
  NamedRange range = FindAttributes(attributeName);
  if (!range.Empty())
    return range.Front();
  return nullptr;
}

ShaderIRAttributeList::Range ShaderIRAttributeList::All()
{
  return mAttributes.All();
}

size_t ShaderIRAttributeList::Size()
{
  return mAttributes.Size();
}

ShaderIRAttribute* ShaderIRAttributeList::GetAtIndex(size_t index)
{
  return &mAttributes[index];
}

ShaderIRAttribute* ShaderIRAttributeList::operator[](size_t index)
{
  return GetAtIndex(index);
}

} // namespace Plasma
