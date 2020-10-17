// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Lightning
{
LightningDefineType(Random, builder, type)
{
  type->AddAttribute(ImportDocumentation);

  LightningFullBindDestructor(builder, type, Random);
  LightningFullBindConstructor(builder, type, Random, nullptr, uint);
  LightningFullBindConstructor(builder, type, Random, nullptr);

  // Change to a property later
  LightningFullBindGetterSetter(
      builder, type, &Random::GetSeed, LightningNoOverload, &Random::SetSeed, LightningNoOverload, "Seed");
  LightningFullBindGetterSetter(
      builder, type, &Random::GetMaxInteger, LightningNoOverload, LightningNoSetter, LightningNoOverload, "MaxInteger");

  // Basic type randoms
  LightningFullBindMethod(builder, type, &Random::Boolean, LightningNoOverload, "Boolean", nullptr);
  LightningFullBindMethod(builder, type, &Random::Integer, LightningNoOverload, "Integer", nullptr);
  LightningFullBindMethod(builder, type, &Random::Real, LightningNoOverload, "Real", nullptr);
  LightningFullBindMethod(builder, type, &Random::DoubleReal, LightningNoOverload, "DoubleReal", nullptr);
  LightningFullBindMethod(builder, type, &Random::UnitReal2, LightningNoOverload, "UnitReal2", nullptr);
  LightningFullBindMethod(builder, type, &Random::Real2, LightningNoOverload, "Real2", "minLength, maxLength");
  LightningFullBindMethod(builder, type, &Random::UnitReal3, LightningNoOverload, "UnitReal3", nullptr);
  LightningFullBindMethod(builder, type, &Random::Real3, LightningNoOverload, "Real3", "minLength, maxLength");
  LightningFullBindMethod(builder, type, &Random::Quaternion, LightningNoOverload, "Quaternion", nullptr);

  // Range/variance helpers
  LightningFullBindMethod(builder, type, &Random::RangeInclusiveMax, LightningNoOverload, "RangeInclusiveMax", "min, max");
  LightningFullBindMethod(builder, type, &Random::RangeExclusiveMax, LightningNoOverload, "RangeExclusiveMax", "min, max");
  LightningFullBindMethod(builder, type, &Random::Range, LightningNoOverload, "Range", "min, max");
  LightningFullBindMethod(builder, type, &Random::DoubleRange, LightningNoOverload, "DoubleRange", "min, max");
  LightningFullBindMethod(builder, type, &Random::Variance, (int (Random::*)(int, int)), "Variance", "baseValue, variance");
  LightningFullBindMethod(
      builder, type, &Random::Variance, (float (Random::*)(float, float)), "Variance", "baseValue, variance");
  LightningFullBindMethod(
      builder, type, &Random::Variance, (double (Random::*)(double, double)), "Variance", "baseValue, variance");

  // Some more "user friendly" functions for designers
  LightningFullBindMethod(builder, type, &Random::DieRoll, LightningNoOverload, "DieRoll", nullptr);
  LightningFullBindMethod(builder, type, &Random::Probability, LightningNoOverload, "Probability", "probabilityOfTrue");
  LightningFullBindMethod(builder, type, &Random::CoinFlip, LightningNoOverload, "CoinFlip", nullptr);
  LightningFullBindMethod(builder, type, &Random::Rotation, LightningNoOverload, "Rotation", nullptr);

  // Bell curve (Gaussian) distribution
  LightningFullBindMethod(builder, type, &Random::BellCurve, (float (Random::*)()), "BellCurve", nullptr);
  LightningFullBindMethod(
      builder, type, &Random::BellCurve, (float (Random::*)(float, float)), "BellCurve", "center, range");
  LightningFullBindMethod(builder,
                      type,
                      &Random::BellCurve,
                      (float (Random::*)(float, float, float)),
                      "BellCurve",
                      "center, range, standardDeviation");
}

Random::Random()
{
  this->OriginalSeed = this->Generator.mSeed;
}

Random::Random(uint seed) : Generator(seed)
{
  this->OriginalSeed = seed;
}

void Random::SetSeed(uint seed)
{
  this->Generator = Math::Random(seed);
  this->OriginalSeed = seed;
}

uint Random::GetSeed()
{
  return this->OriginalSeed;
}

int Random::GetMaxInteger()
{
  return Math::Random::cRandMax;
}

bool Random::Boolean()
{
  return this->Generator.IntRangeInIn(0, 1) == 1;
}

int Random::Integer()
{
  return this->Generator.Next();
}

float Random::Real()
{
  return this->Generator.Float();
}

double Random::DoubleReal()
{
  return this->Generator.Double();
}

Math::Vector2 Random::UnitReal2()
{
  return this->Generator.PointOnUnitCircle();
}

Math::Vector2 Random::Real2(float minLength, float maxLength)
{
  return this->Generator.ScaledVector2(minLength, maxLength);
}

Math::Vector3 Random::UnitReal3()
{
  return this->Generator.PointOnUnitSphere();
}

Math::Vector3 Random::Real3(float minLength, float maxLength)
{
  return this->Generator.ScaledVector3(minLength, maxLength);
}

Lightning::Quaternion Random::Quaternion()
{
  return this->Generator.RotationQuaternion();
}

int Random::RangeInclusiveMax(int min, int max)
{
  return this->Generator.IntRangeInIn(min, max);
}

int Random::RangeExclusiveMax(int min, int max)
{
  return this->Generator.IntRangeInEx(min, max);
}

int Random::Variance(int base, int variance)
{
  return this->Generator.IntVariance(base, variance);
}

float Random::Range(float min, float max)
{
  return this->Generator.FloatRange(min, max);
}

double Random::DoubleRange(double min, double max)
{
  return this->Generator.DoubleRange(min, max);
}

float Random::Variance(float base, float variance)
{
  return this->Generator.FloatVariance(base, variance);
}

double Random::Variance(double base, double variance)
{
  return this->Generator.DoubleVariance(base, variance);
}

uint Random::DieRoll(uint sides)
{
  return this->Generator.DieRoll(sides);
}

bool Random::Probability(float probOfTrue)
{
  return this->Generator.Float() < probOfTrue;
}

bool Random::CoinFlip()
{
  return this->Generator.IntRangeInIn(0, 1) == 1;
}

Lightning::Quaternion Random::Rotation()
{
  return this->Generator.RotationQuaternion();
}

float Random::BellCurve()
{
  return this->Generator.BellCurve(0.5f, 0.5f, 1.0f);
}

float Random::BellCurve(float center, float range)
{
  return this->Generator.BellCurve(center, range, 1.0f);
}

float Random::BellCurve(float center, float range, float standardDeviation)
{
  return this->Generator.BellCurve(center, range, standardDeviation);
}
} // namespace Lightning
