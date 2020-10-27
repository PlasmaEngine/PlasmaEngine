// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

// Bind some extra types to lightning for simulating shaders
namespace Lightning
{

using namespace Plasma;

template <OpType opType>
Lightning::Function* AddFunction(Lightning::LibraryBuilder& builder,
                             Lightning::BoundType* owner,
                             Lightning::BoundType* returnType,
                             StringParam fnName,
                             ParameterArray& params)
{
  Lightning::Function* lightningFn = builder.AddBoundFunction(
      owner, fnName, UnTranslatedBoundFunction, params, returnType, Lightning::FunctionOptions::Static);
  lightningFn->UserData = (void*)&ResolveSimpleFunction<opType>;
  return lightningFn;
}

template <OpType opType>
Lightning::Function*
AddFunction(Lightning::LibraryBuilder& builder, Lightning::BoundType* owner, Lightning::BoundType* returnType, StringParam fnName)
{
  ParameterArray params;
  return AddFunction<opType>(builder, owner, returnType, fnName, params);
}

template <OpType opType>
Lightning::Function* AddFunction(Lightning::LibraryBuilder& builder,
                             Lightning::BoundType* owner,
                             Lightning::BoundType* returnType,
                             StringParam fnName,
                             Lightning::BoundType* param0Type,
                             StringParam param0Name)
{
  DelegateParameter param0(param0Type);
  param0.Name = param0Name;
  ParameterArray params;
  params.PushBack(param0);
  return AddFunction<opType>(builder, owner, returnType, fnName, params);
}

template <OpType opType>
Lightning::Function* AddFunction(Lightning::LibraryBuilder& builder,
                             Lightning::BoundType* owner,
                             Lightning::BoundType* returnType,
                             StringParam fnName,
                             Lightning::BoundType* param0Type)
{
  return AddFunction<opType>(builder, owner, returnType, fnName, param0Type, String());
}

template <OpType opType>
Lightning::Function* AddFunction(Lightning::LibraryBuilder& builder,
                             Lightning::BoundType* owner,
                             Lightning::BoundType* returnType,
                             StringParam fnName,
                             Lightning::BoundType* param0Type,
                             StringParam param0Name,
                             Lightning::BoundType* param1Type,
                             StringParam param1Name)
{
  DelegateParameter param0(param0Type);
  param0.Name = param0Name;
  DelegateParameter param1(param1Type);
  param1.Name = param1Name;

  ParameterArray params;
  params.PushBack(param0);
  params.PushBack(param1);
  return AddFunction<opType>(builder, owner, returnType, fnName, params);
}

template <OpType opType>
Lightning::Function* AddFunction(Lightning::LibraryBuilder& builder,
                             Lightning::BoundType* owner,
                             Lightning::BoundType* returnType,
                             StringParam fnName,
                             Lightning::BoundType* param0Type,
                             Lightning::BoundType* param1Type)
{
  return AddFunction<opType>(builder, owner, returnType, fnName, param0Type, String(), param1Type, String());
}

template <OpType opType>
Lightning::Function* AddFunction(Lightning::LibraryBuilder& builder,
                             Lightning::BoundType* owner,
                             Lightning::BoundType* returnType,
                             StringParam fnName,
                             Lightning::BoundType* param0Type,
                             StringParam param0Name,
                             Lightning::BoundType* param1Type,
                             StringParam param1Name,
                             Lightning::BoundType* param2Type,
                             StringParam param2Name)
{
  DelegateParameter param0(param0Type);
  param0.Name = param0Name;
  DelegateParameter param1(param1Type);
  param1.Name = param1Name;
  DelegateParameter param2(param2Type);
  param2.Name = param2Name;

  ParameterArray params;
  params.PushBack(param0);
  params.PushBack(param1);
  params.PushBack(param2);
  return AddFunction<opType>(builder, owner, returnType, fnName, params);
}

template <OpType opType>
Lightning::Function* AddFunction(Lightning::LibraryBuilder& builder,
                             Lightning::BoundType* owner,
                             Lightning::BoundType* returnType,
                             StringParam fnName,
                             Lightning::BoundType* param0Type,
                             Lightning::BoundType* param1Type,
                             Lightning::BoundType* param2Type)
{
  return AddFunction<opType>(
      builder, owner, returnType, fnName, param0Type, String(), param1Type, String(), param2Type, String());
}

template <OpType opType>
Lightning::Function* AddFunction(Lightning::LibraryBuilder& builder,
                             Lightning::BoundType* owner,
                             Lightning::BoundType* returnType,
                             StringParam fnName,
                             Lightning::BoundType* param0Type,
                             StringParam param0Name,
                             Lightning::BoundType* param1Type,
                             StringParam param1Name,
                             Lightning::BoundType* param2Type,
                             StringParam param2Name,
                             Lightning::BoundType* param3Type,
                             StringParam param3Name)
{
  DelegateParameter param0(param0Type);
  param0.Name = param0Name;
  DelegateParameter param1(param1Type);
  param1.Name = param1Name;
  DelegateParameter param2(param2Type);
  param2.Name = param2Name;
  DelegateParameter param3(param3Type);
  param3.Name = param3Name;

  ParameterArray params;
  params.PushBack(param0);
  params.PushBack(param1);
  params.PushBack(param2);
  params.PushBack(param3);

  return AddFunction<opType>(builder, owner, returnType, fnName, params);
}

template <OpType opType>
Lightning::Function* AddFunction(Lightning::LibraryBuilder& builder,
                             Lightning::BoundType* owner,
                             Lightning::BoundType* returnType,
                             StringParam fnName,
                             Lightning::BoundType* param0Type,
                             Lightning::BoundType* param1Type,
                             Lightning::BoundType* param2Type,
                             Lightning::BoundType* param3Type)
{
  return AddFunction<opType>(builder,
                             owner,
                             returnType,
                             fnName,
                             param0Type,
                             String(),
                             param1Type,
                             String(),
                             param2Type,
                             String(),
                             param3Type,
                             String());
}

void AddMathOps(Lightning::LibraryBuilder& builder, Lightning::BoundType* type, LightningTypeGroups& types)
{
  Lightning::BoundType* voidType = LightningTypeId(void);
  Lightning::BoundType* boolType = types.mBooleanVectorTypes[0];
  Lightning::BoundType* intType = types.mIntegerVectorTypes[0];
  Lightning::BoundType* realType = types.mRealVectorTypes[0];

  for (size_t i = 0; i < types.mBooleanVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mBooleanVectorTypes[i];
    Lightning::BoundType* vectorBoolType = lightningType;

    // Unary
    AddFunction<OpType::OpLogicalNot>(builder, type, lightningType, "LogicalNot", lightningType);
    // Any/all only exists on vector types
    if (i != 0)
    {
      AddFunction<OpType::OpAny>(builder, type, boolType, "Any", lightningType);
      AddFunction<OpType::OpAll>(builder, type, boolType, "All", lightningType);
    }

    // Binary
    AddFunction<OpType::OpLogicalEqual>(builder, type, lightningType, "LogicalEqual", lightningType, lightningType);
    AddFunction<OpType::OpLogicalNotEqual>(builder, type, lightningType, "LogicalNotEqual", lightningType, lightningType);
    AddFunction<OpType::OpLogicalOr>(builder, type, lightningType, "LogicalOr", lightningType, lightningType);
    AddFunction<OpType::OpLogicalAnd>(builder, type, lightningType, "LogicalAnd", lightningType, lightningType);

    AddFunction<OpType::OpSelect>(
        builder, type, lightningType, "Select", vectorBoolType, "condition", lightningType, "obj1", lightningType, "obj2");
  }

  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mIntegerVectorTypes[i];
    Lightning::BoundType* vectorBoolType = types.mBooleanVectorTypes[i];

    // Unary
    AddFunction<OpType::OpBitReverse>(builder, type, lightningType, "BitReverse", lightningType);
    AddFunction<OpType::OpBitCount>(builder, type, lightningType, "BitCount", lightningType);
    AddFunction<OpType::OpNot>(builder, type, lightningType, "Not", lightningType);

    // Binary
    AddFunction<OpType::OpShiftRightLogical>(
        builder, type, lightningType, "ShiftRightLogical", lightningType, "base", lightningType, "shift");
    AddFunction<OpType::OpShiftRightArithmetic>(
        builder, type, lightningType, "ShiftRightArithmetic", lightningType, "base", lightningType, "shift");
    AddFunction<OpType::OpShiftLeftLogical>(
        builder, type, lightningType, "ShiftLeftLogical", lightningType, "base", lightningType, "shift");
    AddFunction<OpType::OpBitwiseOr>(builder, type, lightningType, "BitwiseOr", lightningType, lightningType);
    AddFunction<OpType::OpBitwiseXor>(builder, type, lightningType, "BitwiseXor", lightningType, lightningType);
    AddFunction<OpType::OpBitwiseAnd>(builder, type, lightningType, "BitwiseAnd", lightningType, lightningType);
    AddFunction<OpType::OpIEqual>(builder, type, vectorBoolType, "Equal", lightningType, lightningType);
    AddFunction<OpType::OpINotEqual>(builder, type, vectorBoolType, "NotEqual", lightningType, lightningType);
    AddFunction<OpType::OpSGreaterThan>(builder, type, vectorBoolType, "GreaterThan", lightningType, lightningType);
    AddFunction<OpType::OpSGreaterThanEqual>(builder, type, vectorBoolType, "GreaterThanEqual", lightningType, lightningType);
    AddFunction<OpType::OpSLessThan>(builder, type, vectorBoolType, "LessThan", lightningType, lightningType);
    AddFunction<OpType::OpSLessThanEqual>(builder, type, vectorBoolType, "LessThanEqual", lightningType, lightningType);

    AddFunction<OpType::OpSRem>(builder, type, lightningType, "Remainder", lightningType, lightningType);
    AddFunction<OpType::OpSMod>(builder, type, lightningType, "Mod", lightningType, lightningType);

    // Tri
    AddFunction<OpType::OpSelect>(
        builder, type, lightningType, "Select", vectorBoolType, "condition", lightningType, "obj1", lightningType, "obj2");
    AddFunction<OpType::OpBitFieldSExtract>(
        builder, type, lightningType, "BitFieldExtract", lightningType, "base", intType, "offset", intType, "count");

    // Quad
    AddFunction<OpType::OpBitFieldInsert>(builder,
                                          type,
                                          lightningType,
                                          "BitFieldInsert",
                                          lightningType,
                                          "base",
                                          lightningType,
                                          "insert",
                                          intType,
                                          "offset",
                                          intType,
                                          "count");
  }

  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mRealVectorTypes[i];
    Lightning::BoundType* vectorBoolType = types.mBooleanVectorTypes[i];

