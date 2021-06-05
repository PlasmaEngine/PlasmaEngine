// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// Ranges
LightningDefineRange(InstanceListType::range);
LightningDefineRange(NodeInfoListType::range);

// Enums
LightningDefineEnum(FalloffCurveType);
LightningDefineEnum(SoundPlayMode);
LightningDefineEnum(SoundSelectMode);
LightningDefineEnum(SynthWaveType);
LightningDefineEnum(AudioMixTypes);
LightningDefineEnum(AudioLatency);
LightningDefineEnum(GranularSynthWindows);

// Arrays
PlasmaDefineArrayType(Array<SoundEntry>);
PlasmaDefineArrayType(Array<SoundTagEntry>);

LightningDefineStaticLibrary(SoundLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  LightningInitializeRangeAs(InstanceListType::range, "SoundInstanceRange");
  LightningInitializeRangeAs(NodeInfoListType::range, "NodeInfoListRange");

  // Enums
  LightningInitializeEnum(FalloffCurveType);
  LightningInitializeEnum(SoundPlayMode);
  LightningInitializeEnum(SoundSelectMode);
  LightningInitializeEnum(SynthWaveType);
  LightningInitializeEnum(AudioMixTypes);
  LightningInitializeEnum(AudioLatency);
  LightningInitializeEnum(GranularSynthWindows);

  // Arrays
  PlasmaInitializeArrayTypeAs(Array<SoundEntry>, "Sounds");
  PlasmaInitializeArrayTypeAs(Array<SoundTagEntry>, "SoundTags");

  // Events
  LightningInitializeType(SoundInstanceEvent);
  LightningInitializeType(SoundEvent);
  LightningInitializeType(MidiEvent);
  LightningInitializeType(AudioFloatDataEvent);
  LightningInitializeType(CustomAudioNodeEvent);
  LightningInitializeType(AudioByteDataEvent);

  LightningInitializeTypeAs(SoundSystem, "Audio");
  LightningInitializeType(SoundNode);
  LightningInitializeType(SimpleCollapseNode);
  LightningInitializeType(SoundAsset);
  LightningInitializeType(SoundListener);
  LightningInitializeType(ListenerNode);
  LightningInitializeType(AudioSettings);
  LightningInitializeType(SoundSpace);
  LightningInitializeType(SoundAttenuatorDisplay);
  LightningInitializeType(SoundAttenuator);
  LightningInitializeType(AttenuatorNode);
  LightningInitializeType(SoundEmitterDisplay);
  LightningInitializeType(SoundEmitter);
  LightningInitializeType(EmitterNode);
  LightningInitializeType(SoundInstance);
  LightningInitializeType(SoundEntryDisplay);
  LightningInitializeType(SoundEntry);
  LightningInitializeType(SoundTagEntryDisplay);
  LightningInitializeType(SoundTagEntry);
  LightningInitializeType(SoundCueDisplay);
  LightningInitializeType(SoundCue);
  LightningInitializeType(SoundDisplay);
  LightningInitializeType(Sound);
  LightningInitializeType(SimpleSound);
  LightningInitializeType(SoundBuffer);
  LightningInitializeType(CustomAudioNode);
  LightningInitializeType(GeneratedWaveNode);
  LightningInitializeType(VolumeNode);
  LightningInitializeType(PitchNode);
  LightningInitializeType(LowPassNode);
  LightningInitializeType(HighPassNode);
  LightningInitializeType(BandPassNode);
  LightningInitializeType(EqualizerNode);
  LightningInitializeType(ReverbNode);
  LightningInitializeType(DelayNode);
  LightningInitializeType(FlangerNode);
  LightningInitializeType(ChorusNode);
  LightningInitializeType(RecordingNode);
  LightningInitializeType(CompressorNode);
  LightningInitializeType(ExpanderNode);
  LightningInitializeType(SoundTag);
  LightningInitializeType(PanningNode);
  LightningInitializeType(AddNoiseNode);
  LightningInitializeType(AdsrEnvelope);
  LightningInitializeType(AdditiveSynthNode);
  LightningInitializeType(GranularSynthNode);
  LightningInitializeType(ModulationNode);
  LightningInitializeType(MicrophoneInputNode);
  LightningInitializeType(SaveAudioNode);
  LightningInitializeType(SoundTagDisplay);
  LightningInitializeType(SoundTag);
  LightningInitializeType(TagObject);
  LightningInitializeType(NodePrintInfo);
  LightningInitializeType(SimpleCollapseNode);
  LightningInitializeType(OutputNode);
  LightningInitializeType(CombineNode);
  LightningInitializeType(CombineAndPauseNode);
  LightningInitializeType(SoundAsset);
  LightningInitializeType(DecompressedSoundAsset);
  LightningInitializeType(StreamingSoundAsset);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void SoundLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

void SoundLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Plasma
