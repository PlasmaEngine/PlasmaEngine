// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
namespace PL
{
Joysticks* gJoysticks = nullptr;
}

const String cUnknownString("Unknown");

namespace Events
{
DefineEvent(JoystickUpdated);
DefineEvent(JoystickButtonDown);
DefineEvent(JoystickButtonUp);

DefineEvent(JoystickFound);
DefineEvent(JoystickLost);
DefineEvent(JoysticksChanged);
} // namespace Events

LightningDefineType(RawControlMapping, builder, type)
{
}

RawControlMapping::RawControlMapping()
{
}

RawControlMapping::RawControlMapping(const PlatformInputDevice& device)
{
  mName = device.mName;
  forRange (const PlatformAxis& platformAxis, device.mAxes)
  {
    RawAxis& rawAxis = mAxes.PushBack();
    rawAxis.Name = platformAxis.mName;
    rawAxis.Offset = platformAxis.mOffset;
    rawAxis.Size = platformAxis.mSize;
    rawAxis.Min = platformAxis.mMin;
    rawAxis.Max = platformAxis.mMax;

    rawAxis.Mid = (rawAxis.Max + rawAxis.Min) / 2;
    rawAxis.DeadZonePercent = 0.16f;
    rawAxis.CanCalibrate = true;
    rawAxis.UseMid = true;
    rawAxis.Reversed = false;
    rawAxis.CanBeDisabled = platformAxis.mCanBeDisabled;

    // If this is a hat, we need to make sure it doesn't get calibrated
    if (platformAxis.mUsbUsage == UsbUsage::HatSwitch)
    {
      rawAxis.CanCalibrate = false;
      rawAxis.UseMid = false;
      rawAxis.DeadZonePercent = 0.0f;
    }
  }

  forRange (const PlatformButton& platformButton, device.mButtons)
  {
    RawButton& rawButton = mButtons.PushBack();
    rawButton.Name = platformButton.mName;
    rawButton.Offset = platformButton.mOffset;
    rawButton.Bit = platformButton.mBit;
  }
}

void RawAxis::Serialize(Serializer& stream)
{
  SerializeNameDefault(Name, cUnknownString);
  SerializeNameDefault(Offset, (uint)0);
  SerializeNameDefault(Size, (uint)8);
  SerializeNameDefault(Min, (uint)0);
  SerializeNameDefault(Mid, (uint)128);
  SerializeNameDefault(Max, (uint)255);
  SerializeNameDefault(DeadZonePercent, 0.0f);
  SerializeNameDefault(CanCalibrate, true);
  SerializeNameDefault(UseMid, true);
  SerializeNameDefault(Reversed, false);
  SerializeNameDefault(CanBeDisabled, false);
}

void RawButton::Serialize(Serializer& stream)
{
  SerializeNameDefault(Name, cUnknownString);
  SerializeNameDefault(Offset, (uint)0);
  SerializeNameDefault(Bit, (uint)0);
}

void RawControlMapping::Serialize(Serializer& stream)
{
  SerializeNameDefault(mName, cUnknownString);
  SerializeName(mAxes);
  SerializeName(mButtons);
}

bool RawControlMapping::IsSame(RawControlMapping* map)
{
  // Compare the
  if (this->mName != map->mName)
    return false;

  // Compare the number of buttons (names should all match)
  if (this->mButtons.Size() != map->mButtons.Size())
    return false;

  // Compare the size of the axes
  if (this->mAxes.Size() != map->mAxes.Size())
    return false;

  // Compare names of the axes
  for (size_t i = 0; i < this->mAxes.Size(); ++i)
  {
    // Compare the axis names
    if (this->mAxes[i].Name != map->mAxes[i].Name)
    {
      return false;
    }
  }

  // Otherwise, it was mostly the same!
  return true;
}

// JoystickEvent
LightningDefineType(JoystickEvent, builder, type)
{
  type->HandleManager = LightningManagerId(PointerManager);
  PlasmaBindDocumented();

  LightningBindFieldProperty(mButton);
  LightningBindFieldProperty(mJoystick);
}

JoystickEvent::JoystickEvent(Joystick* joystick, int button)
{
  mJoystick = joystick;
  mButton = button;
}