    // Unary
    AddFunction<OpType::OpDPdx>(builder, type, lightningType, "Ddx", lightningType)
        ->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
    AddFunction<OpType::OpDPdy>(builder, type, lightningType, "Ddy", lightningType)
        ->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
    AddFunction<OpType::OpFwidth>(builder, type, lightningType, "FWidth", lightningType)
        ->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
    AddFunction<OpType::OpIsNan>(builder, type, vectorBoolType, "IsNan", lightningType);
    AddFunction<OpType::OpIsInf>(builder, type, vectorBoolType, "IsInf", lightningType);

    // Binary
    AddFunction<OpType::OpFRem>(builder, type, lightningType, "Remainder", lightningType, lightningType);
    AddFunction<OpType::OpFMod>(builder, type, lightningType, "Mod", lightningType, lightningType);

    AddFunction<OpType::OpFOrdEqual>(builder, type, vectorBoolType, "OrderedEqual", lightningType, lightningType);
    AddFunction<OpType::OpFOrdNotEqual>(builder, type, vectorBoolType, "OrderedNotEqual", lightningType, lightningType);
    AddFunction<OpType::OpFOrdLessThan>(builder, type, vectorBoolType, "OrderedLessThan", lightningType, lightningType);
    AddFunction<OpType::OpFOrdLessThanEqual>(
        builder, type, vectorBoolType, "OrderedLessThanEqual", lightningType, lightningType);
    AddFunction<OpType::OpFOrdGreaterThan>(builder, type, vectorBoolType, "OrderedGreaterThan", lightningType, lightningType);
    AddFunction<OpType::OpFOrdGreaterThanEqual>(
        builder, type, vectorBoolType, "OrderedGreaterThanEqual", lightningType, lightningType);

