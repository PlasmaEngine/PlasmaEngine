// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

TypeCastKey::TypeCastKey()
{
  mPair.first = nullptr;
  mPair.second = nullptr;
}

TypeCastKey::TypeCastKey(Lightning::Type* fromType, Lightning::Type* toType)
{
  mPair.first = fromType;
  mPair.second = toType;
}

size_t TypeCastKey::Hash() const
{
  return mPair.Hash();
}

bool TypeCastKey::operator==(const TypeCastKey& rhs) const
{
  return mPair == rhs.mPair;
}

UnaryOperatorKey::UnaryOperatorKey()
{
  mPair.first = nullptr;
  mPair.second = (int)Lightning::Grammar::Invalid;
}

UnaryOperatorKey::UnaryOperatorKey(Lightning::Type* type, Lightning::Grammar::Enum op)
{
  mPair.first = type;
  mPair.second = (int)op;
}

size_t UnaryOperatorKey::Hash() const
{
  return mPair.Hash();
}

bool UnaryOperatorKey::operator==(const UnaryOperatorKey& rhs) const
{
  return mPair == rhs.mPair;
}

BinaryOperatorKey::BinaryOperatorKey()
{
  mPair.first.first = nullptr;
  mPair.first.second = nullptr;
  mPair.second = (int)Lightning::Grammar::Invalid;
}

BinaryOperatorKey::BinaryOperatorKey(Lightning::Type* lhsType, Lightning::Type* rhsType, Lightning::Grammar::Enum op)
{
  mPair.first.first = lhsType;
  mPair.first.second = rhsType;
  mPair.second = (int)op;
}

size_t BinaryOperatorKey::Hash() const
{
  return mPair.Hash();
}

bool BinaryOperatorKey::operator==(const BinaryOperatorKey& rhs) const
{
  return mPair == rhs.mPair;
}

} // namespace Plasma
