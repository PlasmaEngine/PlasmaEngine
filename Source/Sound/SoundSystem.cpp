// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Plasma
{

namespace Tags
{

DefineTag(Sound);

} // namespace Tags

namespace Events
{

DefineEvent(MIDINoteOn);
DefineEvent(MIDINoteOff);
DefineEvent(MIDIPitchWheel);
DefineEvent(MIDIVolume);
DefineEvent(MIDIModWheel);
DefineEvent(MIDIOtherControl);
DefineEvent(SoundInstancePlayed);
DefineEvent(MicrophoneUncompressedFloatData);
DefineEvent(MicrophoneCompressedByteData);

} // namespace Events

namespace PL
{

SoundSystem* gSound;

} // namespace PL

System* CreateSoundSystem()
{
  return new SoundSystem();
}

// Sound Event

LightningDefineType(SoundEvent, builder, type)
{
  PlasmaBindDocumented();
}

// MIDI Event

LightningDefineType(MidiEvent, builder, type)
{
  PlasmaBindDocumented();

  LightningBindField(Channel);
  LightningBindField(MIDINumber);
  LightningBindField(Value);
}

// Audio Float Data Event

LightningDefineType(AudioFloatDataEvent, builder, type)
{
  PlasmaBindDocumented();

  LightningBindField(Channels);
  LightningBindMember(AudioData);
}

// Audio Byte Data Event

LightningDefineType(AudioByteDataEvent, builder, type)
{
  PlasmaBindDocumented();

  LightningBindMember(AudioData);
}

// Sound System

LightningDefineType(SoundSystem, builder, type)
{
  type->HandleManager = LightningManagerId(PointerManager);

  LightningBindGetterSetter(SystemVolume);
  LightningBindGetter(PeakOutputLevel);
  LightningBindGetter(RMSOutputLevel);
  LightningBindGetter(PeakInputLevel);
  LightningBindMethod(GetNodeGraphInfo);
  LightningBindGetterSetter(LatencySetting);
  LightningBindGetterSetter(DispatchMicrophoneUncompressedFloatData);
  LightningBindGetterSetter(DispatchMicrophoneCompressedByteData);
  LightningBindGetter(OutputChannels);
  LightningBindGetterSetter(MuteAllAudio);

  LightningBindMethod(VolumeNode);
  LightningBindMethod(PanningNode);
  LightningBindMethod(PitchNode);
  LightningBindMethod(LowPassNode);
  LightningBindMethod(HighPassNode);
  LightningBindMethod(BandPassNode);
  LightningBindMethod(EqualizerNode);
  LightningBindMethod(ReverbNode);
  LightningBindMethod(DelayNode);
  LightningBindMethod(CustomAudioNode);
  LightningBindMethod(SoundBuffer);
  LightningBindMethod(FlangerNode);
  LightningBindMethod(ChorusNode);
  LightningBindMethod(CompressorNode);
  LightningBindMethod(ExpanderNode);
  LightningBindMethod(GeneratedWaveNode);
  LightningBindMethod(RecordingNode);
  LightningBindMethod(AddNoiseNode);
  LightningBindMethod(AdditiveSynthNode);
  LightningBindMethod(GranularSynthNode);
  LightningBindMethod(ModulationNode);
  LightningBindMethod(MicrophoneInputNode);
  LightningBindMethod(SaveAudioNode);

  PlasmaBindEvent(Events::MIDINoteOn, MidiEvent);
  PlasmaBindEvent(Events::MIDINoteOff, MidiEvent);
  PlasmaBindEvent(Events::MIDIPitchWheel, MidiEvent);
  PlasmaBindEvent(Events::MIDIVolume, MidiEvent);
  PlasmaBindEvent(Events::MIDIModWheel, MidiEvent);
  PlasmaBindEvent(Events::MIDIOtherControl, MidiEvent);
  PlasmaBindEvent(Events::SoundInstancePlayed, SoundInstanceEvent);
  PlasmaBindEvent(Events::MicrophoneUncompressedFloatData, AudioFloatDataEvent);
  PlasmaBindEvent(Events::MicrophoneCompressedByteData, AudioByteDataEvent);
}

SoundSystem::SoundSystem() :
    mCounter(0),
    mPreviewInstance(0),
    mLatency(AudioLatency::Low),
    mSendMicEvents(false),
    mSendCompressedMicEvents(false),
    mSoundSpaceCounter(0),
    mUseRandomSeed(true),
    mSeed(0)
{
}

SoundSystem::~SoundSystem()
{
  // If currently previewing a sound, stop
  SoundInstance* previewInstance = mPreviewInstance;
  if (previewInstance)
  {
    previewInstance->Stop();
  }

  Mixer.ShutDown();
}

void SoundSystem::Initialize(SystemInitializer& initializer)
{
  PL::gSound = this;

  // Create a System object and initialize.
  Plasma::Status status;
  Mixer.StartMixing(status);
  if (status.Failed())
    DoNotifyWarning("Audio Initialization Unsuccessful", status.Message);

  mOutputNode = new CombineNode("AudioOutput", mCounter++);
  Mixer.FinalOutputNode->AddInputNode(mOutputNode);

  InitializeResourceManager(SoundManager);
  InitializeResourceManager(SoundCueManager);
  InitializeResourceManager(SoundTagManager);
  InitializeResourceManager(SoundAttenuatorManager);

  if (!mUseRandomSeed)
    mRandom.SetSeed(mSeed);
}

NodeInfoListType::range SoundSystem::GetNodeGraphInfo()
{
  return NodeGraph.GetNodeInfoList();
}

float SoundSystem::GetSystemVolume()
{
  return Mixer.GetVolume();
}

void SoundSystem::SetSystemVolume(float volume)
{
  Mixer.SetVolume(Math::Max(volume, 0.0f));
}

bool SoundSystem::GetMuteAllAudio()
{
  return Mixer.GetMuteAllAudio();
}

void SoundSystem::SetMuteAllAudio(bool muteAudio)
{
  Mixer.SetMuteAllAudio(muteAudio);
}

float SoundSystem::GetPeakOutputLevel()
{
  return Mixer.GetPeakOutputVolume();
}

float SoundSystem::GetRMSOutputLevel()
{
  return Mixer.GetRMSOutputVolume();
}

float SoundSystem::GetPeakInputLevel()
{
  return Mixer.GetPeakInputVolume();
}

AudioLatency::Enum SoundSystem::GetLatencySetting()
{
  return mLatency;
}

void SoundSystem::SetLatencySetting(AudioLatency::Enum latency)
{
  mLatency = latency;

  Mixer.SetLatency(latency);
}

bool SoundSystem::GetDispatchMicrophoneUncompressedFloatData()
{
  return mSendMicEvents;
}

void SoundSystem::SetDispatchMicrophoneUncompressedFloatData(bool dispatchData)
{
  mSendMicEvents = dispatchData;
  Mixer.SetSendUncompressedMicInput(dispatchData);
}

bool SoundSystem::GetDispatchMicrophoneCompressedByteData()
{
  return mSendCompressedMicEvents;
}

void SoundSystem::SetDispatchMicrophoneCompressedByteData(bool dispatchData)
{
  mSendCompressedMicEvents = dispatchData;
  Mixer.SetSendCompressedMicInput(dispatchData);
}

int SoundSystem::GetOutputChannels()
{
  return Mixer.GetOutputChannels();
}

VolumeNode* SoundSystem::VolumeNode()
{
  Plasma::VolumeNode* node = new Plasma::VolumeNode("VolumeNode", PL::gSound->mCounter++);
  return node;
}

PanningNode* SoundSystem::PanningNode()
{
  Plasma::PanningNode* node = new Plasma::PanningNode("PanningNode", PL::gSound->mCounter++);
  return node;
}

PitchNode* SoundSystem::PitchNode()
{
  Plasma::PitchNode* node = new Plasma::PitchNode("PitchNode", PL::gSound->mCounter++);
  return node;
}

LowPassNode* SoundSystem::LowPassNode()
{
  Plasma::LowPassNode* node = new Plasma::LowPassNode("LowPassNode", PL::gSound->mCounter++);
  return node;
}

HighPassNode* SoundSystem::HighPassNode()
{
  Plasma::HighPassNode* node = new Plasma::HighPassNode("HighPassNode", PL::gSound->mCounter++);
  return node;
}

BandPassNode* SoundSystem::BandPassNode()
{
  Plasma::BandPassNode* node = new Plasma::BandPassNode("BandPassNode", PL::gSound->mCounter++);
  return node;
}

EqualizerNode* SoundSystem::EqualizerNode()
{
  Plasma::EqualizerNode* node = new Plasma::EqualizerNode("EqualizerNode", PL::gSound->mCounter++);
  return node;
}

ReverbNode* SoundSystem::ReverbNode()
{
  Plasma::ReverbNode* node = new Plasma::ReverbNode("ReverbNode", PL::gSound->mCounter++);
  return node;
}

DelayNode* SoundSystem::DelayNode()
{
  Plasma::DelayNode* node = new Plasma::DelayNode("DelayNode", PL::gSound->mCounter++);
  return node;
}

FlangerNode* SoundSystem::FlangerNode()
{
  Plasma::FlangerNode* node = new Plasma::FlangerNode("FlangerNode", PL::gSound->mCounter++);
  return node;
}

ChorusNode* SoundSystem::ChorusNode()
{
  Plasma::ChorusNode* node = new Plasma::ChorusNode("ChorusNode", PL::gSound->mCounter++);
  return node;
}

CompressorNode* SoundSystem::CompressorNode()
{
  Plasma::CompressorNode* node = new Plasma::CompressorNode("CompressorNode", PL::gSound->mCounter++);
  return node;
}

ExpanderNode* SoundSystem::ExpanderNode()
{
  Plasma::ExpanderNode* node = new Plasma::ExpanderNode("ExpanderNode", PL::gSound->mCounter++);
  return node;
}

CustomAudioNode* SoundSystem::CustomAudioNode()
{
  Plasma::CustomAudioNode* node = new Plasma::CustomAudioNode("CustomAudioNode", PL::gSound->mCounter++);
  return node;
}

SoundBuffer* SoundSystem::SoundBuffer()
{
  Plasma::SoundBuffer* buffer = new Plasma::SoundBuffer();
  return buffer;
}

GeneratedWaveNode* SoundSystem::GeneratedWaveNode()
{
  Plasma::GeneratedWaveNode* node = new Plasma::GeneratedWaveNode("GeneratedWaveNode", PL::gSound->mCounter++);
  return node;
}

RecordingNode* SoundSystem::RecordingNode()
{
  Plasma::RecordingNode* node = new Plasma::RecordingNode("RecordingNode", PL::gSound->mCounter++);
  return node;
}

AddNoiseNode* SoundSystem::AddNoiseNode()
{
  Plasma::AddNoiseNode* node = new Plasma::AddNoiseNode("AddNoiseNode", PL::gSound->mCounter++);
  return node;
}

AdditiveSynthNode* SoundSystem::AdditiveSynthNode()
{
  Plasma::AdditiveSynthNode* node = new Plasma::AdditiveSynthNode("AdditiveSynthNode", PL::gSound->mCounter++);
  return node;
}

ModulationNode* SoundSystem::ModulationNode()
{
  Plasma::ModulationNode* node = new Plasma::ModulationNode("ModulationNode", PL::gSound->mCounter++);
  return node;
}

MicrophoneInputNode* SoundSystem::MicrophoneInputNode()
{
  Plasma::MicrophoneInputNode* node = new Plasma::MicrophoneInputNode("MicrophoneInputNode", PL::gSound->mCounter++);
  return node;
}

SaveAudioNode* SoundSystem::SaveAudioNode()
{
  Plasma::SaveAudioNode* node = new Plasma::SaveAudioNode("SaveAudioNode", PL::gSound->mCounter++);
  return node;
}

GranularSynthNode* SoundSystem::GranularSynthNode()
{
  Plasma::GranularSynthNode* node = new Plasma::GranularSynthNode("GranularSynthNode", PL::gSound->mCounter++);
  return node;
}

void SoundSystem::Update(bool debugger)
{
  if (debugger)
    return;

  // Update spaces (also updates emitters)
  forRange (SoundSpace& space, mSpaces.All())
    space.Update();

  // Update audio system
  Mixer.Update();
}

void SoundSystem::StopPreview()
{
  SoundInstance* sound = mPreviewInstance;
  if (sound)
  {
    sound->Stop();
    mPreviewInstance = nullptr;
  }
}

void SoundSystem::AddSoundSpace(SoundSpace* space, bool isEditor)
{
  mSpaces.PushBack(space);

  // If not an editor space, increase the counter and notify tags if necessary
  if (!isEditor)
  {
    ++mSoundSpaceCounter;

    if (mSoundSpaceCounter == 1)
    {
      forRange (SoundTag& tag, mSoundTags.All())
        tag.CreateTag();
    }
  }
}

void SoundSystem::RemoveSoundSpace(SoundSpace* space, bool isEditor)
{
  mSpaces.Erase(space);

  // If not an editor space, decrease the counter and notify tags if necessary
  if (!isEditor)
  {
    --mSoundSpaceCounter;

    ErrorIf(mSoundSpaceCounter < 0, "SoundSystem's space tracking has become negative");

    if (mSoundSpaceCounter == 0)
    {
      forRange (SoundTag& tag, mSoundTags.All())
        tag.ReleaseTag();
    }
  }
}

// Audio Settings

LightningDefineType(AudioSettings, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindTag(Tags::Sound);
  PlasmaBindDocumented();

  LightningBindGetterSetterProperty(SystemVolume)->Add(new EditorSlider(0.0f, 2.0f, 0.01f));
  LightningBindGetterSetterProperty(MuteAllAudio);
  LightningBindGetterSetterProperty(UseRandomSeed)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  LightningBindGetterSetterProperty(Seed)->PlasmaFilterEquality(mUseRandomSeed, bool, false);
  LightningBindGetterSetterProperty(MixType);
  LightningBindGetterSetterProperty(MinVolumeThreshold)->Add(new EditorSlider(0.0f, 0.2f, 0.001f));
  LightningBindGetterSetterProperty(LatencySetting);
}

AudioSettings::AudioSettings() :
    mSystemVolume(1.0f),
    mMinVolumeThreshold(0.015f),
    mMixType(AudioMixTypes::AutoDetect),
    mLatency(AudioLatency::Low),
    mUseRandomSeed(true),
    mSeed(0)
{
}

void AudioSettings::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSystemVolume, 1.0f);
  SerializeEnumNameDefault(AudioMixTypes, mMixType, AudioMixTypes::AutoDetect);
  SerializeNameDefault(mMinVolumeThreshold, 0.015f);
  SerializeEnumNameDefault(AudioLatency, mLatency, AudioLatency::Low);
  SerializeNameDefault(mUseRandomSeed, true);
  SerializeNameDefault(mSeed, 0u);
}

