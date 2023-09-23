// MIT Licensed (see LICENSE.md).

#pragma once
#ifndef LIGHTNING_VIRTUAL_MACHINE_HPP
#  define LIGHTNING_VIRTUAL_MACHINE_HPP

namespace Lightning
{
// This is just a special identifier that means we jumped, there's really no
// reason to the number... ;)
static const int ExceptionJumpResult = 1729;

// This class is responsible for executing a stream of opcodes
class PlasmaShared VirtualMachine
{
public:
  // Fills out a table of function pointers with all the instruction functions
  // we use
  static void InitializeJumpTable();

  // Execute a function, starting from a given stack frame
  static void ExecuteNext(Call& call, ExceptionReport& report);

  // Return the value of an enum property (the user data Contains the value)
  static void EnumerationProperty(Call& call, ExceptionReport& report);

  // Return an events name string (the user data Contains the value)
  static void EventsProperty(Call& call, ExceptionReport& report);

  // The native constructor invokes SetNativeTypeFullyConstructed on the Handle
  // manager before calling the actual constructor code
  static void NativeConstructor(Call& call, ExceptionReport& report);

  // A special function that always returns a default value (plasma, null, etc)
  // When we patch a library with a newer version, but the old version had
  // functions that the newer one does not they will be patched with this bound
  // function (so that they do absolutely nothing)
  static void PatchDummy(Call& call, ExceptionReport& report);

  // Executes a destructor not actually from opcode, but
  // rather from a separate handle list on the ClassType
  static void PostDestructor(BoundType* boundType, ::byte* objectData);

  // When an enum or flags fail and we just print the value, then we also end up
  // printing the type too
  static String UnknownEnumerationToString(const BoundType* type, const ::byte* data);

  // Conversion from an enumeration into a string (prints out the value of the
  // enum, or an integer if it fails)
  static String EnumerationToString(const BoundType* type, const ::byte* data);

  // Conversion from a flags into a string (prints out all enabled flags, or an
  // integer if it fails)
  static String FlagsToString(const BoundType* type, const ::byte* data);

  // Generic integral power (expands to other integral sizes)
  template <typename T>
  static inline T IntegralPower(T base, T exponent)
  {
    T result = exponent >= 0;
    while (exponent && result)
    {
      if (exponent & 1)
      {
        result *= base;
      }

      exponent >>= 1;
      base *= base;
    }

    return result;
  }

  // A generic wrapper around 'raise to a power'
  // Note that in cases of compound assignment, the value can be the out!
  template <typename T>
  static inline void GenericPow(T& out, const T& base, const T& exponent)
  {
    out = std::pow(base, exponent);
  }

  // A generic wrapper around 'modulus / remainder'
  // Note that in cases of compound assignment, the value can be the out!
  template <typename T>
  static inline void GenericMod(T& out, const T& value, const T& mod)
  {
    out = value % mod;
  }

  // A generic wrapper around 'increment'
  // Note that in cases of compound assignment, the value can be the out!
  template <typename T>
  static inline void GenericIncrement(T& value)
  {
    ++value;
  }

  // A generic wrapper around 'decrement'
  // Note that in cases of compound assignment, the value can be the out!
  template <typename T>
  static inline void GenericDecrement(T& value)
  {
    --value;
  }

  // Checks if a value is plasma or Contains any plasmas (in the case of vectors)
  template <typename T>
  static inline bool GenericIsZero(const T& value)
  {
    return value == 0;
  }

  template <typename T>
  static inline void GenericIsZeroThrow(PerFrameData* ourFrame, const T& value, const char* name)
  {
    if (GenericIsZero(value))
    {
      ExecutableState::GetCallingState()->ThrowException(String::Format("Attempted to %s by plasma", name));
      longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
    }
  }

  // A generic wrapper around 'vector raise to a scalar power'
  // Note that in cases of compound assignment, the value can be the out!
  template <typename VectorType, typename ScalarType>
  static inline void GenericScalarPow(VectorType& out, const VectorType& base, const ScalarType& exponent);

  // A generic wrapper around 'vector modulus / remainder by a scalar'
  // Note that in cases of compound assignment, the value can be the out!
  template <typename VectorType, typename ScalarType>
  static inline void GenericScalarMod(VectorType& out, const VectorType& value, const ScalarType& mod)
  {
    out = value % mod;
  }

// Define instruction functions for all of our opcodes
#  define LightningEnumValue(Name)                                                                                         \
    static void Instruction##Name(ExecutableState* state,                                                              \
                                  Call& call,                                                                          \
                                  ExceptionReport& report,                                                             \
                                  size_t& programCounter,                                                              \
                                  PerFrameData* ourFrame,                                                              \
                                  const Opcode& opcode);
#  include "InstructionsEnum.inl"
#  undef LightningEnumValue
};

// Note: These HAVE to be declared in namespace scope according to the C++ spec
// (cannot be put inside the class) Specializations for Pow
template <>
PlasmaShared inline void VirtualMachine::GenericPow<Byte>(Byte& out, const Byte& base, const Byte& exponent);
template <>
PlasmaShared inline void VirtualMachine::GenericPow<Integer>(Integer& out, const Integer& base, const Integer& exponent);
template <>
PlasmaShared inline void VirtualMachine::GenericPow<Integer2>(Integer2& out,
                                                            const Integer2& base,
                                                            const Integer2& exponent);
