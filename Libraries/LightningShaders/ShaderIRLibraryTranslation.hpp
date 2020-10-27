// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

// Registered lightning function that doesn't have an actual implementation. No
// function that uses this should be invoked.
void UnTranslatedBoundFunction(Lightning::Call& call, Lightning::ExceptionReport& report);
// Registered lightning function that doesn't have an implementation but shouldn't
// throw an error. Currently needed for things like FixedArray that can get
// their constructor called via pre-initialization when inspecting other
// properties.
void DummyBoundFunction(Lightning::Call& call, Lightning::ExceptionReport& report);


/// Helper struct to pass around groups of types for generating library
/// translation
struct LightningTypeGroups
{
	Lightning::BoundType* mVoidType;

	// Index 0 is Real1 (e.g. Real)
	Array<Lightning::BoundType*> mRealVectorTypes;
	Array<Lightning::BoundType*> mRealMatrixTypes;

    Array<Lightning::BoundType*> mIntegerVectorTypes;
	Array<Lightning::BoundType*> mBooleanVectorTypes;

	Lightning::BoundType* mQuaternionType;
	// SpirV does not support non-floating point matrix types
	// Array<Lightning::BoundType*> mIntegerMatrixTypes;

	Lightning::BoundType* GetMatrixType(u32 y, u32 x)
	{
	y -= 2;
	x -= 2;
	return mRealMatrixTypes[x + y * 3];
	}
};

/// Helper struct to pass around groups of types for generating library
/// translation
struct ShaderTypeGroups
{
	LightningShaderIRType* mVoidType;

	// Index 0 is Real1 (e.g. Real)
	Array<LightningShaderIRType*> mRealVectorTypes;
	Array<LightningShaderIRType*> mRealMatrixTypes;

	Array<LightningShaderIRType*> mIntegerVectorTypes;
	Array<LightningShaderIRType*> mBooleanVectorTypes;
	LightningShaderIRType* mQuaternionType;
	// SpirV does not support non-floating point matrix types
	// Array<LightningShaderIRType*> mIntegerMatrixTypes;

	LightningShaderIRType* GetMatrixType(int y, int x)
	{
	y -= 2;
	x -= 2;
	return mRealMatrixTypes[x + y * 3];
	}
};

void ResolveSimpleFunctionFromOpType(LightningSpirVFrontEnd* translator,
                                     Lightning::FunctionCallNode* functionCallNode,
                                     Lightning::MemberAccessNode* memberAccessNode,
                                     OpType opType,
                                     LightningSpirVFrontEndContext* context);

// A simple helper to resolve a function (assumed to be value types) into
// calling a basic op function.
template <OpType opType>
inline void ResolveSimpleFunction(LightningSpirVFrontEnd* translator,
                                  Lightning::FunctionCallNode* functionCallNode,
                                  Lightning::MemberAccessNode* memberAccessNode,
                                  LightningSpirVFrontEndContext* context)
{
  return ResolveSimpleFunctionFromOpType(translator, functionCallNode, memberAccessNode, opType, context);
}

void ResolveVectorTypeCount(LightningSpirVFrontEnd* translator,
                            Lightning::FunctionCallNode* functionCallNode,
                            Lightning::MemberAccessNode* memberAccessNode,
                            LightningSpirVFrontEndContext* context);
void ResolvePrimitiveGet(LightningSpirVFrontEnd* translator,
                         Lightning::FunctionCallNode* functionCallNode,
                         Lightning::MemberAccessNode* memberAccessNode,
                         LightningSpirVFrontEndContext* context);
void ResolvePrimitiveSet(LightningSpirVFrontEnd* translator,
                         Lightning::FunctionCallNode* functionCallNode,
                         Lightning::MemberAccessNode* memberAccessNode,
                         LightningSpirVFrontEndContext* context);

void ResolveVectorGet(LightningSpirVFrontEnd* translator,
                      Lightning::FunctionCallNode* functionCallNode,
                      Lightning::MemberAccessNode* memberAccessNode,
                      LightningSpirVFrontEndContext* context);
void ResolveVectorSet(LightningSpirVFrontEnd* translator,
                      Lightning::FunctionCallNode* functionCallNode,
                      Lightning::MemberAccessNode* memberAccessNode,
                      LightningSpirVFrontEndContext* context);