void AudioSettings::Initialize(CogInitializer& initializer)
{
  PL::gSound->Mixer.SetVolume(mSystemVolume);
  SetMixType(mMixType);
  PL::gSound->Mixer.SetMinimumVolumeThreshold(mMinVolumeThreshold);
  PL::gSound->SetLatencySetting(mLatency);
  PL::gSound->mUseRandomSeed = mUseRandomSeed;
  PL::gSound->mSeed = mSeed;
  if (mUseRandomSeed)
    PL::gSound->mRandom.SetSeed(mSeed);
}

float AudioSettings::GetSystemVolume()
{
  mSystemVolume = PL::gSound->Mixer.GetVolume();
  return mSystemVolume;
}

void AudioSettings::SetSystemVolume(float volume)
{
  mSystemVolume = Math::Clamp(volume, 0.0f, AudioConstants::cMaxVolumeValue);

  PL::gSound->Mixer.SetVolume(mSystemVolume);
}

bool AudioSettings::GetMuteAllAudio()
{
  return PL::gSound->GetMuteAllAudio();
}

void AudioSettings::SetMuteAllAudio(bool muteAudio)
{
  PL::gSound->SetMuteAllAudio(muteAudio);
}

AudioMixTypes::Enum AudioSettings::GetMixType()
{
  return mMixType;
}

