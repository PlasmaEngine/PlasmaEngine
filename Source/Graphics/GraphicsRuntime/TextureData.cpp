// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(TextureData, builder, type)
{
  PlasmaBindDocumented();

  LightningBindConstructor(TextureFormat::Enum, int, int);
  LightningBindConstructor(TextureFormat::Enum, int, int, int);
  LightningBindDestructor();
  type->CreatableInScript = true;

  LightningBindFieldGetter(mFormat);
  LightningBindFieldGetter(mWidth);
  LightningBindFieldGetter(mHeight);
  LightningBindFieldGetter(mDepth);
  LightningBindFieldGetter(mPixelCount);

  LightningBindOverloadedMethod(Get, LightningInstanceOverload(Vec4, uint));
  LightningBindOverloadedMethod(Get, LightningInstanceOverload(Vec4, uint, uint));
  LightningBindOverloadedMethod(Get, LightningInstanceOverload(Vec4, uint, uint, uint));
  LightningBindOverloadedMethod(Set, LightningInstanceOverload(void, uint, Vec4));
  LightningBindOverloadedMethod(Set, LightningInstanceOverload(void, uint, uint, Vec4));
  LightningBindOverloadedMethod(Set, LightningInstanceOverload(void, uint, uint, uint, Vec4));
}

TextureData::TextureData(TextureFormat::Enum format, int width, int height)
{
    TextureData(format, width, height, 1);
}

TextureData::TextureData(TextureFormat::Enum format, int width, int height, int depth) : mPixelCount(0), mData(nullptr)
{
  if (IsColorFormat(format) == false)
  {
    DoNotifyException("Error", "Unsupported format.");
    return;
  }

  width = Math::Clamp(width, 1, 4096);
  height = Math::Clamp(height, 1, 4096);
  depth = Math::Clamp(height, 1, 4096);

  mFormat = format;
  mWidth = width;
  mHeight = height;
  mDepth = depth;
  mPixelCount = width * height * depth;

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
    return Get(x, y, 1);
}

Vec4 TextureData::Get(uint x, uint y, uint z)
{
  if (x >= mWidth || y >= mHeight || z >= mDepth)
  {
    DoNotifyException("Error", "Index out of range.");
    return Vec4::cZero;
  }

  return Get(x + y + z * mWidth);
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
    Set(x, y, 0, value);
}

void TextureData::Set(uint x, uint y, uint z, Vec4 value)
{
  if (x >= mWidth || y >= mHeight || z >= mDepth)
  {
    DoNotifyException("Error", "Index out of range.");
    return;
  }

  Set(x + y + z * mWidth, value);
}

} // namespace Plasma