    // Any/all only exists on vector types
    if (i != 0)
    {
      AddFunction<OpType::OpDot>(builder, type, realType, "Dot", lightningType, lightningType);
      AddFunction<OpType::OpVectorTimesScalar>(builder, type, lightningType, "VectorTimesScalar", lightningType, realType);
    }

    // Not implemented in glsl
    AddFunction<OpType::OpFUnordEqual>(builder, type, vectorBoolType, "UnorderedEqual", lightningType, lightningType);
    AddFunction<OpType::OpFUnordNotEqual>(builder, type, vectorBoolType, "UnorderedNotEqual", lightningType, lightningType);
    AddFunction<OpType::OpFUnordLessThan>(builder, type, vectorBoolType, "UnorderedLessThan", lightningType, lightningType);
    AddFunction<OpType::OpFUnordLessThanEqual>(
        builder, type, vectorBoolType, "UnorderedLessThanEqual", lightningType, lightningType);
    AddFunction<OpType::OpFUnordGreaterThan>(
        builder, type, vectorBoolType, "UnorderedGreaterThan", lightningType, lightningType);
    AddFunction<OpType::OpFUnordGreaterThanEqual>(
        builder, type, vectorBoolType, "UnorderedGreaterThanEqual", lightningType, lightningType);

    // Trinary
    AddFunction<OpType::OpSelect>(
        builder, type, lightningType, "Select", vectorBoolType, "condition", lightningType, "obj1", lightningType, "obj2");
  }

  for (u32 y = 2; y <= 4; ++y)
  {
    for (u32 x = 2; x <= 4; ++x)
    {
      Lightning::BoundType* lightningType = types.GetMatrixType(y, x);

      // Unary
      AddFunction<OpType::OpTranspose>(builder, type, lightningType, "Transpose", lightningType);

      // Binary
      AddFunction<OpType::OpMatrixTimesScalar>(builder, type, lightningType, "MatrixTimesScalar", lightningType, realType);

      // Ignore for now (have to figure out row/column order stuff)
      // AddFunction<OpType::OpMatrixTimesMatrix>(builder, type, lightningType,
      // "MatrixTimesMatrix", lightningType, lightningType);
    }
  }

  // Conversion
  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningRealType = types.mRealVectorTypes[i];
    Lightning::BoundType* lightningIntType = types.mIntegerVectorTypes[i];
    Lightning::BoundType* lightningBoolType = types.mBooleanVectorTypes[i];

    AddFunction<OpType::OpConvertFToS>(builder, type, lightningIntType, "ConvertFToS", lightningRealType);
    AddFunction<OpType::OpConvertSToF>(builder, type, lightningRealType, "ConvertSToF", lightningIntType);

    AddFunction<OpType::OpBitcast>(builder, type, lightningRealType, "BitCastToReal", lightningIntType);
    AddFunction<OpType::OpBitcast>(builder, type, lightningIntType, "BitCastToInteger", lightningRealType);
  }
}