void AudioSettings::SetMixType(AudioMixTypes::Enum mixType)
{
  mMixType = mixType;

  switch (mixType)
  {
  case AudioMixTypes::AutoDetect:
    PL::gSound->Mixer.SetOutputChannels(0);
    break;
  case AudioMixTypes::Mono:
    PL::gSound->Mixer.SetOutputChannels(1);
    break;
  case AudioMixTypes::Stereo:
    PL::gSound->Mixer.SetOutputChannels(2);
    break;
  case AudioMixTypes::Quad:
    PL::gSound->Mixer.SetOutputChannels(4);
    break;
  case AudioMixTypes::FiveOne:
    PL::gSound->Mixer.SetOutputChannels(6);
    break;
  case AudioMixTypes::SevenOne:
    PL::gSound->Mixer.SetOutputChannels(8);
    break;
  default:
    PL::gSound->Mixer.SetOutputChannels(2);
    break;
  }
}

float AudioSettings::GetMinVolumeThreshold()
{
  return mMinVolumeThreshold;
}

void AudioSettings::SetMinVolumeThreshold(float volume)
{
  mMinVolumeThreshold = Math::Clamp(volume, 0.0f, 0.5f);
  PL::gSound->Mixer.SetMinimumVolumeThreshold(mMinVolumeThreshold);
}

Plasma::AudioLatency::Enum AudioSettings::GetLatencySetting()
{
  return mLatency;
}

void AudioSettings::SetLatencySetting(AudioLatency::Enum latency)
{
  mLatency = latency;
  PL::gSound->SetLatencySetting(latency);
}

bool AudioSettings::GetUseRandomSeed()
{
  return mUseRandomSeed;
}

void AudioSettings::SetUseRandomSeed(bool useRandom)
{
  if (useRandom && !mUseRandomSeed)
  {
    PL::gSound->mRandom.SetSeed(PL::gSound->mRandom.mGlobalSeed);
  }
  else if (!useRandom && mUseRandomSeed)
  {
    PL::gSound->mRandom.SetSeed(mSeed);
  }

  mUseRandomSeed = useRandom;
  PL::gSound->mUseRandomSeed = useRandom;
}

uint AudioSettings::GetSeed()
{
  return mSeed;
}

void AudioSettings::SetSeed(uint seed)
{
  if (!mUseRandomSeed)
    PL::gSound->mRandom.SetSeed(seed);

  mSeed = seed;
  PL::gSound->mSeed = seed;
}

} // namespace Plasma
