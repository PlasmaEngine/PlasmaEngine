// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

/// Represents a unique hashable identifier for a type-cast. Currently
/// type-casts are not function calls in lightning so we can't use a function
/// pointer to resolve them.
struct TypeCastKey
{
  TypeCastKey();
  TypeCastKey(Lightning::Type* fromType, Lightning::Type* toType);

  size_t Hash() const;
  bool operator==(const TypeCastKey& rhs) const;

  Pair<Lightning::Type*, Lightning::Type*> mPair;
};

/// Represents a unique hashable identifier for unary operators. Currently
/// operators are not function calls in lightning so we can't use a function pointer
/// to resolve them.
struct UnaryOperatorKey
{
  UnaryOperatorKey();
  UnaryOperatorKey(Lightning::Type* type, Lightning::Grammar::Enum op);

  size_t Hash() const;
  bool operator==(const UnaryOperatorKey& rhs) const;

  Pair<Lightning::Type*, int> mPair;
};

/// Represents a unique hashable identifier for binary operators. Currently
/// operators are not function calls in lightning so we can't use a function pointer
/// to resolve them.
struct BinaryOperatorKey
{
  BinaryOperatorKey();
  BinaryOperatorKey(Lightning::Type* type1, Lightning::Type* type2, Lightning::Grammar::Enum op);

  size_t Hash() const;
  bool operator==(const BinaryOperatorKey& rhs) const;

  typedef Pair<Lightning::Type*, Lightning::Type*> LightningTypePair;
  Pair<LightningTypePair, int> mPair;
};

} // namespace Plasma
