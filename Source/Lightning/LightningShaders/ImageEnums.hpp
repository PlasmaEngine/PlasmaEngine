#pragma once

#include "LightningShadersStandard.hpp"

namespace Plasma
{

DeclareEnum7(ImageDimension, Dim1D, Dim2D, Dim3D, Cube, Rect, Buffer, SubpassData);
DeclareEnum3(ImageDepthMode, None, Depth, Unknown);
DeclareEnum2(ImageArrayedMode, None, Arrayed);
DeclareEnum2(ImageMultiSampledMode, SingleSampled, MultieSampled);
DeclareEnum3(ImageSampledMode, RunTime, Sampling, NoSampling);

namespace ImageFormat
{
  enum Enum  { Unknown = 0, Rgba32f = 1, Rgba16f = 2, R32f = 3, Rgba8 = 4, Rgba32i = 21 };
  enum { Size = 6 };
  static const cstr Names[] = {"Unknown", "Rgba32f", "Rgba16f", "R32f", "Rgba8", "Rgba32i", nullptr};
  static const uint Values[] = {Unknown, Rgba32f, Rgba16f, R32f, Rgba8 , Rgba32i};
}//namespace ImageFormat

namespace ImageOperands
{
  enum Enum
  {
    None = 0,
    Bias = 0x1,
    Lod = 0x2,
    Grad = 0x4,
    ConstOffset = 0x8,
    Offset = 0x10,
    ConstOffsets = 0x20,
    Sample = 0x40,
    MinLod = 0x80,
  };
  enum { Size = 9 };
  static const cstr Names[] = {"None", "Bias", "Lod", "Grad", "ConstOffset", "Offset", "ConstOffsets", "Sample", "MinLod", nullptr};
  static const uint Values[] = {None, Bias, Lod, Grad, ConstOffset, Offset, ConstOffsets, Sample, MinLod};
}//namespace ImageOperands

}//namespace Plasma