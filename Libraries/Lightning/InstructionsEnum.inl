// MIT Licensed (see LICENSE.md).

// Note: These macros mirror those inside of Shared and VirtualMachine (for
// generation of instructions)

// Copy
#define LightningCopyInstructions(Type) LightningEnumValue(Copy##Type)

// Equality and inequality
#define LightningEqualityInstructions(Type) LightningEnumValue(TestInequality##Type) LightningEnumValue(TestEquality##Type)

// Less and greater comparison
#define LightningComparisonInstructions(Type)                                                                              \
  LightningEnumValue(TestLessThan##Type) LightningEnumValue(TestLessThanOrEqualTo##Type) LightningEnumValue(TestGreaterThan##Type) \
      LightningEnumValue(TestGreaterThanOrEqualTo##Type)

// Generic numeric operators, copy, equality
#define LightningNumericInstructions(Type)                                                                                 \
  LightningCopyInstructions(Type) LightningEqualityInstructions(Type) /* No instruction for unary plus */                      \
      LightningEnumValue(Negate##Type) LightningEnumValue(Increment##Type) LightningEnumValue(Decrement##Type)                     \
          LightningEnumValue(Add##Type) LightningEnumValue(Subtract##Type) LightningEnumValue(Multiply##Type)                      \
              LightningEnumValue(Divide##Type) LightningEnumValue(Modulo##Type) LightningEnumValue(Pow##Type)                      \
                  LightningEnumValue(AssignmentAdd##Type) LightningEnumValue(AssignmentSubtract##Type)                         \
                      LightningEnumValue(AssignmentMultiply##Type) LightningEnumValue(AssignmentDivide##Type)                  \
                          LightningEnumValue(AssignmentModulo##Type) LightningEnumValue(AssignmentPow##Type)

// Generic numeric operators, copy, equality, comparison
#define LightningScalarInstructions(Type) LightningNumericInstructions(Type) LightningComparisonInstructions(Type)

// Vector operations, generic numeric operators, copy, equality
#define LightningVectorInstructions(Type)                                                                                  \
  LightningNumericInstructions(Type) LightningComparisonInstructions(Type) LightningEnumValue(ScalarMultiply##Type)                \
      LightningEnumValue(ScalarDivide##Type) LightningEnumValue(ScalarModulo##Type) LightningEnumValue(ScalarPow##Type)            \
          LightningEnumValue(AssignmentScalarMultiply##Type) LightningEnumValue(AssignmentScalarDivide##Type)                  \
              LightningEnumValue(AssignmentScalarModulo##Type) LightningEnumValue(AssignmentScalarPow##Type)

// Special integral operators, generic numeric operators, copy, equality, and
// comparison
#define LightningIntegralInstructions(Type)                                                                                \
  LightningEnumValue(BitwiseNot##Type) LightningEnumValue(BitshiftLeft##Type) LightningEnumValue(BitshiftRight##Type)              \
      LightningEnumValue(BitwiseOr##Type) LightningEnumValue(BitwiseXor##Type) LightningEnumValue(BitwiseAnd##Type)                \
          LightningEnumValue(AssignmentBitshiftLeft##Type) LightningEnumValue(AssignmentBitshiftRight##Type)                   \
              LightningEnumValue(AssignmentBitwiseOr##Type) LightningEnumValue(AssignmentBitwiseXor##Type)                     \
                  LightningEnumValue(AssignmentBitwiseAnd##Type)

// Core instructions
LightningEnumValue(InvalidInstruction)

    LightningEnumValue(InternalDebugBreakpoint) LightningEnumValue(ThrowException) LightningEnumValue(PropertyDelegate)

        LightningEnumValue(TypeId)

            LightningEnumValue(BeginTimeout) LightningEnumValue(EndTimeout)

                LightningEnumValue(BeginScope) LightningEnumValue(EndScope)

                    LightningEnumValue(ToHandle)

                        LightningEnumValue(BeginStringBuilder) LightningEnumValue(EndStringBuilder)
                            LightningEnumValue(AddToStringBuilder)

                                LightningEnumValue(CreateInstanceDelegate) LightningEnumValue(CreateStaticDelegate)

                                    LightningEnumValue(IfFalseRelativeGoTo) LightningEnumValue(IfTrueRelativeGoTo)
                                        LightningEnumValue(RelativeGoTo)

                                            LightningEnumValue(Return) LightningEnumValue(PrepForFunctionCall)
                                                LightningEnumValue(FunctionCall)

                                                    LightningEnumValue(NewObject) LightningEnumValue(LocalObject)
                                                        LightningEnumValue(DeleteObject)

    // Primitive type instructions
    LightningIntegralInstructions(Byte) LightningScalarInstructions(Byte) LightningIntegralInstructions(
        Integer) LightningScalarInstructions(Integer) LightningVectorInstructions(Integer2) LightningVectorInstructions(Integer3)
        LightningVectorInstructions(Integer4) LightningIntegralInstructions(Integer2) LightningIntegralInstructions(
            Integer3) LightningIntegralInstructions(Integer4) LightningScalarInstructions(Real) LightningVectorInstructions(Real2)
            LightningVectorInstructions(Real3) LightningVectorInstructions(Real4) LightningScalarInstructions(
                DoubleReal) LightningIntegralInstructions(DoubleInteger) LightningScalarInstructions(DoubleInteger)

                LightningEqualityInstructions(Boolean) LightningEqualityInstructions(Handle) LightningEqualityInstructions(
                    Delegate) LightningEqualityInstructions(Any) LightningEqualityInstructions(Value)

                    LightningCopyInstructions(Boolean) LightningCopyInstructions(Any) LightningCopyInstructions(
                        Handle) LightningCopyInstructions(Delegate) LightningCopyInstructions(Value)

                        LightningEnumValue(LogicalNotBoolean)

                            LightningEnumValue(ConvertByteToReal) LightningEnumValue(ConvertByteToBoolean) LightningEnumValue(
                                ConvertByteToInteger) LightningEnumValue(ConvertByteToDoubleInteger) LightningEnumValue(ConvertByteToDoubleReal)
                                LightningEnumValue(ConvertIntegerToReal) LightningEnumValue(ConvertIntegerToBoolean) LightningEnumValue(
                                    ConvertIntegerToByte) LightningEnumValue(ConvertIntegerToDoubleInteger)
                                    LightningEnumValue(ConvertIntegerToDoubleReal) LightningEnumValue(ConvertRealToInteger) LightningEnumValue(
                                        ConvertRealToBoolean) LightningEnumValue(ConvertRealToByte) LightningEnumValue(ConvertRealToDoubleInteger)
                                        LightningEnumValue(ConvertRealToDoubleReal) LightningEnumValue(ConvertBooleanToInteger) LightningEnumValue(
                                            ConvertBooleanToReal) LightningEnumValue(ConvertBooleanToByte)
                                            LightningEnumValue(ConvertBooleanToDoubleInteger) LightningEnumValue(
                                                ConvertBooleanToDoubleReal) LightningEnumValue(ConvertDoubleIntegerToReal)
                                                LightningEnumValue(ConvertDoubleIntegerToBoolean) LightningEnumValue(
                                                    ConvertDoubleIntegerToByte) LightningEnumValue(ConvertDoubleIntegerToInteger)
                                                    LightningEnumValue(ConvertDoubleIntegerToDoubleReal) LightningEnumValue(
                                                        ConvertDoubleRealToReal) LightningEnumValue(ConvertDoubleRealToBoolean)
                                                        LightningEnumValue(ConvertDoubleRealToByte) LightningEnumValue(
                                                            ConvertDoubleRealToInteger) LightningEnumValue(ConvertDoubleRealToDoubleInteger)

                                                            LightningEnumValue(ConvertInteger2ToReal2) LightningEnumValue(
                                                                ConvertInteger2ToBoolean2) LightningEnumValue(ConvertReal2ToInteger2)
                                                                LightningEnumValue(ConvertReal2ToBoolean2) LightningEnumValue(
                                                                    ConvertBoolean2ToInteger2) LightningEnumValue(ConvertBoolean2ToReal2)

                                                                    LightningEnumValue(ConvertInteger3ToReal3) LightningEnumValue(
                                                                        ConvertInteger3ToBoolean3) LightningEnumValue(ConvertReal3ToInteger3)
                                                                        LightningEnumValue(ConvertReal3ToBoolean3) LightningEnumValue(
                                                                            ConvertBoolean3ToInteger3) LightningEnumValue(ConvertBoolean3ToReal3)

                                                                            LightningEnumValue(ConvertInteger4ToReal4) LightningEnumValue(
                                                                                ConvertInteger4ToBoolean4)
                                                                                LightningEnumValue(ConvertReal4ToInteger4) LightningEnumValue(
                                                                                    ConvertReal4ToBoolean4)
                                                                                    LightningEnumValue(
                                                                                        ConvertBoolean4ToInteger4)
                                                                                        LightningEnumValue(
                                                                                            ConvertBoolean4ToReal4)

                                                                                            LightningEnumValue(
                                                                                                ConvertStringToStringRangeExtended)

                                                                                                LightningEnumValue(
                                                                                                    ConvertDowncast)
                                                                                                    LightningEnumValue(
                                                                                                        ConvertToAny)
                                                                                                        LightningEnumValue(
                                                                                                            ConvertFromAny)
                                                                                                            LightningEnumValue(
                                                                                                                AnyDynamicMemberGet)
                                                                                                                LightningEnumValue(
                                                                                                                    AnyDynamicMemberSet)
