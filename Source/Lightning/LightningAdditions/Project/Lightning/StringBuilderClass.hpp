#pragma once
#ifndef LIGHTNING_STRING_BUILDER_HPP
#define LIGHTNING_STRING_BUILDER_HPP

namespace Lightning
{
  // Forward declarations
  typedef const Any& AnyParam;

  // Represents one utf-8 character in a string.
  class PlasmaShared Rune
  {
  public:
    LightningDeclareType(Rune, TypeCopyMode::ValueType);

    Rune();
    Rune(int value);
    Rune(const Plasma::Rune zeroRune);
    operator Plasma::Rune() const;

    static String ToString(const BoundType* type, const ::byte* data);

    int GetValue();
    void SetValue(int value);

    Plasma::Rune mValue;
  };

  // Plasma::Runes are converted to and treated as Lightning::Runes
  LightningDeclareDefineImplicitRedirectType(Plasma::Rune, Lightning::Rune, PlasmaShared);

  // String builder is a convenient way to concatenate strings
  class PlasmaShared StringBuilderExtended : public StringBuilder
  {
  public:
    LightningDeclareType(StringBuilderExtended, TypeCopyMode::ReferenceType);

    // Write to the string builder
    void Write(AnyParam value);
    void Write(StringParam value);
    void Write(StringRange value);
    void Write(char value);
    void Write(Plasma::Rune value);
    void Write(Rune value);
    void Write(cstr value);
    void Write(Boolean value);
    void Write(Boolean2Param value);
    void Write(Boolean3Param value);
    void Write(Boolean4Param value);
    void Write(Integer value);
    void Write(Integer2Param value);
    void Write(Integer3Param value);
    void Write(Integer4Param value);
    void Write(Real value);
    void Write(Real2Param value);
    void Write(Real3Param value);
    void Write(Real4Param value);
    void Write(unsigned long long value);
    void Write(DoubleInteger value);
    void Write(DoubleReal value);
    void Write(QuaternionParam value);
    void WriteLine();
    void WriteLine(AnyParam value);
    void WriteLine(StringParam value);
    void WriteLine(StringRange value);
    void WriteLine(cstr value);
    void WriteLine(char value);
    void WriteLine(Plasma::Rune value);
    void WriteLine(Rune value);
    void WriteLine(Boolean value);
    void WriteLine(Boolean2Param value);
    void WriteLine(Boolean3Param value);
    void WriteLine(Boolean4Param value);
    void WriteLine(Integer value);
    void WriteLine(Integer2Param value);
    void WriteLine(Integer3Param value);
    void WriteLine(Integer4Param value);
    void WriteLine(Real value);
    void WriteLine(Real2Param value);
    void WriteLine(Real3Param value);
    void WriteLine(Real4Param value);
    void WriteLine(unsigned long long value);
    void WriteLine(DoubleInteger value);
    void WriteLine(DoubleReal value);
    void WriteLine(QuaternionParam value);

    // Clears the current buffer and frees all blocks
    void Clear();

    // Convert the internally stored buffers into a string
    String ToString() const;
  };

  // An iterator to one rune (think char) in a string
  // This iterator also keeps a reference to the string to keep it alive
  class PlasmaShared RuneIterator
  {
  public:
    LightningDeclareType(RuneIterator, TypeCopyMode::ReferenceType);
    
    static String ToString(const BoundType* type, const ::byte* data);

    // Range interface
    static void All(Call& call, ExceptionReport& report);
    static void Current(Call& call, ExceptionReport& report);
    static void IsNotEmpty(Call& call, ExceptionReport& report);

    // Iterator interface
    static void Increment(Call& call, ExceptionReport& report);
    static void Decrement(Call& call, ExceptionReport& report);
    static void Equals(Call& call, ExceptionReport& report);

    static void GetByteIndex(Call& call, ExceptionReport& report);
    static void GetOriginalString(Call& call, ExceptionReport& report);
    static void FindRuneIndexFromIterator(Call& call, ExceptionReport& report);

