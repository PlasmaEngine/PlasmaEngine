// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Plasma
{

// Sound Attenuator

LightningDefineType(SoundAttenuatorDisplay, builder, type)
{
}

String SoundAttenuatorDisplay::GetName(HandleParam object)
{
  SoundAttenuator* soundAtten = object.Get<SoundAttenuator*>(GetOptions::AssertOnNull);
  return BuildString("SoundAttenuator: ", soundAtten->Name);
}

String SoundAttenuatorDisplay::GetDebugText(HandleParam object)
{
  return GetName(object);
}

String SoundAttenuationToString(const BoundType* meta, const ::byte* data)
{
  SoundAttenuator* soundAtten = (SoundAttenuator*)data;
  return BuildString("SoundAttenuator: ", soundAtten->Name);
}

LightningDefineType(SoundAttenuator, builder, type)
{
  PlasmaBindDocumented();
  type->ToStringFunction = SoundAttenuationToString;

  LightningBindGetterSetterProperty(StartDistance);
  LightningBindGetterSetterProperty(StopDistance);
  LightningBindGetterSetterProperty(MinAttenuatedVolume)->Add(new EditorSlider(0.0f, 1.0f, 0.01f));
  LightningBindGetterSetterProperty(UseLowPassFilter)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  LightningBindGetterSetterProperty(LowPassStartDistance)->PlasmaFilterBool(mUseLowPassFilter);
  LightningBindGetterSetterProperty(LowPassCutoffFreq)->PlasmaFilterBool(mUseLowPassFilter);
  LightningBindGetterSetterProperty(FalloffCurveType)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  LightningBindGetterSetterProperty(FalloffCurve)
      ->PlasmaFilterEquality(mFalloffCurveType, FalloffCurveType::Enum, FalloffCurveType::Custom);
}

SoundAttenuator::SoundAttenuator() :
    mStartDistance(1.0f),
    mStopDistance(70.0f),
    mMinAttenuatedVolume(0.0f),
    mFalloffCurveType(FalloffCurveType::Log),
    mCustomFalloffCurve(nullptr),
    mUseLowPassFilter(true),
    mLowPassStartDistance(20.0f),
    mLowPassCutoffFreq(1000.0f)
{
  mResourceIconName = cAudioIcon;
}

SoundAttenuator::~SoundAttenuator()
{
  // Delete any existing SoundAttenuatorNode objects
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty();)
  {
    SoundAttenuatorNode* node = &nodes.Front();
    nodes.PopFront();
    delete node;
  }
}

void SoundAttenuator::Serialize(Serializer& stream)
{
  SerializeNameDefault(mStartDistance, 1.0f);
  SerializeNameDefault(mStopDistance, 70.0f);
  SerializeNameDefault(mMinAttenuatedVolume, 0.0f);
  SerializeNameDefault(mUseLowPassFilter, true);
  SerializeNameDefault(mLowPassStartDistance, 20.0f);
  SerializeNameDefault(mLowPassCutoffFreq, 1000.0f);
  SerializeNullableResourceNameDefault(mCustomFalloffCurve, CurveManager, nullptr);
  SerializeEnumNameDefault(FalloffCurveType, mFalloffCurveType, FalloffCurveType::Log);
}

void SoundAttenuator::Initialize()
{
  if (mFalloffCurveType == FalloffCurveType::Custom && mCustomFalloffCurve)
  {
    UpdateCurve(nullptr);
    SampleCurve* curve = mCustomFalloffCurve;
    if (curve)
      ConnectThisTo(curve, Events::ObjectModified, UpdateCurve);
  }
}

void SoundAttenuator::Unload()
{
  mCustomFalloffCurve = nullptr;
}

float SoundAttenuator::GetStartDistance()
{
  return mStartDistance;
}

void SoundAttenuator::SetStartDistance(float value)
{
  mStartDistance = Math::Clamp(value, 0.0f, mStopDistance);

  // Update the attenuation information on all existing nodes
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
    nodes.Front().mNode->SetStartDistance(mStartDistance);
}

float SoundAttenuator::GetStopDistance()
{
  return mStopDistance;
}

void SoundAttenuator::SetStopDistance(float value)
{
  mStopDistance = Math::Max(value, mStartDistance);

  // Update the attenuation information on all existing nodes
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
    nodes.Front().mNode->SetEndDistance(mStopDistance);
}

float SoundAttenuator::GetMinAttenuatedVolume()
{
  return mMinAttenuatedVolume;
}

void SoundAttenuator::SetMinAttenuatedVolume(float value)
{
  mMinAttenuatedVolume = Math::Clamp(value, 0.0f, AudioConstants::cMaxVolumeValue);

  // Update the attenuation information on all existing nodes
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
    nodes.Front().mNode->SetMinimumVolume(mMinAttenuatedVolume);
}

SampleCurve* SoundAttenuator::GetFalloffCurve()
{
  return mCustomFalloffCurve;
}