LightningDefineType(Joystick, builder, type)
{
  type->HandleManager = LightningManagerId(PointerManager);
  PlasmaBindDocumented();

  LightningBindGetterProperty(IsActive);
  LightningBindGetterProperty(Name);

  LightningBindMethod(GetAxisValue);
  LightningBindMethod(GetAxisName);
  LightningBindMethod(GetAxisIndex);
  LightningBindMethod(GetAxisValueByName);

  LightningBindGetter(DisabledValue);

  LightningBindMethod(GetButtonValue);

  LightningBindGetterProperty(ButtonCount);
  LightningBindGetterProperty(AxisCount);

  LightningBindMethod(LoadInputMapping);
  LightningBindMethod(SaveInputMapping);

  LightningBindMethod(StartCalibration);
  LightningBindMethod(EndCalibration);
  LightningBindMethod(Calibrating);

  PlasmaBindEvent(Events::JoystickButtonDown, JoystickEvent);
  PlasmaBindEvent(Events::JoystickButtonUp, JoystickEvent);
  PlasmaBindEvent(Events::JoystickUpdated, JoystickEvent);
}

Joystick::Joystick()
{
  // We are not yet active
  mDeviceHandle = 0;
  mHardwareGuid = 0;
  mIsActive = false;
  mRawMapping = nullptr;
  mAutoCalibrate = false;

  // Clear out the joystick state
  InactiveClear();
}

Joystick::~Joystick()
{
  SafeDelete(mRawMapping);
}

void Joystick::InactiveClear()
{
  // Clear the buttons (bits)
  mButtons = 0;

  // Clear out all the axes
  for (size_t i = 0; i < mAxes.Size(); ++i)
  {
    if (mRawMapping && mRawMapping->mAxes[i].CanBeDisabled)
      mAxes[i] = GetDisabledValue();
    else
      mAxes[i] = 0.0f;
  }
}

String Joystick::GetName()
{
  return mName;
}

float Joystick::GetAxisValue(int index)
{
  ReturnIf(index >= int(mRawMapping->mAxes.Size()), 0.0f, "Joystick axis index was out of bounds");

  return mAxes[index];
}

float Joystick::GetDisabledValue()
{
  return FLT_MAX;
}

String Joystick::GetAxisName(int index)
{
  ReturnIf(index >= int(mRawMapping->mAxes.Size()), "Invalid", "Joystick axis index was out of bounds");

  return mRawMapping->mAxes[index].Name;
}

int Joystick::GetAxisIndex(StringParam name)
{
  for (size_t i = 0; i < mRawMapping->mAxes.Size(); ++i)
  {
    if (mRawMapping->mAxes[i].Name == name)
    {
      return i;
    }
  }

  // DoNotifyError("Joystick Error",
  //  String::Format("The axis with the name '%s' was not found",
  //  name.c_str()));
  return -1;
}

float Joystick::GetAxisValueByName(StringParam name)
{
  int index = GetAxisIndex(name);
  return GetAxisValue(index);
}

uint Joystick::GetButtonCount()
{
  return (uint)mRawMapping->mButtons.Size();
}

uint Joystick::GetAxisCount()
{
  return (uint)mRawMapping->mAxes.Size();
}

bool Joystick::GetButtonValue(uint index)
{
  return (mButtons & (1 << index)) != 0;
}

bool Joystick::GetIsActive()
{
  return mIsActive;
}

void Joystick::LoadInputMapping(StringParam name)
{
  TextBlock* block = TextBlockManager::FindOrNull(name);
  if (block)
  {
    RawControlMapping* mapping = new RawControlMapping();

    DataBlock dataBlock((::byte*)block->Text.Data(), block->Text.SizeInBytes());
    LoadFromDataBlock(*mapping, dataBlock, DataFileFormat::Text);

    this->InternalSetInputMapping(mapping);
  }
}

void Joystick::SaveInputMapping(StringParam name)
{
  TextBlock* block = TextBlockManager::FindOrNull(name);
  if (block && mRawMapping)
  {
    DataBlock dataBlock = SaveToDataBlock(*mRawMapping, DataFileFormat::Text);
    if (dataBlock)
    {
      block->Text = StringRange((cstr)dataBlock.Data, (cstr)dataBlock.Data, (cstr)dataBlock.Data + dataBlock.Size);
      block->mContentItem->SaveContent();
      FreeBlock(dataBlock);
    }
  }
}

RawControlMapping* Joystick::GetInputMapping()
{
  return mRawMapping;
}

void Joystick::InternalSetInputMapping(RawControlMapping* map)
{
  SafeDelete(mRawMapping);
  mRawMapping = map;
  mAxes.Resize(map->mAxes.Size());
  InactiveClear();
}

void Joystick::InternalSetInputMappingIfDifferent(RawControlMapping* map)
{
  // If our mappings are the same, just keep our own (keeps calibration data)
  if (mRawMapping->IsSame(map))
  {
    SafeDelete(map);
  }
  else
  {
    // For some reason our mappings changed, we need to update ourselves
    InternalSetInputMapping(map);
  }
}