LightningDefineType(ShaderIntrinsics, builder, type)
{
  Lightning::BoundType* voidType = LightningTypeId(void);
  Lightning::BoundType* boolType = LightningTypeId(bool);
  Lightning::BoundType* intType = LightningTypeId(int);

  LightningShaderIRCore& shaderCore = LightningShaderIRCore::GetInstance();
  LightningTypeGroups& types = shaderCore.mLightningTypes;

  // This technically needs to be restricted to pixel fragment types.
  AddFunction<OpType::OpKill>(builder, type, voidType, "Kill")
      ->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);

  AddMathOps(builder, type, types);
  AddGlslExtensionIntrinsicOps(builder, shaderCore.mGlsl450ExtensionsLibrary, type, types);
  AddImageFunctions(builder, type, types);

  Lightning::ParameterArray parameters = OneParameter(intType, "language");
  Lightning::Function* isLanguageFn = builder.AddBoundFunction(
      type, "IsLanguage", Plasma::DummyBoundFunction, parameters, boolType, Lightning::FunctionOptions::Static);
  isLanguageFn->UserData = (void*)&ResolveIsLanguage;

  parameters = ThreeParameters(intType, "language", intType, "minVersion", intType, "maxVersion");
  isLanguageFn = builder.AddBoundFunction(
      type, "IsLanguage", Plasma::DummyBoundFunction, parameters, boolType, Lightning::FunctionOptions::Static);
  isLanguageFn->UserData = (void*)&ResolveIsLanguageMinMaxVersion;
}