template <>
PlasmaShared inline void VirtualMachine::GenericPow<Integer3>(Integer3& out,
                                                            const Integer3& base,
                                                            const Integer3& exponent);
template <>
PlasmaShared inline void VirtualMachine::GenericPow<Integer4>(Integer4& out,
                                                            const Integer4& base,
                                                            const Integer4& exponent);
template <>
PlasmaShared inline void VirtualMachine::GenericPow<Real2>(Real2& out, const Real2& base, const Real2& exponent);
template <>
PlasmaShared inline void VirtualMachine::GenericPow<Real3>(Real3& out, const Real3& base, const Real3& exponent);
template <>
PlasmaShared inline void VirtualMachine::GenericPow<Real4>(Real4& out, const Real4& base, const Real4& exponent);
template <>
PlasmaShared inline void VirtualMachine::GenericPow<DoubleInteger>(DoubleInteger& out,
                                                                 const DoubleInteger& base,
                                                                 const DoubleInteger& exponent);

// Specializations for Mod
template <>
PlasmaShared inline void VirtualMachine::GenericMod<Real>(Real& out, const Real& value, const Real& mod);
template <>
PlasmaShared inline void VirtualMachine::GenericMod<Real2>(Real2& out, const Real2& value, const Real2& mod);
template <>
PlasmaShared inline void VirtualMachine::GenericMod<Real3>(Real3& out, const Real3& value, const Real3& mod);
template <>
PlasmaShared inline void VirtualMachine::GenericMod<Real4>(Real4& out, const Real4& value, const Real4& mod);
template <>
PlasmaShared inline void VirtualMachine::GenericMod<DoubleReal>(DoubleReal& out,
                                                              const DoubleReal& value,
                                                              const DoubleReal& mod);

// Specializations for Scalar Pow
template <>
PlasmaShared inline void VirtualMachine::GenericScalarPow<Integer2, Integer>(Integer2& out,
                                                                           const Integer2& base,
                                                                           const Integer& exponent);
template <>
PlasmaShared inline void VirtualMachine::GenericScalarPow<Integer3, Integer>(Integer3& out,
                                                                           const Integer3& base,
                                                                           const Integer& exponent);
template <>
PlasmaShared inline void VirtualMachine::GenericScalarPow<Integer4, Integer>(Integer4& out,
                                                                           const Integer4& base,
                                                                           const Integer& exponent);
template <>
PlasmaShared inline void VirtualMachine::GenericScalarPow<Real2, Real>(Real2& out,
                                                                     const Real2& base,
                                                                     const Real& exponent);
template <>
PlasmaShared inline void VirtualMachine::GenericScalarPow<Real3, Real>(Real3& out,
                                                                     const Real3& base,
                                                                     const Real& exponent);
template <>
PlasmaShared inline void VirtualMachine::GenericScalarPow<Real4, Real>(Real4& out,
                                                                     const Real4& base,
                                                                     const Real& exponent);

// Specializations for Scalar Mod
template <>
PlasmaShared inline void VirtualMachine::GenericScalarMod<Real2, Real>(Real2& out, const Real2& value, const Real& mod);
template <>
PlasmaShared inline void VirtualMachine::GenericScalarMod<Real3, Real>(Real3& out, const Real3& value, const Real& mod);
template <>
PlasmaShared inline void VirtualMachine::GenericScalarMod<Real4, Real>(Real4& out, const Real4& value, const Real& mod);

// Specializations for Scalar Increment
template <>
PlasmaShared inline void VirtualMachine::GenericIncrement<Real2>(Real2& out);
template <>
PlasmaShared inline void VirtualMachine::GenericIncrement<Real3>(Real3& out);
template <>
PlasmaShared inline void VirtualMachine::GenericIncrement<Real4>(Real4& out);

// Specializations for Scalar Decrement
template <>
PlasmaShared inline void VirtualMachine::GenericDecrement<Real2>(Real2& out);
template <>
PlasmaShared inline void VirtualMachine::GenericDecrement<Real3>(Real3& out);
template <>
PlasmaShared inline void VirtualMachine::GenericDecrement<Real4>(Real4& out);

// Specializations for IsZero
template <>
PlasmaShared inline bool VirtualMachine::GenericIsZero<Integer2>(const Integer2& value);
template <>
PlasmaShared inline bool VirtualMachine::GenericIsZero<Integer3>(const Integer3& value);
template <>
PlasmaShared inline bool VirtualMachine::GenericIsZero<Integer4>(const Integer4& value);
template <>
PlasmaShared inline bool VirtualMachine::GenericIsZero<Real2>(const Real2& value);
template <>
PlasmaShared inline bool VirtualMachine::GenericIsZero<Real3>(const Real3& value);
template <>
PlasmaShared inline bool VirtualMachine::GenericIsZero<Real4>(const Real4& value);
} // namespace Lightning

// Crash report capture variables
// This is an attempt to force crash reports to store a variable / indirectly
// referenced memory (not thread safe either) Do NOT ever attempt to access this
// variable or do anything with it in code
PlasmaShared extern ::byte* LightningLastRunningOpcode;
PlasmaShared extern Lightning::Function* LightningLastRunningFunction;
PlasmaShared extern size_t LightningLastRunningOpcodeLength;

#endif
