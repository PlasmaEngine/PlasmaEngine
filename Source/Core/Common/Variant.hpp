// MIT Licensed (see LICENSE.md).
#pragma once
#include "VariantConfig.hpp"

namespace Plasma
{

//                                  Variant //

/// Variant is a fixed-size buffer capable of managing a single value of any
/// arbitrary C++ type. Useful for dynamic programming and storing complex user
/// data. Variant utilizes automatically generated native type meta. A variant
/// may contain any native type which has both an accessible copy constructor
/// and destructor. Stored values must be retrieved (using Is/Get/etc.) exactly
/// as the types they were assigned as, ignoring qualifications. Variant cannot
/// implicitly up-cast nor down-cast polymorphic types. Higher or lower types
/// are considered to be completely unrelated. All types are first unqualified
/// (stripped of their top-most reference and cv-qualifiers) on assignment,
/// retrieval, and comparison. Variant is optimized to work best with small
/// types which can be directly constructed within variant's fixed-size local
/// buffer. Larger types, which do not fit within variant's fixed-size local
/// buffer, are constructed on the heap and internally managed by pointer
/// instead. Regardless of whether or not a stored type is large or small, the
/// user does not need to change how they interact with variant. The distinction
/// between large and small types is only made known for performance
/// considerations (large types cause memory allocation on assignment).
class PlasmaShared Variant
{
public:
  /// Constructs an empty variant
  Variant();

  /// Constructs a variant by copying the specified native type and value to be
  /// managed (If nativeType is null, the variant is simply default constructed)
  /// (If valueAddress is null, the variant is assigned a default constructed
  /// value of the specified native type instead)
  explicit Variant(NativeType* nativeType, const void* valueAddress = nullptr);

  /// Constructs a variant by copying the specified value to be managed
  template <typename T>
  explicit Variant(const T& rhs);
  /// Constructs a variant by moving the specified value to be managed
  template <typename T>
  explicit Variant(MoveReference<T> rhs);

  /// Constructs a variant by copying the value to be managed, if any, from the
  /// specified variant
  Variant(const Variant& rhs);
  /// Constructs a variant by moving the value to be managed, if any, from the
  /// specified variant
  Variant(MoveReference<Variant> rhs);

  /// Destructs the variant (clears any stored value)
  ~Variant();

  //
  // Member Operators
  //

  /// Clears the variant and copies the specified value to be managed
  template <typename T>
  Variant& operator=(const T& rhs);
  /// Clears the variant and moves the specified value to be managed
  template <typename T>
  Variant& operator=(MoveReference<T> rhs);

  /// Clears the variant and copies the value to be managed, if any, from the
  /// specified variant
  Variant& operator=(const Variant& rhs);
  /// Clears the variant and moves the value to be managed, if any, from the
  /// specified variant
  Variant& operator=(MoveReference<Variant> rhs);

  /// Returns the stored value if the variant stored type is T, else asserts and
  /// returns an invalid T
  template <typename T>
  explicit operator T() const;

  /// Returns true if this variant is equivalent to rhs (same stored type and
  /// value), else false
  template <typename T>
  bool operator==(const T& rhs) const;
  /// Returns true if this variant is equivalent to rhs (same stored type and
  /// value), else false
  bool operator==(const Variant& rhs) const;

  /// Returns true if this variant is not equivalent to rhs (different stored
  /// type or value), else false
  template <typename T>
  bool operator!=(const T& rhs) const;
  /// Returns true if this variant is not equivalent to rhs (different stored
  /// type or value), else false
  bool operator!=(const Variant& rhs) const;

  //
  // Stored Value Access
  //

  /// Returns true if the variant stored type is the specified type T, else
  /// false
  template <typename T>
  bool Is() const;
  /// Returns true if the variant stored type is the specified native type, else
  /// false
  bool Is(NativeType* nativeType) const;