LightningDefineType(GeometryStreamUserData, builder, type)
{
  LightningBindDefaultCopyDestructor();
}

void GeometryStreamUserData::Set(spv::ExecutionMode executionMode)
{
  mExecutionMode = executionMode;
  // Use the execution mode to compute the size of this stream and the
  // input/output mode
  if (executionMode == spv::ExecutionModeInputPoints)
  {
    mInput = true;
    mSize = 1;
  }
  else if (executionMode == spv::ExecutionModeInputLines)
  {
    mInput = true;
    mSize = 2;
  }
  else if (executionMode == spv::ExecutionModeTriangles)
  {
    mInput = true;
    mSize = 3;
  }
  else if (executionMode == spv::ExecutionModeOutputPoints)
  {
    mInput = false;
    mSize = 1;
  }
  else if (executionMode == spv::ExecutionModeOutputLineStrip)
  {
    mInput = false;
    mSize = 2;
  }
  else if (executionMode == spv::ExecutionModeOutputTriangleStrip)
  {
    mInput = false;
    mSize = 3;
  }
}

LightningDefineType(GeometryFragmentUserData, builder, type)
{
  LightningBindDefaultCopyDestructor();
}

GeometryFragmentUserData::GeometryFragmentUserData()
{
  mInputStreamType = nullptr;
  mOutputStreamType = nullptr;
}

LightningShaderIRType* GeometryFragmentUserData::GetInputVertexType()
{
  ILightningShaderIR* param = mInputStreamType->mParameters[0];
  LightningShaderIRType* subType = param->As<LightningShaderIRType>();
  return subType;
}

LightningShaderIRType* GeometryFragmentUserData::GetOutputVertexType()
{
  ILightningShaderIR* param = mOutputStreamType->mParameters[0];
  LightningShaderIRType* subType = param->As<LightningShaderIRType>();
  return subType;
}

LightningDefineType(ComputeFragmentUserData, builder, type)
{
  LightningBindDefaultCopyDestructor();
}

ComputeFragmentUserData::ComputeFragmentUserData()
{
  mLocalSizeX = 1;
  mLocalSizeY = 1;
  mLocalSizeZ = 1;
}

LightningDefineType(UnsignedInt, builder, type)
{
  LightningBindDefaultCopyDestructor();
}

} // namespace Lightning
