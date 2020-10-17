// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// Editor Property Extension
LightningDefineType(EditorPropertyExtension, builder, type)
{
}

// Indexed String Array
LightningDefineType(EditorIndexedStringArray, builder, type)
{
}

EditorIndexedStringArray::EditorIndexedStringArray(GetStringsFunc sourceArray) : AccessSourceArray(sourceArray){};

void EditorIndexedStringArray::Enumerate(HandleParam instance, Property* property, Array<String>& strings)
{
  if (AccessSourceArray)
    AccessSourceArray(instance, property, strings);
  else
    strings = mFixedValues;
}

// Editor Range
LightningDefineType(EditorRange, builder, type)
{
  LightningBindFieldProperty(Min);
  LightningBindFieldProperty(Max);
  LightningBindFieldProperty(Increment)->AddAttribute(PropertyAttributes::cOptional);
}

EditorRange::EditorRange() : Min(-Math::PositiveMax()), Max(Math::PositiveMax()), Increment(0.01f)
{
}

EditorRange::EditorRange(float minValue, float maxValue, float increment) :
    Min(minValue),
    Max(maxValue),
    Increment(increment)
{
}

void EditorRange::PostProcess(Status& status, ReflectionObject* owner)
{
  if (Increment == 0)
  {
    status.SetFailed("'Increment' cannot be 0.");
    return;
  }
  if (Min >= Max)
  {
    status.SetFailed("'Max' must be greater than or equal to 'Min'.");
    return;
  }
}

// Editor Slider
LightningDefineType(EditorSlider, builder, type)
{
}

EditorSlider::EditorSlider()
{
  Min = 0;
  Max = 1;
  Increment = 0.01f;
}

// EditorRotationBasis
LightningDefineType(EditorRotationBasis, builder, type)
{
}

EditorRotationBasis::EditorRotationBasis() : mIntData(0), mGizmoName("EditorGizmo")
{
}

EditorRotationBasis::EditorRotationBasis(StringParam archetypeName) :
    mIntData(0),
    mGizmoName("EditorGizmo"),
    mArchetypeName(archetypeName)
{
}

EditorRotationBasis::EditorRotationBasis(StringParam archetypeName, StringParam gizmoName, int intData) :
    mIntData(intData),
    mGizmoName(gizmoName),
    mArchetypeName(archetypeName)
{
}

// Editor Resource
LightningDefineType(MetaEditorResource, builder, type)
{
  LightningBindField(FilterTag)->AddAttribute(PropertyAttributes::cOptional);
  LightningBindField(AllowAdd)->AddAttribute(PropertyAttributes::cOptional);
  LightningBindField(AllowNone)->AddAttribute(PropertyAttributes::cOptional);
  LightningBindField(SearchPreview)->AddAttribute(PropertyAttributes::cOptional);
}

MetaEditorResource::MetaEditorResource(
    bool allowAdd, bool allowNone, StringParam filterTag, bool forceCompact, bool searchPreview) :
    FilterTag(filterTag),
    AllowAdd(allowAdd),
    AllowNone(allowNone),
    ForceCompact(forceCompact),
    SearchPreview(searchPreview),
    Filter(nullptr)
{
}

MetaEditorResource::MetaEditorResource(SearchFilter filter) :
    AllowAdd(false),
    AllowNone(false),
    FilterTag(""),
    ForceCompact(false),
    SearchPreview(true),
    Filter(filter)
{
}

bool MetaEditorResource::FilterPropertySearchResult(HandleParam object,
                                                    Property* property,
                                                    HandleParam result,
                                                    Status& status)
{
  if (Filter)
    return Filter(object, property, result, status);
  return true;
}

// Meta Property Filter
LightningDefineType(MetaPropertyFilter, builder, type)
{
}

// Property Basic Filter
LightningDefineType(MetaPropertyBasicFilter, builder, type)
{
}

// Meta Editor Gizmo
LightningDefineType(MetaEditorGizmo, builder, type)
{
  LightningBindFieldProperty(mGizmoArchetype);
}

// Meta Shader Input
LightningDefineType(MetaShaderInput, builder, type)
{
  LightningBindFieldProperty(mFragmentName)->AddAttribute(PropertyAttributes::cOptional);
  LightningBindFieldProperty(mInputName)->AddAttribute(PropertyAttributes::cOptional);
}

// Meta Group
LightningDefineType(MetaGroup, builder, type)
{
  LightningBindFieldProperty(mName);
}

MetaGroup::MetaGroup(StringParam name) : mName(name)
{
}

// Meta Custom Ui
LightningDefineType(MetaCustomUi, builder, type)
{
}

// Property Rename
LightningDefineType(MetaPropertyRename, builder, type)
{
}

MetaPropertyRename::MetaPropertyRename(StringParam oldName) : mOldName(oldName)
{
}

} // namespace Plasma
