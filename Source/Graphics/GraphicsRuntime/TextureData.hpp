// MIT Licensed (see LICENSE.md).

#pragma once

namespace Plasma
{

/// Modifiable texture data that can be used to upload to a runtime Texture
/// resource. All formats use one interface for get/set, all values are
/// converted to/from floats and unused channels are ignored. Integer formats
/// are represented in the normalized range [0, 1].
class TextureData
{
public:
  LightningDeclareType(TextureData, TypeCopyMode::ReferenceType);

  TextureData(TextureFormat::Enum format, int width, int height);
  TextureData(TextureFormat::Enum format, int width, int height, int depth);
  ~TextureData();

  /// Memory format of the stored pixel data.
  TextureFormat::Enum mFormat;

  /// Width of the texture data in pixels.
  uint mWidth;

  /// Height of the texture data in pixels.
  uint mHeight;

  /// Depth of the texture data in pixels.
  uint mDepth;

  /// Total number of pixels in texture data.
  uint mPixelCount;

  /// Returns the pixel values at the given index.
  Vec4 Get(uint index);

  /// Returns the pixel values at the given index.
  Vec4 Get(uint x, uint y);

  /// Returns the pixel values at the given index.
  Vec4 Get(uint x, uint y, uint z);

  /// Sets the pixel values at the given index.
  void Set(uint index, Vec4 value);

  /// Sets the pixel values at the given index.
  void Set(uint x, uint y, Vec4 value);

  /// Sets the pixel values at the given index.
  void Set(uint x, uint y, uint z, Vec4 value);

  // Internal

  uint mPixelSize;
  uint mDataSize;
  byte* mData;
};

} // namespace Plasma
