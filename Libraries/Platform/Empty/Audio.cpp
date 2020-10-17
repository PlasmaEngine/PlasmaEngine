// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

AudioInputOutput::AudioInputOutput()
{
}

AudioInputOutput::~AudioInputOutput()
{
}

StreamStatus::Enum AudioInputOutput::InitializeAPI(Plasma::String* resultMessage)
{
  return StreamStatus::Uninitialized;
}

StreamStatus::Enum AudioInputOutput::InitializeStream(StreamTypes::Enum whichStream, Plasma::String* resultMessage)
{
  return StreamStatus::Uninitialized;
}

StreamStatus::Enum AudioInputOutput::StartStream(StreamTypes::Enum whichStream,
                                                 Plasma::String* resultMessage,
                                                 IOCallbackType* callback,
                                                 void* callbackData)
{
  return StreamStatus::Uninitialized;
}

StreamStatus::Enum AudioInputOutput::StopStream(StreamTypes::Enum whichStream, Plasma::String* resultMessage)
{
  return StreamStatus::Uninitialized;
}

void AudioInputOutput::ShutDownAPI()
{
}

unsigned AudioInputOutput::GetStreamChannels(StreamTypes::Enum whichStream)
{
  return 0;
}

unsigned AudioInputOutput::GetStreamSampleRate(StreamTypes::Enum whichStream)
{
  return 0;
}

float AudioInputOutput::GetBufferSizeMultiplier()
{
  return 1.0f;
}

MidiInput::MidiInput() : mOnMidiData(nullptr), mHandle(nullptr), mUserData(nullptr)
{
}

MidiInput::~MidiInput()
{
}

} // namespace Plasma
