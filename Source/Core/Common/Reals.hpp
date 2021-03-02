// MIT Licensed (see LICENSE.md).
#pragma once

#include "Standard.hpp"

namespace Math
{

typedef float real;
typedef short half;

// a pointer of the given type
typedef real* RealPointer;
// a const pointer of the given type
typedef const real* ConstRealPointer;
// a reference of the given type
typedef real& RealRef;
// a const reference of the given type
typedef const real& ConstRealRef;

const uint cX = 0;
const uint cY = 1;
const uint cZ = 2;
const uint cW = 3;

// these cannot be constants
extern const real& cInfinite;

extern const real cPi;
extern const real cTwoPi;

// Golden ratio!
const real cGoldenRatio = real(1.6180339887498948482045868343656);

PlasmaShared real Epsilon(void);
PlasmaShared real PositiveMax(void);
PlasmaShared real PositiveMin();
PlasmaShared bool Equal(real lhs, real rhs);
PlasmaShared bool Equal(real lhs, real rhs, real epsilon);
PlasmaShared bool NotEqual(real lhs, real rhs);
PlasmaShared bool IsZero(real val);
PlasmaShared bool IsNegative(real number);
PlasmaShared bool IsPositive(real number);
PlasmaShared bool LessThan(real lhs, real rhs);
PlasmaShared bool LessThanOrEqual(real lhs, real rhs);
PlasmaShared bool GreaterThan(real lhs, real rhs);
PlasmaShared bool GreaterThanOrEqual(real lhs, real rhs);
PlasmaShared real Sqrt(real val);
PlasmaShared bool SafeSqrt(real val, real& result);
PlasmaShared real Rsqrt(real val);
PlasmaShared real Sq(real sqrt);
PlasmaShared real Pow(real base, real exp);
PlasmaShared real Log(real val);
PlasmaShared real Log(real val, real base);
PlasmaShared real Log10(real val);
PlasmaShared real Log2(real val);
PlasmaShared real Exp(real val);
PlasmaShared real Exp2(real val);
PlasmaShared u8 Abs(u8 val);
PlasmaShared u16 Abs(u16 val);
PlasmaShared u32 Abs(u32 val);
PlasmaShared u64 Abs(u64 val);
PlasmaShared s8 Abs(s8 val);
PlasmaShared s16 Abs(s16 val);
PlasmaShared s32 Abs(s32 val);
PlasmaShared s64 Abs(s64 val);
PlasmaShared float Abs(float val);
PlasmaShared double Abs(double val);
PlasmaShared real FMod(real dividend, real divisor);
PlasmaShared bool SafeFMod(real dividend, real divisor, real& result);
PlasmaShared real GetSign(real val);
PlasmaShared int Sign(real val);
PlasmaShared int Sign(int val);
PlasmaShared real Cos(real val);
PlasmaShared real Sin(real val);
PlasmaShared real Tan(real angle);
PlasmaShared real Cot(real angle);
PlasmaShared real Cosh(real val);
PlasmaShared real Sinh(real val);
PlasmaShared real Tanh(real angle);
PlasmaShared real ArcCos(real angle);
PlasmaShared real ArcSin(real angle);
PlasmaShared real ArcTan(real angle);
PlasmaShared real ArcTan2(real y, real x);
PlasmaShared bool SafeArcCos(real radians, real& result);
PlasmaShared bool SafeArcSin(real radians, real& result);
PlasmaShared real RadToDeg(real radians);
PlasmaShared real DegToRad(real degrees);
PlasmaShared real Fractional(real val);
PlasmaShared real Round(real val);
PlasmaShared real Round(real value, int places);
PlasmaShared real Round(real value, int places, int base);
PlasmaShared real Truncate(real val);
PlasmaShared real Truncate(real val, int places);
PlasmaShared real Truncate(real val, int places, int base);
PlasmaShared real Ceil(real val);
PlasmaShared real Ceil(real val, int places);
PlasmaShared real Ceil(real val, int places, int base);
PlasmaShared real Floor(real val);
PlasmaShared real Floor(real val, int places);
PlasmaShared real Floor(real val, int places, int base);
/// If y <= x then 1 is returned, otherwise 0 is returned.
PlasmaShared real Step(real y, real x);
PlasmaShared int CountBits(int value);
PlasmaShared bool IsValid(real val);

PlasmaShared double DoublePositiveMax();
PlasmaShared double DoublePositiveMin();
PlasmaShared byte BytePositiveMax();
PlasmaShared byte BytePositiveMin();
PlasmaShared int IntegerPositiveMax();
PlasmaShared int IntegerNegativeMin();
PlasmaShared long long int DoubleIntegerPositiveMax();
PlasmaShared long long int DoubleIntegerNegativeMin();

template <typename T>
PlasmaSharedTemplate inline T Max(const T lhs, const T rhs)
{
  return lhs > rhs ? lhs : rhs;
}

template <typename T>
PlasmaSharedTemplate inline T Min(const T lhs, const T rhs)
{
  return lhs > rhs ? rhs : lhs;
}

template <typename T>
PlasmaSharedTemplate inline T Clamp(const T x, const T xMin, const T xMax)
{
  return Max(xMin, Min(x, xMax));
}

template <typename T>
PlasmaSharedTemplate inline T Clamp(const T value)
{
  return Clamp(value, T(0), T(1));
}

/// Clamps between min and max but it sets a bool saying whether or not a value
/// was clamped.
template <typename T>
PlasmaSharedTemplate inline T DebugClamp(const T x, const T xMin, const T xMax, bool& wasClamped)
{
  wasClamped = true;
  if (x < xMin)
    return xMin;
  if (x > xMax)
    return xMax;
  wasClamped = false;
  return x;
}

template <typename T>
PlasmaSharedTemplate inline T ClampIfClose(const T x, const T xMin, const T xMax, const T epsilon)
{
  real value = x < xMin && x > (xMin - epsilon) ? xMin : x;
  value = value > xMax && value < (xMax + epsilon) ? xMax : value;
  return value;
}

template <typename T>
PlasmaSharedTemplate inline bool TryClampIfClose(T& x, const T xMin, const T xMax, const T epsilon)
{
  if (x < xMin)
  {
    if (x > (xMin - epsilon))
      x = xMin;
    else
      return false;
  }
  if (x > xMax)
  {
    if (x < (xMax + epsilon))
      x = xMax;
    else
      return false;
  }
  return true;
}

template <typename T>
PlasmaSharedTemplate inline real InverseLerp(const T x, const T start, const T end)
{
  if (end == start)
  {
    return real(1.0);
  }

  return (x - start) / (end - start);
}

template <typename T>
PlasmaSharedTemplate inline real InverseLerpClamped(const T x, const T start, const T end)
{
  return Clamp(InverseLerp(x, start, end));
}

/// Checks to see if x is within the interval of [xMin, xMax]
template <typename T>
PlasmaSharedTemplate inline bool InRange(const T x, const T xMin, const T xMax)
{
  return ((xMin <= x) && (x <= xMax));
}

/// Checks to see if x is within the interval of (xMin, xMax)
template <typename T>
PlasmaSharedTemplate inline bool InBounds(const T x, const T xMin, const T xMax)
{
  return ((xMin < x) && (x < xMax));
}

template <typename T>
PlasmaSharedTemplate inline T Wrap(const T x, const T xMin, const T xMax)
{
  return (x < xMin) ? (x + (xMax - xMin)) : ((x > xMax) ? (x - (xMax - xMin)) : x);
}

template <typename T>
PlasmaSharedTemplate inline void Swap(T& a, T& b)
{
  T temp(a);
  a = b;
  b = temp;
}

template <typename Data, typename T>
PlasmaSharedTemplate inline Data Lerp(const Data& start, const Data& end, T interpolationValue)
{
  return (Data)((T(1.0) - interpolationValue) * start + interpolationValue * end);
}

template <typename Data>
PlasmaSharedTemplate inline Data SmoothStep(const Data& start, const Data& end, real t)
{
  t = Clamp((t - start) / (end - start));

  // 3t^2 - 2t^3
  return t * t * (3 - 2 * t);
}

} // namespace Math