  /// Returns the stored value if the variant stored type is T, else asserts and
  /// returns an invalid T
  template <typename T, typename UnqualifiedType = typename Decay<T>::Type>
  UnqualifiedType& GetOrError() const;
  /// Returns the stored value if the variant stored type is T, else
  /// defaultValue
  template <typename T, typename UnqualifiedType = typename Decay<T>::Type>
  UnqualifiedType& GetOrDefault(const UnqualifiedType& defaultValue = UnqualifiedType()) const;
  /// Returns a pointer to the stored value if the variant stored type is T,
  /// else nullptr
  template <typename T, typename UnqualifiedType = typename Decay<T>::Type>
  UnqualifiedType* GetOrNull() const;

  /// Returns a pointer to the stored value if the variant is non-empty, else
  /// nullptr
  void* GetData() const;

  /// Returns the hash of the stored value if the variant is non-empty, else 0
  size_t Hash() const;

  /// Returns the string representation of the stored value if the variant is
  /// non-empty, else String()
  String ToString(bool shortFormat = false) const;

  /// Parses the string as a value of the specified type T and assigns the
  /// parsed value to the variant, else clears the variant
  template <typename T>
  void ToValue(StringRange range);
  /// Parses the string as a value of the specified native type and assigns the
  /// parsed value to the variant, else clears the variant
  void ToValue(StringRange range, NativeType* nativeType);
  /// Parses the string as a value of the variant's stored type and assigns the
  /// parsed value to the variant, else clears the variant
  void ToValue(StringRange range);

  /// Returns true if the variant does not contain a stored value, else false
  /// (Empty implies the variant is also zeroed. However the reverse may not be
  /// true.
  ///  For example, variant may be managing a small stored type with a "zeroed"
  ///  data state.)
  bool IsEmpty() const;
  /// Returns true if the variant contains a stored value, else false
  bool IsNotEmpty() const;

  /// Returns true if the variant's local buffer is completely zeroed, else
  /// false (Zeroed does not imply the variant is empty!
  ///  For example, variant may be managing a small stored type with a "zeroed"
  ///  data state.)
  bool IsZeroed() const;
  /// Returns true if the variant's local buffer is not completely zeroed, else
  /// false
  bool IsNotZeroed() const;

  /// Returns true if the variant is storing a small type (fits within variant's
  /// local buffer), else false
  bool IsSmallType() const;
  /// Returns true if the variant is storing a large type (does not fit within
  /// variant's local buffer), else false
  bool IsLargeType() const;

  /// Returns true if the specified type T is a small type (fits within
  /// variant's local buffer), else false
  template <typename T>
  static bool IsSmallType();
  /// Returns true if the specified type T is a large type (does not fit within
  /// variant's local buffer), else false
  template <typename T>
  static bool IsLargeType();

  /// Returns true if the specified native type is a small type (fits within
  /// variant's local buffer), else false
  static bool IsSmallType(NativeType* nativeType);
  /// Returns true if the specified native type is a large type (does not fit
  /// within variant's local buffer), else false
  static bool IsLargeType(NativeType* nativeType);

  /// Returns the stored value's native type if the variant is non-empty, else
  /// nullptr
  NativeType* GetNativeType() const;
  /// Returns the stored value's native type ID if the variant is non-empty,
  /// else cInvalidNativeTypeId
  NativeTypeId GetNativeTypeId() const;

  //
  // Stored Value Management
  //

  /// Clears the variant and assigns a default constructed value of the
  /// specified native type to be managed
  template <typename T>
  void DefaultConstruct();
  /// Clears the variant and assigns a default constructed value of the
  /// specified native type to be managed (If nativeType is null, the variant is
  /// simply cleared)
  void DefaultConstruct(NativeType* nativeType = nullptr);