void ResolveMatrixGet(LightningSpirVFrontEnd* translator,
                      Lightning::FunctionCallNode* functionCallNode,
                      Lightning::MemberAccessNode* memberAccessNode,
                      LightningSpirVFrontEndContext* context);
void ResolveMatrixSet(LightningSpirVFrontEnd* translator,
                      Lightning::FunctionCallNode* functionCallNode,
                      Lightning::MemberAccessNode* memberAccessNode,
                      LightningSpirVFrontEndContext* context);
void ResolveStaticBinaryFunctionOp(LightningSpirVFrontEnd* translator,
                                   Lightning::FunctionCallNode* functionCallNode,
                                   OpType opType,
                                   LightningSpirVFrontEndContext* context);
void TranslatePrimitiveDefaultConstructor(LightningSpirVFrontEnd* translator,
                                          Lightning::Type* lightningResultType,
                                          LightningSpirVFrontEndContext* context);
void TranslatePrimitiveDefaultConstructor(LightningSpirVFrontEnd* translator,
                                          Lightning::FunctionCallNode* fnCallNode,
                                          Lightning::StaticTypeNode* staticTypeNode,
                                          LightningSpirVFrontEndContext* context);
void TranslateBackupPrimitiveConstructor(LightningSpirVFrontEnd* translator,
                                         Lightning::FunctionCallNode* fnCallNode,
                                         Lightning::StaticTypeNode* staticTypeNode,
                                         LightningSpirVFrontEndContext* context);
void TranslateCompositeDefaultConstructor(LightningSpirVFrontEnd* translator,
                                          Lightning::Type* lightningResultType,
                                          LightningSpirVFrontEndContext* context);
void TranslateCompositeDefaultConstructor(LightningSpirVFrontEnd* translator,
                                          Lightning::FunctionCallNode* fnCallNode,
                                          Lightning::StaticTypeNode* staticTypeNode,
                                          LightningSpirVFrontEndContext* context);
void TranslateBackupCompositeConstructor(LightningSpirVFrontEnd* translator,
                                         Lightning::FunctionCallNode* fnCallNode,
                                         Lightning::StaticTypeNode* staticTypeNode,
                                         LightningSpirVFrontEndContext* context);
void TranslateMatrixDefaultConstructor(LightningSpirVFrontEnd* translator,
                                       Lightning::Type* lightningResultType,
                                       LightningSpirVFrontEndContext* context);
void TranslateMatrixDefaultConstructor(LightningSpirVFrontEnd* translator,
                                       Lightning::FunctionCallNode* fnCallNode,
                                       Lightning::StaticTypeNode* staticTypeNode,
                                       LightningSpirVFrontEndContext* context);
void TranslateMatrixFullConstructor(LightningSpirVFrontEnd* translator,
                                    Lightning::FunctionCallNode* fnCallNode,
                                    Lightning::StaticTypeNode* staticTypeNode,
                                    LightningSpirVFrontEndContext* context);
LightningShaderIROp* RecursivelyTranslateCompositeSplatConstructor(LightningSpirVFrontEnd* translator,
                                                               Lightning::FunctionCallNode* fnCallNode,
                                                               Lightning::StaticTypeNode* staticTypeNode,
                                                               LightningShaderIRType* type,
                                                               LightningShaderIROp* splatValueOp,
                                                               LightningSpirVFrontEndContext* context);
void TranslateCompositeSplatConstructor(LightningSpirVFrontEnd* translator,
                                        Lightning::FunctionCallNode* fnCallNode,
                                        Lightning::StaticTypeNode* staticTypeNode,
                                        LightningSpirVFrontEndContext* context);
bool IsVectorSwizzle(StringParam memberName);
void ResolveScalarComponentAccess(LightningSpirVFrontEnd* translator,
                                  Lightning::MemberAccessNode* memberAccessNode,
                                  byte componentName,
                                  LightningSpirVFrontEndContext* context);
void ResolveScalarSwizzle(LightningSpirVFrontEnd* translator,
                          Lightning::MemberAccessNode* memberAccessNode,
                          StringParam memberName,
                          LightningSpirVFrontEndContext* context);
void ScalarBackupFieldResolver(LightningSpirVFrontEnd* translator,
                               Lightning::MemberAccessNode* memberAccessNode,
                               LightningSpirVFrontEndContext* context);