void Joystick::StartCalibration(void)
{
  // Move all axes in their full range of motion
  mAutoCalibrate = true;

  for (uint i = 0; i < mRawMapping->mAxes.Size(); ++i)
  {
    RawAxis& axis = mRawMapping->mAxes[i];
    axis.Min = (uint)-1;
    axis.Max = 0;
    axis.Mid = 0;
  }
}

void Joystick::EndCalibration(void)
{
  // Leave all axes in their resting position and throttle all the way down
  mAutoCalibrate = false;
}

bool Joystick::Calibrating(void)
{
  return mAutoCalibrate;
}

void Joystick::RawSetAxis(uint index, uint rawValue)
{
  ReturnIf(mIsActive == false, , "We should not be updating a joystick that is not active");

  float value = 0.0f;
  RawAxis& axis = mRawMapping->mAxes[index];

  if (this->mAutoCalibrate && axis.CanCalibrate)
  {
    axis.Max = Math::Max(axis.Max, rawValue);
    axis.Min = Math::Min(axis.Min, rawValue);
    axis.Mid = rawValue;
  }

  // If this axis has a mid range value
  if (axis.UseMid)
  {
    if (rawValue >= axis.Mid)
    {
      if (axis.Max != axis.Mid)
      {
        // Upper range [0, 1]
        value = (float(rawValue) - axis.Mid) / (float(axis.Max) - axis.Mid);
        value = Math::Clamp(value, 0.0f, 1.0f);
      }
    }
    else
    {
      if (axis.Min != axis.Mid)
      {
        // Lower range [-1, 0]
        value = (float(rawValue) - axis.Mid) / (float(axis.Min) - axis.Mid);
        value = Math::Clamp(-value, -1.0f, 0.0f);
      }
    }
  }
  // Otherwise we just use the min and max to determine our value
  else
  {
    value = (float(rawValue) - axis.Min) / (float(axis.Max) - axis.Min);
    value = value * 2.0f - 1.0f;
    value = Math::Clamp(value, -1.0f, 1.0f);
  }

  if (Math::Abs(value) < axis.DeadZonePercent)
  {
    value = 0.0f;
  }

  if (axis.Reversed)
    value = -value;

  this->mAxes[index] = value;
}

void Joystick::RawSetAxisDisabled(uint index)
{
  this->mAxes[index] = GetDisabledValue();
}

void Joystick::RawSetButtons(uint newState)
{
  ReturnIf(mIsActive == false, , "We should not be updating a joystick that is not active");

  for (uint i = 0; i < GetButtonCount(); ++i)
  {
    // check the button of index i
    uint oldFlag = mButtons & (1 << i);
    uint newFlag = newState & (1 << i);

    // If there is no state change on this button, there is no event to send
    if (oldFlag == newFlag)
      continue;

    if (oldFlag == 0)
    {
      // If the button wasn't down and now is, send a button down
      JoystickEvent e(this, i);
      this->DispatchEvent(Events::JoystickButtonDown, &e);
    }
    else
    {
      JoystickEvent e(this, i);
      this->DispatchEvent(Events::JoystickButtonUp, &e);
    }
  }

  mButtons = newState;
}

void Joystick::SignalUpdated()
{
  JoystickEvent event(this, 0);
  this->DispatchEvent(Events::JoystickUpdated, &event);
}

void Joystick::RawProcess(DataBlock dataBlock)
{
  ReturnIf(mIsActive == false, , "We should not be updating a joystick that is not active");

  ErrorIf(this->mRawMapping == nullptr,
          "We should not be calling raw process with a raw data block when we "
          "have no mapping set");

  RawControlMapping* mapping = this->mRawMapping;

  // Clear the buttons
  uint buttons = 0;

  // Scan all buttons
  for (uint i = 0; i < mapping->mButtons.Size(); ++i)
  {
    RawButton& button = mapping->mButtons[i];

    // Find the byte with the bit in it
    uint data = dataBlock.Data[button.Offset];

    // Check the bit
    uint mask = 1 << button.Bit;
    if ((data & mask) != 0)
    {
      buttons |= (1 << i);
    }
  }

  // Update the buttons (we'll also send out events for button changes)
  this->RawSetButtons(buttons);

  // Scan all mAxes
  for (uint i = 0; i < mapping->mAxes.Size(); ++i)
  {
    RawAxis& axis = mapping->mAxes[i];
    uint rawValue = dataBlock.Data[axis.Offset];

    // Initialize the axis value
    this->RawSetAxis(i, rawValue);
  }
}