    // Checks if the iterators are both referencing the same string
    static bool ValidateIteratorPair(RuneIterator& rhs, RuneIterator& lhs);
    // Checks to makes sure the start and end iterators are in a valid arrangement
    static bool ValidateIteratorOrder(RuneIterator& start, RuneIterator& end);

    // The original string this range was made from. Also keeps this string alive.
    StringRange mRange;
  };

  // A string range bound to Lightning. This range will also keep the string that it's referencing alive
  class PlasmaShared StringRangeExtended
  {
    LightningDeclareType(StringRangeExtended, TypeCopyMode::ReferenceType);

    static String ToString(const BoundType* type, const ::byte* data);

    // Range interface
    static void All(Call& call, ExceptionReport& report);
    void MoveNext();
    static void Current(Call& call, ExceptionReport& report);
    static void Empty(Call& call, ExceptionReport& report);
    static void IsNotEmpty(Call& call, ExceptionReport& report);

    static void Begin(Call& call, ExceptionReport& report);
    int CompareTo(StringRangeExtended range);
    bool Contains(StringRangeExtended string);
    static void ConvertToString(Call& call, ExceptionReport& report);
    static void End(Call& call, ExceptionReport& report);
    bool EndsWith(StringRangeExtended subString);
    static void FindFirstOf(Call& call, ExceptionReport& report);
    static void FindLastOf(Call& call, ExceptionReport& report);
    static void FindRangeExclusive(Call& call, ExceptionReport& report);
    static void FindRangeInclusive(Call& call, ExceptionReport& report);
    String Replace(StringRangeExtended oldValue, StringRangeExtended newValue);
    static void RuneIteratorFromByteIndex(Call& call, ExceptionReport& report);
    static void RuneIteratorFromByteIndexInternal(Call& call, ExceptionReport& report, StringParam strRef, StringRange range, int byteIndex);
    static void RuneIteratorFromRuneIndex(Call& call, ExceptionReport& report);
    static void RuneIteratorFromRuneIndexInternal(Call& call, ExceptionReport& report, StringParam strRef, StringRange range, int runeIndex);
    static void Split(Call& call, ExceptionReport& report);
    bool StartsWith(StringRangeExtended subString);
    static void SubString(Call& call, ExceptionReport& report);
    static void SubStringFromRuneIndices(Call& call, ExceptionReport& report);
    static void SubStringBytes(Call& call, ExceptionReport& report);
    static void Trim(Call& call, ExceptionReport& report);
    static void TrimEnd(Call& call, ExceptionReport& report);
    static void TrimStart(Call& call, ExceptionReport& report);
    String ToLower();
    String ToUpper();
    static void SetResultStringRange(Call& call, ExceptionReport& report, const String& strRef, StringRange result);
    static void SetResultStringSplitRange(Call& call, ExceptionReport& report, StringParam strRef, const Plasma::StringSplitRange& result);
    static void SetResultIterator(Call& call, ExceptionReport& report, StringRange result);
    static void GetOriginalString(Call& call, ExceptionReport& report);

    static bool ValidateRange(StringParam strRef, const StringRange& range);

    StringRange mRange;
    // The original string this range was made from. Also keeps this string alive
    String mOriginalStringReference;
  };

  // The results from a String.Split operation
  class PlasmaShared StringSplitRangeExtended
  {
    LightningDeclareType(StringSplitRangeExtended, TypeCopyMode::ReferenceType);

    static void All(Call& call, ExceptionReport& report);
    static void MoveNext(Call& call, ExceptionReport& report);
    static void Current(Call& call, ExceptionReport& report);
    static void IsNotEmpty(Call& call, ExceptionReport& report);

    // The original string this range was made from. Also keeps this string alive
    String mOriginalStringReference;
    Plasma::StringSplitRange mSplitRange;
  };
}

#endif