void ResolveVectorComponentAccess(LightningSpirVFrontEnd* translator,
                                  LightningShaderIROp* selfInstance,
                                  LightningShaderIRType* componentType,
                                  byte componentName,
                                  LightningSpirVFrontEndContext* context);
void ResolveVectorComponentAccess(LightningSpirVFrontEnd* translator,
                                  Lightning::MemberAccessNode* memberAccessNode,
                                  byte componentName,
                                  LightningSpirVFrontEndContext* context);
void ResolveVectorSwizzle(LightningSpirVFrontEnd* translator,
                          ILightningShaderIR* selfInstance,
                          LightningShaderIRType* resultType,
                          StringParam memberName,
                          LightningSpirVFrontEndContext* context);
void ResolveVectorSwizzle(LightningSpirVFrontEnd* translator,
                          Lightning::MemberAccessNode* memberAccessNode,
                          StringParam memberName,
                          LightningSpirVFrontEndContext* context);
void VectorBackupFieldResolver(LightningSpirVFrontEnd* translator,
                               Lightning::MemberAccessNode* memberAccessNode,
                               LightningSpirVFrontEndContext* context);
void ResolverVectorSwizzleSetter(LightningSpirVFrontEnd* translator,
                                 Lightning::MemberAccessNode* memberAccessNode,
                                 LightningShaderIROp* resultValue,
                                 LightningSpirVFrontEndContext* context);
void VectorBackupPropertySetter(LightningSpirVFrontEnd* translator,
                                Lightning::MemberAccessNode* memberAccessNode,
                                LightningShaderIROp* resultValue,
                                LightningSpirVFrontEndContext* context);
bool MatrixElementAccessResolver(LightningSpirVFrontEnd* translator,
                                 Lightning::MemberAccessNode* memberAccessNode,
                                 LightningSpirVFrontEndContext* context,
                                 Lightning::MatrixUserData& matrixUserData);
void MatrixBackupFieldResolver(LightningSpirVFrontEnd* translator,
                               Lightning::MemberAccessNode* memberAccessNode,
                               LightningSpirVFrontEndContext* context);

// Quaternions
void TranslateQuaternionDefaultConstructor(LightningSpirVFrontEnd* translator,
                                           Lightning::Type* lightningResultType,
                                           LightningSpirVFrontEndContext* context);
void TranslateQuaternionDefaultConstructor(LightningSpirVFrontEnd* translator,
                                           Lightning::FunctionCallNode* fnCallNode,
                                           Lightning::StaticTypeNode* staticTypeNode,
                                           LightningSpirVFrontEndContext* context);
void QuaternionBackupFieldResolver(LightningSpirVFrontEnd* translator,
                                   Lightning::MemberAccessNode* memberAccessNode,
                                   LightningSpirVFrontEndContext* context);
void QuaternionBackupPropertySetter(LightningSpirVFrontEnd* translator,
                                    Lightning::MemberAccessNode* memberAccessNode,
                                    LightningShaderIROp* resultValue,
                                    LightningSpirVFrontEndContext* context);
void ResolveQuaternionTypeCount(LightningSpirVFrontEnd* translator,
                                Lightning::FunctionCallNode* functionCallNode,
                                Lightning::MemberAccessNode* memberAccessNode,
                                LightningSpirVFrontEndContext* context);
void ResolveQuaternionGet(LightningSpirVFrontEnd* translator,
                          Lightning::FunctionCallNode* functionCallNode,
                          Lightning::MemberAccessNode* memberAccessNode,
                          LightningSpirVFrontEndContext* context);
void ResolveQuaternionSet(LightningSpirVFrontEnd* translator,
                          Lightning::FunctionCallNode* functionCallNode,
                          Lightning::MemberAccessNode* memberAccessNode,
                          LightningSpirVFrontEndContext* context);
void TranslateQuaternionSplatConstructor(LightningSpirVFrontEnd* translator,
                                         Lightning::FunctionCallNode* fnCallNode,
                                         Lightning::StaticTypeNode* staticTypeNode,
                                         LightningSpirVFrontEndContext* context);