LightningDefineType(Joysticks, builder, type)
{
  type->HandleManager = LightningManagerId(PointerManager);
  PlasmaBindDocumented();
  LightningBindGetterProperty(JoystickCount);
  LightningBindGetterProperty(Joysticks);

  PlasmaBindEvent(Events::JoystickFound, JoystickEvent);
  PlasmaBindEvent(Events::JoystickLost, JoystickEvent);
  PlasmaBindEvent(Events::JoysticksChanged, ObjectEvent);
}

Joysticks::Joysticks()
{
  // Make the joysticks system globally available
  PL::gJoysticks = this;
}

void Joysticks::DeactivateAll()
{
  forRange (Joystick* joyStick, mDeviceToJoystick.Values())
  {
    if (!joyStick->mIsActive)
      continue;

    // Set the joystick as not active and clear its state
    joyStick->mIsActive = false;
    joyStick->InactiveClear();

    // Dispatch a joystick lost message to both the joystick system and the
    // joystick itself
    JoystickEvent event(joyStick, 0);
    joyStick->DispatchEvent(Events::JoystickLost, &event);
    this->DispatchEvent(Events::JoystickLost, &event);
  }
}

Joysticks::~Joysticks()
{
  // Loop through all the joysticks we have available
  forRange (Joystick* joystick, mDeviceToJoystick.Values())
  {
    // Delete the allocated joystick object
    delete joystick;
  }
}

uint Joysticks::GetJoystickCount()
{
  return mDeviceToJoystick.Size();
}

Joystick* Joysticks::GetJoystickByDevice(OsHandle handle)
{
  // Give back the joystick at that index
  return mDeviceToJoystick.FindValue(handle, nullptr);
}

JoystickDeviceRange Joysticks::GetJoysticks()
{
  return mDeviceToJoystick.Values();
}

void Joysticks::AddJoystickDevice(const PlatformInputDevice& device)
{
  // Look for the joystick by device
  Joystick* joyStick = mDeviceToJoystick.FindValue(device.mDeviceHandle, nullptr);

  // This will either be 'stolen' below or will be deleted in the internal set
  // functions
  RawControlMapping* map = new RawControlMapping(device);

  // If we didn't find the joystick by device...
  if (joyStick == nullptr)
  {
    // Attempt to look for the joystick by guid
    joyStick = mGuidToJoystick.FindValue(device.mGuid, nullptr);

    // We found the joystick by guid, which means it's probably one that was
    // already plugged in! This actually may not be the case if it turns out two
    // joysticks had the same guid... which we try to guarantee they don't but
    // can't be certain We need to make sure this joystick isn't already active
    // (if it is, we need to make a new one)
    if (joyStick != nullptr && joyStick->mIsActive == false)
    {
      // Reactivate the joystick
      joyStick->mIsActive = true;

      // Generally the mappings should be the same if our guid matched, but we
      // should do this anyways
      joyStick->InternalSetInputMappingIfDifferent(map);

      // We need to update the device of the joystick
      mDeviceToJoystick.Erase(joyStick->mDeviceHandle);
      joyStick->mDeviceHandle = device.mDeviceHandle;
      mDeviceToJoystick.Insert(device.mDeviceHandle, joyStick);
    }
    else
    {
      // Create a new joystick
      joyStick = new Joystick();
      joyStick->mIsActive = true;
      joyStick->mName = device.mName;
      joyStick->mDeviceHandle = device.mDeviceHandle;
      joyStick->mHardwareGuid = device.mGuid;
      joyStick->InternalSetInputMapping(map);

      // Add the joystick to the maps
      mDeviceToJoystick.InsertOrError(device.mDeviceHandle, joyStick);
      mGuidToJoystick.InsertNoOverwrite(device.mGuid, joyStick);
    }
  }
  else
  {
    // Make sure the joystick hasn't already been activated
    ErrorIf(joyStick->mIsActive,
            "Someone else activated the joystick, but two devices can't have "
            "the same device id!");

    // Reactivate the joystick
    joyStick->mIsActive = true;

    // Only set the new input mapping in the rare case that it's different
    // (usually only when a custom mapping is loaded)
    joyStick->InternalSetInputMappingIfDifferent(map);
  }

  // Dispatch a joystick found message to both the joystick system
  // and the joystick itself (useful when it is deactivated then reactivated)
  JoystickEvent event(joyStick, 0);
  joyStick->DispatchEvent(Events::JoystickFound, &event);
  this->DispatchEvent(Events::JoystickFound, &event);
}

void Joysticks::JoysticksChanged()
{
  ObjectEvent event(this);
  this->DispatchEvent(Events::JoysticksChanged, &event);
}

} // namespace Plasma