  /// Clears the variant and copies the specified native type and value to be
  /// managed (If nativeType is null, the variant is simply cleared) (If
  /// valueAddress is null, the variant is assigned a default constructed value
  /// of the specified native type instead)
  void Assign(NativeType* nativeType = nullptr, const void* valueAddress = nullptr);

  /// Clears the variant and copies the specified value to be managed
  template <typename T>
  void Assign(const T& rhs);
  /// Clears the variant and moves the specified value to be managed
  template <typename T>
  void Assign(MoveReference<T> rhs);

  /// Clears the variant and copies the value to be managed, if any, from the
  /// specified variant
  void Assign(const Variant& rhs);
  /// Clears the variant and moves the value to be managed, if any, from the
  /// specified variant
  void Assign(MoveReference<Variant> rhs);

  /// Destroys the stored value currently being managed, if any, and plasmaes out
  /// the variant (Safe to call multiple times. After this call, the variant is
  /// guaranteed to be empty.)
  void Clear();

  //
  // Primitive Member Access (Arithmetic Types Only)
  //

  /// (Only defined for arithmetic types)
  /// Returns the stored value's primitive member at the specified index if the
  /// variant stored type is T, else asserts and returns an invalid primitive
  template <typename T,
            typename UnqualifiedType = typename Decay<T>::Type,
            TF_ENABLE_IF(IsBasicNativeTypeArithmetic<UnqualifiedType>::Value),
            typename PrimitiveType = typename BasicNativeTypePrimitiveMembers<UnqualifiedType>::Type>
  PrimitiveType& GetPrimitiveMemberOrError(size_t index) const;

  /// (Only defined for arithmetic types)
  /// Returns the stored value's primitive member at the specified index if the
  /// variant stored type is T, else defaultValue
  template <typename T,
            typename UnqualifiedType = typename Decay<T>::Type,
            TF_ENABLE_IF(IsBasicNativeTypeArithmetic<UnqualifiedType>::Value),
            typename PrimitiveType = typename BasicNativeTypePrimitiveMembers<UnqualifiedType>::Type>
  PrimitiveType& GetPrimitiveMemberOrDefault(size_t index, const PrimitiveType& defaultValue = PrimitiveType()) const;

  /// (Only defined for arithmetic types)
  /// Returns a pointer to the stored value's primitive member at the specified
  /// index if the variant stored type is T, else nullptr
  template <typename T,
            typename UnqualifiedType = typename Decay<T>::Type,
            TF_ENABLE_IF(IsBasicNativeTypeArithmetic<UnqualifiedType>::Value),
            typename PrimitiveType = typename BasicNativeTypePrimitiveMembers<UnqualifiedType>::Type>
  PrimitiveType* GetPrimitiveMemberOrNull(size_t index) const;

private:
  //
  // Internal Helper Functions
  //

  /// (Assumes there is a stored type and, if it is a large type, that the heap
  /// buffer has been allocated) Returns a pointer to the buffer where the
  /// stored value should exist (may be valid, destructed, or plasmaed)
  void* InternalGetData() const;

  /// Plasmaes the variant's local buffer (memsets it all to 0)
  void InternalPlasmaLocalBuffer();

  /// (Assumes the variant has a large stored type and has allocated a heap
  /// buffer) Plasmaes the variant's heap buffer (memsets it all to 0)
  void InternalPlasmaHeapBuffer();

  /// (Assumes the variant has a large stored type and has not allocated a heap
  /// buffer) Allocates the variant's heap buffer Does not plasma the heap buffer
  void InternalAllocateHeapBuffer();

  /// (Assumes the variant has a large stored type and has allocated a heap
  /// buffer) Frees the variant's heap buffer
  void InternalFreeHeapBuffer();

  /// Destructs the variant's stored value, if any
  /// Does not affect stored type status and does not free any memory
  void InternalDestructStoredValue();