void TranslateBackupQuaternionConstructor(LightningSpirVFrontEnd* translator,
                                          Lightning::FunctionCallNode* fnCallNode,
                                          Lightning::StaticTypeNode* staticTypeNode,
                                          LightningSpirVFrontEndContext* context);

void ResolveBinaryOp(LightningSpirVFrontEnd* translator,
                     Lightning::BinaryOperatorNode* binaryOpNode,
                     OpType opType,
                     LightningSpirVFrontEndContext* context);
void ResolveBinaryOp(LightningSpirVFrontEnd* translator,
                     Lightning::BinaryOperatorNode* binaryOpNode,
                     OpType opType,
                     ILightningShaderIR* lhs,
                     ILightningShaderIR* rhs,
                     LightningSpirVFrontEndContext* context);

void ResolveUnaryOperator(LightningSpirVFrontEnd* translator,
                          Lightning::UnaryOperatorNode* unaryOpNode,
                          OpType opType,
                          LightningSpirVFrontEndContext* context);

template <OpType opType>
inline void ResolveUnaryOperator(LightningSpirVFrontEnd* translator,
                                 Lightning::UnaryOperatorNode* unaryOpNode,
                                 LightningSpirVFrontEndContext* context)
{
  ResolveUnaryOperator(translator, unaryOpNode, opType, context);
}

template <OpType opType>
inline void ResolveBinaryOperator(LightningSpirVFrontEnd* translator,
                                  Lightning::BinaryOperatorNode* binaryOpNode,
                                  LightningSpirVFrontEndContext* context)
{
  ResolveBinaryOp(translator, binaryOpNode, opType, context);
}

// Shader intrinsics to write backend specific code by checking the current
// language/version
void ResolveIsLanguage(LightningSpirVFrontEnd* translator,
                       Lightning::FunctionCallNode* functionCallNode,
                       Lightning::MemberAccessNode* memberAccessNode,
                       LightningSpirVFrontEndContext* context);
void ResolveIsLanguageMinMaxVersion(LightningSpirVFrontEnd* translator,
                                    Lightning::FunctionCallNode* functionCallNode,
                                    Lightning::MemberAccessNode* memberAccessNode,
                                    LightningSpirVFrontEndContext* context);

void ResolveStaticBinaryFunctionOp(LightningSpirVFrontEnd* translator,
                                   Lightning::FunctionCallNode* functionCallNode,
                                   OpType opType,
                                   LightningSpirVFrontEndContext* context);
void RegisterArithmeticOps(LightningSpirVFrontEnd* translator, LightningShaderIRLibrary* shaderLibrary, LightningTypeGroups& types);
void RegisterConversionOps(LightningSpirVFrontEnd* translator, LightningShaderIRLibrary* shaderLibrary, LightningTypeGroups& types);
void RegisterLogicalOps(LightningSpirVFrontEnd* translator, LightningShaderIRLibrary* shaderLibrary, LightningTypeGroups& types);
void RegisterBitOps(LightningSpirVFrontEnd* translator, LightningShaderIRLibrary* shaderLibrary, LightningTypeGroups& types);
void RegisterGlsl450Extensions(LightningShaderIRLibrary* shaderLibrary,
                               SpirVExtensionLibrary* extLibrary,
                               LightningTypeGroups& types);
void AddGlslExtensionIntrinsicOps(Lightning::LibraryBuilder& builder,
                                  SpirVExtensionLibrary* extLibrary,
                                  Lightning::BoundType* type,
                                  LightningTypeGroups& types);
void RegisterShaderIntrinsics(LightningSpirVFrontEnd* translator, LightningShaderIRLibrary* shaderLibrary);
void RegisterColorsOps(LightningSpirVFrontEnd* translator, LightningShaderIRLibrary* shaderLibrary, LightningTypeGroups& types);
void FixedArrayResolver(LightningSpirVFrontEnd* translator, Lightning::BoundType* lightningFixedArrayType);
void RuntimeArrayResolver(LightningSpirVFrontEnd* translator, Lightning::BoundType* lightningRuntimeArrayType);
void GeometryStreamInputResolver(LightningSpirVFrontEnd* translator, Lightning::BoundType* lightningFixedArrayType);
void GeometryStreamOutputResolver(LightningSpirVFrontEnd* translator, Lightning::BoundType* lightningFixedArrayType);

} // namespace Plasma