void SoundAttenuator::SetFalloffCurve(SampleCurve* newCurve)
{
  SampleCurve* curve = mCustomFalloffCurve;
  if (curve)
    curve->GetDispatcher()->DisconnectEvent(Events::ObjectModified, this);

  mCustomFalloffCurve = newCurve;

  UpdateCurve(nullptr);

  curve = mCustomFalloffCurve;

  if (curve)
  {
    ConnectThisTo(curve, Events::ObjectModified, UpdateCurve);

    Array<Vec3> curveData;
    curve->GetCurve(curveData);

    // Send the custom curve data to all existing nodes
    for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
      nodes.Front().mNode->SetCurveType(FalloffCurveType::Custom, &curveData);
  }
  else
  {
    // Set the custom curve data to null on all existing nodes
    for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
      nodes.Front().mNode->SetCurveType(FalloffCurveType::Custom, nullptr);
  }
}

FalloffCurveType::Enum SoundAttenuator::GetFalloffCurveType()
{
  return mFalloffCurveType;
}

void SoundAttenuator::SetFalloffCurveType(FalloffCurveType::Enum newtype)
{
  mFalloffCurveType = newtype;

  if (newtype != FalloffCurveType::Custom)
  {
    // Set the curve type on all existing nodes
    for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
      nodes.Front().mNode->SetCurveType(newtype, nullptr);
  }
}

bool SoundAttenuator::GetUseLowPassFilter()
{
  return mUseLowPassFilter;
}

void SoundAttenuator::SetUseLowPassFilter(bool useFilter)
{
  mUseLowPassFilter = useFilter;

  // Update all existing nodes
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
    nodes.Front().mNode->SetUsingLowPass(useFilter);
}

float SoundAttenuator::GetLowPassStartDistance()
{
  return mLowPassStartDistance;
}

void SoundAttenuator::SetLowPassStartDistance(float distance)
{
  mLowPassStartDistance = Math::Max(distance, 0.0f);

  // Update all existing nodes
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
    nodes.Front().mNode->SetLowPassDistance(distance);
}

float SoundAttenuator::GetLowPassCutoffFreq()
{
  return mLowPassCutoffFreq;
}

void SoundAttenuator::SetLowPassCutoffFreq(float frequency)
{
  mLowPassCutoffFreq = Math::Clamp(frequency, 0.0f, 20000.0f);

  // Update all existing nodes
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
    nodes.Front().mNode->SetLowPassCutoffFreq(frequency);
}

void SoundAttenuator::UpdateCurve(Event* event)
{
  // We don't ever use the fallback or default resources... just roll back to
  // 'none'
  SampleCurve* curve = mCustomFalloffCurve;
  if (curve)
  {
    CurveManager* manager = CurveManager::GetInstance();
    String& name = curve->Name;
    if (name == manager->DefaultResourceName || name == manager->FallbackResourceName)
      mCustomFalloffCurve = nullptr;
  }
}

SoundAttenuatorNode* SoundAttenuator::GetAttenuationNode(StringParam name, unsigned ID)
{
  SoundAttenuatorNode* node;
  // If using a custom curve, create the SoundAttenuatorNode with that curve
  if (mFalloffCurveType == FalloffCurveType::Custom && mCustomFalloffCurve)
  {
    Array<Vec3> curve;
    mCustomFalloffCurve->GetCurve(curve);

    node = new SoundAttenuatorNode(new AttenuatorNode(
        name, ID, Math::Vec3(0, 0, 0), mStartDistance, mStopDistance, mMinAttenuatedVolume, mFalloffCurveType, &curve));
  }
  // Otherwise create it for the specified curve type
  else
    node = new SoundAttenuatorNode(new AttenuatorNode(name,
                                                      ID,
                                                      Math::Vec3(0, 0, 0),
                                                      mStartDistance,
                                                      mStopDistance,
                                                      mMinAttenuatedVolume,
                                                      mFalloffCurveType,
                                                      nullptr));

  node->mNode->SetUsingLowPass(mUseLowPassFilter);
  if (mUseLowPassFilter)
  {
    node->mNode->SetLowPassDistance(mLowPassStartDistance);
    node->mNode->SetLowPassCutoffFreq(mLowPassCutoffFreq);
  }

  mNodeList.PushBack(node);

  return node;
}

void SoundAttenuator::RemoveAttenuationNode(SoundAttenuatorNode* node)
{
  mNodeList.Erase(node);
  delete node;
}

bool SoundAttenuator::HasInput()
{
  // If any of the SoundAttenuatorNodes has input, return true
  forRange (SoundAttenuatorNode& node, mNodeList.All())
  {
    if (node.mNode->GetHasInputs())
      return true;
  }

  return false;
}

// Sound Attenuator Manager

ImplementResourceManager(SoundAttenuatorManager, SoundAttenuator);

SoundAttenuatorManager::SoundAttenuatorManager(BoundType* resourceType) : ResourceManager(resourceType)
{
  AddLoader("SoundAttenuator", new TextDataFileLoader<SoundAttenuatorManager>());
  DefaultResourceName = "DefaultNoAttenuation";
  mCategory = "Sound";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.SoundAttenuator.data"));
  mCanCreateNew = true;
  mCanDuplicate = true;
  mExtension = DataResourceExtension;
}

} // namespace Plasma
