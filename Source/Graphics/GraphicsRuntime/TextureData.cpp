// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(TextureData, builder, type)
{
  PlasmaBindDocumented();

  LightningBindConstructor(TextureFormat::Enum, int, int);
  LightningBindDestructor();
  type->CreatableInScript = true;

  LightningBindFieldGetter(mFormat);
  LightningBindFieldGetter(mWidth);
  LightningBindFieldGetter(mHeight);
  LightningBindFieldGetter(mPixelCount);

  LightningBindOverloadedMethod(Get, LightningInstanceOverload(Vec4, uint));
  LightningBindOverloadedMethod(Get, LightningInstanceOverload(Vec4, uint, uint));
  LightningBindOverloadedMethod(Set, LightningInstanceOverload(void, uint, Vec4));
  LightningBindOverloadedMethod(Set, LightningInstanceOverload(void, uint, uint, Vec4));
}

TextureData::TextureData(TextureFormat::Enum format, int width, int height) : mPixelCount(0), mData(nullptr)
{
  if (IsColorFormat(format) == false)
  {
    DoNotifyException("Error", "Unsupported format.");
    return;
  }

  width = Math::Clamp(width, 1, 4096);
  height = Math::Clamp(height, 1, 4096);

  mFormat = format;
  mWidth = width;
  mHeight = height;
  mPixelCount = width * height;

  mPixelSize = GetPixelSize(format);
  mDataSize = mPixelCount * mPixelSize;
  mData = new byte[mDataSize];
  memset(mData, 0, mDataSize);
}

TextureData::~TextureData()
{
  delete[] mData;
}

Vec4 TextureData::Get(uint index)
{
  Vec4 value = Vec4::cZero;

  if (index >= mPixelCount)
  {
    DoNotifyException("Error", "Index out of range.");
    return value;
  }

  uint dataIndex = index * mPixelSize;
  ReadPixelData(mData, dataIndex, value, mFormat);

  return value;
}

Vec4 TextureData::Get(uint x, uint y)
{
  if (x >= mWidth || y >= mHeight)
  {
    DoNotifyException("Error", "Index out of range.");
    return Vec4::cZero;
  }

  return Get(x + y * mWidth);
}

void TextureData::Set(uint index, Vec4 value)
{
  if (index >= mPixelCount)
  {
    DoNotifyException("Error", "Index out of range.");
    return;
  }

  uint dataIndex = index * mPixelSize;
  SetPixelData(mData, dataIndex, value, mFormat);
}

void TextureData::Set(uint x, uint y, Vec4 value)
{
  if (x >= mWidth || y >= mHeight)
  {
    DoNotifyException("Error", "Index out of range.");
    return;
  }

  Set(x + y * mWidth, value);
}

} // namespace Plasma
