// MIT Licensed (see LICENSE.md).
#pragma once
#include "Core/Common/CommonStandard.hpp"
#include "PlatformStandard.hpp"
#include "Core/Serialization/SerializationStandard.hpp"
#include "Core/Meta/MetaStandard.hpp"
#include "Core/Support/SupportStandard.hpp"
#include "Core/Engine/EngineStandard.hpp"
#include "Core/Geometry/DebugDraw.hpp"

namespace Plasma
{
// Forward declarations
class SoundSystem;
class SoundSpace;
class SoundNode;
class SoundInstance;
class SoundCue;
class SoundEntry;
class SoundTag;
class SoundTagEntry;
class Sound;

// Sound library
class PlasmaNoImportExport SoundLibrary : public Lightning::StaticLibrary
{
public:
  LightningDeclareStaticLibraryInternals(SoundLibrary, "PlasmaEngine");

  static void Initialize();
  static void Shutdown();
};

} // namespace Plasma

#include "Definitions.hpp"
#include "RingBuffer.hpp"
#include "LockFreeQueue.hpp"
#include "Interpolator.hpp"
#include "AudioIOInterface.hpp"
#include "Filters.hpp"
#include "Resampler.hpp"
#include "VBAP.hpp"
#include "PitchChange.hpp"
#include "FileDecoder.hpp"
#include "VolumeModifier.hpp"
#include "SoundAsset.hpp"
#include "SoundNode.hpp"
#include "SoundTag.hpp"
#include "AudioMixer.hpp"
#include "AttenuatorNode.hpp"
#include "EmitterNode.hpp"
#include "ListenerNode.hpp"
#include "CustomAudioNode.hpp"
#include "GeneratedAudio.hpp"
#include "Recording.hpp"
#include "SoundListener.hpp"
#include "DspFilterNodes.hpp"
#include "SoundAttenuator.hpp"
#include "SoundEmitter.hpp"
#include "SoundSpace.hpp"
#include "SoundNodeGraph.hpp"
#include "SoundInstance.hpp"
#include "SoundSystem.hpp"
#include "Sound.hpp"
#include "SoundCue.hpp"
#include "SimpleSound.hpp"
#include "SimpleSound.hpp"