  /// (Assumes the current stored value, if any, has already been destructed)
  /// (If the variant currently has a large stored type, assumes the heap buffer
  /// has not been freed yet) Prepares internal buffers (by
  /// allocating/plasmaing/freeing) to be replaced with a value of the new stored
  /// type
  void InternalReplaceStoredType(NativeType* newStoredType);

  /// (Assumes the appropriate buffer exists and has been plasmaed)
  /// Default constructs a new stored value in the appropriate plasmaed buffer
  /// Alternatively plasma-initializes if the stored type is missing a default
  /// constructor
  void InternalDefaultConstructValue();

  /// (Assumes the appropriate buffer exists and has been plasmaed)
  /// Copy constructs a new stored value in the appropriate plasmaed buffer
  void InternalCopyConstructValue(const void* source);

  /// (Assumes the appropriate buffer exists and has been plasmaed)
  /// Move constructs a new stored value in the appropriate plasmaed buffer
  /// Alternatively copy constructs if the stored type is missing a move
  /// constructor
  void InternalMoveConstructValue(void* source);

  /// (Assumes there is a stored value and that it is the same type as rhs)
  /// Returns true if the stored value is equal to rhs, else false
  /// Alternatively returns false if the stored type is missing a valid compare
  /// policy
  bool InternalEqualToValue(const void* rhs) const;

  /// (Assumes there is a stored value)
  /// Returns the hash of the stored value
  /// Alternatively returns 0 if the stored type is missing a valid hash policy
  size_t InternalHashStoredValue() const;

  /// (Assumes there is a stored value)
  /// Returns the string representation of the stored value
  /// Alternatively returns String() if the stored type is missing a global to
  /// string function
  String InternalStoredValueToString(bool shortFormat) const;

  /// (Assumes there is a stored value)
  /// Parses the string as a value of the stored type and assigns the parsed
  /// value over the stored value Alternatively default constructs if the stored
  /// type is missing a global to value function
  void InternalStringToStoredValue(StringRange range);

public:
  //
  // Data
  //

  /// Stored value's native type
  /// (Null means the variant is empty)
  NativeType* mNativeType;

  /// Fixed-size local buffer
  /// Contains the small stored value or a pointer to the large stored value
  /// (Should be plasmaed if the variant is empty)
  union {
    ::byte mData[16];
    int mDataAsInts[4];
    float mDataAsFloats[4];
    // String mDataAsString;
    void* mDataAsPointer;
  };
};

// Convenience typedef
typedef const Variant& VariantParam;

/// Variant Move-Without-Destruction Operator
template <>
struct PlasmaShared MoveWithoutDestructionOperator<Variant>
{
  static inline void MoveWithoutDestruction(Variant* dest, Variant* source)
  {
    // Move source variant to destination
    new (dest) Variant(PlasmaMove(*source));

    // Source variant should now be empty, which means it should also be plasmaed,
    // which means it's safe to skip calling it's destructor (As a sanity check
    // we're going to explicitly verify that the source variant is now both
    // empty and plasmaed.
    //  If this assertion fails, then the source variant's local buffer was
    //  likely modified by some external means.)
    Assert(source->IsEmpty() && source->IsZeroed());
  }
};

//
// Variant ToString / ToValue
//

/// Returns the string representation of the stored value if the variant is
/// non-empty, else String()
PlasmaShared inline String ToString(const Variant& value, bool shortFormat = false);

/// Parses the string as a value of the specified type T and assigns the parsed
/// value to the variant, else clears the variant
template <typename T>
PlasmaSharedTemplate inline void ToValue(StringRange range, Variant& value);
/// Parses the string as a value of the specified native type and assigns the
/// parsed value to the variant, else clears the variant
PlasmaShared inline void ToValue(StringRange range, Variant& value, NativeType* nativeType);
/// Parses the string as a value of the variant's stored type and assigns the
/// parsed value to the variant, else clears the variant
PlasmaShared inline void ToValue(StringRange range, Variant& value);

} // namespace Plasma

// Includes
#include "Variant.inl"
