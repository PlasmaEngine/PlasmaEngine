// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "GLSL.std.450.h"

namespace Plasma
{

// Helper data used to translate glsl intrinsic functions
struct ExtensionLibraryUserData
{
  ExtensionLibraryUserData()
  {
    mOpCode = 0;
    mExtensionLibrary = nullptr;
  }
  ExtensionLibraryUserData(int opCode, SpirVExtensionLibrary* extensionLibrary)
  {
    mOpCode = opCode;
    mExtensionLibrary = extensionLibrary;
  }
  // The op-code to call.
  int mOpCode;
  // The extension library required for this op-code
  SpirVExtensionLibrary* mExtensionLibrary;
};

LightningShaderIROp* MakeBasicExtensionFunction(LightningSpirVFrontEnd* translator,
                                            Lightning::FunctionCallNode* functionCallNode,
                                            int extensionOpId,
                                            LightningShaderExtensionImport* importLibraryIR,
                                            LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);
  LightningShaderIRConstantLiteral* instructionLiteral = translator->GetOrCreateConstantLiteral(extensionOpId);
  LightningShaderIROp* extensionOp = translator->BuildIROpNoBlockAdd(OpType::OpExtInst, resultType, context);
  extensionOp->mArguments.PushBack(importLibraryIR);
  extensionOp->mArguments.PushBack(instructionLiteral);
  return extensionOp;
}

template <int extensionOpId>
void BasicExtensionFunction(LightningSpirVFrontEnd* translator,
                            Lightning::FunctionCallNode* functionCallNode,
                            Lightning::MemberAccessNode* memberAccessNode,
                            LightningShaderExtensionImport* importLibraryIR,
                            LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* extensionOp =
      MakeBasicExtensionFunction(translator, functionCallNode, extensionOpId, importLibraryIR, context);

  // Write the remaining function arguments
  translator->WriteFunctionCallArguments(functionCallNode, extensionOp, context);

  BasicBlock* currentBlock = context->GetCurrentBlock();
  currentBlock->mLines.PushBack(extensionOp);
}

void ResolveGlslExtensionFunction(LightningSpirVFrontEnd* translator,
                                  Lightning::FunctionCallNode* functionCallNode,
                                  Lightning::MemberAccessNode* memberAccessNode,
                                  LightningSpirVFrontEndContext* context)
{
  ExtensionLibraryUserData& userData =
      memberAccessNode->AccessedFunction->ComplexUserData.ReadObject<ExtensionLibraryUserData>(0);
  LightningShaderExtensionImport* importOp = translator->mLibrary->FindExtensionLibraryImport(userData.mExtensionLibrary);
  LightningShaderIROp* extensionOp =
      MakeBasicExtensionFunction(translator, functionCallNode, userData.mOpCode, importOp, context);

  // Write the remaining function arguments
  translator->WriteFunctionCallArguments(functionCallNode, extensionOp, context);

  BasicBlock* currentBlock = context->GetCurrentBlock();
  currentBlock->mLines.PushBack(extensionOp);
}

// SpirV FSign returns a float but in lightning it returns an int.
void GenerateFSign(LightningSpirVFrontEnd* translator,
                   Lightning::FunctionCallNode* functionCallNode,
                   Lightning::MemberAccessNode* memberAccessNode,
                   LightningShaderExtensionImport* importLibraryIR,
                   LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  LightningShaderIRType* intResultType = translator->FindType(functionCallNode->ResultType, functionCallNode);
  LightningShaderIRType* realResultType =
      translator->FindType(functionCallNode->Arguments[0]->ResultType, functionCallNode);
  // Convert the FSign instruction with the float type
  LightningShaderIRConstantLiteral* instructionLiteral = translator->GetOrCreateConstantLiteral((int)GLSLstd450FSign);
  LightningShaderIROp* extensionOp = translator->BuildIROpNoBlockAdd(OpType::OpExtInst, realResultType, context);
  extensionOp->mArguments.PushBack(importLibraryIR);
  extensionOp->mArguments.PushBack(instructionLiteral);

  // Write the remaining function arguments
  translator->WriteFunctionCallArguments(functionCallNode, extensionOp, context);
  // Pop the extension op off the stack and write the instruction to the current
  // block
  context->PopIRStack();
  currentBlock->mLines.PushBack(extensionOp);

  // Now write out the conversion from float to int so the types match lightning
  LightningShaderIROp* intSignOp =
      translator->BuildIROp(currentBlock, OpType::OpConvertFToS, intResultType, extensionOp, context);
  context->PushIRStack(intSignOp);
}

void GenerateAngleAndTrigFunctions(SpirVExtensionLibrary* extLibrary, LightningTypeGroups& types)
{
  Lightning::Core& core = Lightning::Core::GetInstance();
  Lightning::BoundType* mathType = core.MathType;

  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mRealVectorTypes[i];
    String lightningTypeName = lightningType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ToRadians", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Radians>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ToDegrees", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Degrees>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Cos", lightningTypeName), BasicExtensionFunction<GLSLstd450Cos>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sin", lightningTypeName), BasicExtensionFunction<GLSLstd450Sin>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Tan", lightningTypeName), BasicExtensionFunction<GLSLstd450Tan>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ACos", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Acos>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ASin", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Asin>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ATan", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Atan>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ATan2", lightningTypeName, lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Atan2>);

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Cosh", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Cosh>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sinh", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Sinh>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Tanh", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Tanh>);
  }
}

void GenerateExponentialFunctions(SpirVExtensionLibrary* extLibrary, LightningTypeGroups& types)
{
  Lightning::Core& core = Lightning::Core::GetInstance();
  Lightning::BoundType* mathType = core.MathType;

  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mRealVectorTypes[i];
    String lightningTypeName = lightningType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Pow", lightningTypeName, lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Pow>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Log", lightningTypeName), BasicExtensionFunction<GLSLstd450Log>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Exp", lightningTypeName), BasicExtensionFunction<GLSLstd450Exp>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Log2", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Log2>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Exp2", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Exp2>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sqrt", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Sqrt>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "RSqrt", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450InverseSqrt>);
  }
}

void GenerateCommonFloatFunctions(SpirVExtensionLibrary* extLibrary, LightningTypeGroups& types)
{
  Lightning::Core& core = Lightning::Core::GetInstance();
  Lightning::BoundType* mathType = core.MathType;

  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mRealVectorTypes[i];
    String lightningTypeName = lightningType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Abs", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450FAbs>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sign", lightningTypeName), GenerateFSign);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Floor", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Floor>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Ceil", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Ceil>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Frac", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Fract>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Truncate", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Trunc>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Round", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Round>);

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Min", lightningTypeName, lightningTypeName),
                              BasicExtensionFunction<GLSLstd450FMin>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Max", lightningTypeName, lightningTypeName),
                              BasicExtensionFunction<GLSLstd450FMax>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Clamp", lightningTypeName, lightningTypeName, lightningTypeName),
                              BasicExtensionFunction<GLSLstd450FClamp>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Lerp", lightningTypeName, lightningTypeName, lightningTypeName),
                              BasicExtensionFunction<GLSLstd450FMix>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Step", lightningTypeName, lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Step>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "SmoothStep", lightningTypeName, lightningTypeName, lightningTypeName),
                              BasicExtensionFunction<GLSLstd450SmoothStep>);
  }
}

void GenerateCommonIntFunctions(SpirVExtensionLibrary* extLibrary, LightningTypeGroups& types)
{
  Lightning::Core& core = Lightning::Core::GetInstance();
  Lightning::BoundType* mathType = core.MathType;

  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mIntegerVectorTypes[i];
    String lightningTypeName = lightningType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Abs", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450SAbs>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Clamp", lightningTypeName, lightningTypeName, lightningTypeName),
                              BasicExtensionFunction<GLSLstd450SClamp>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Min", lightningTypeName, lightningTypeName),
                              BasicExtensionFunction<GLSLstd450SMin>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Max", lightningTypeName, lightningTypeName),
                              BasicExtensionFunction<GLSLstd450SMax>);

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sign", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450SSign>);
  }
}

void GenerateGeometricFloatFunctions(SpirVExtensionLibrary* extLibrary, LightningTypeGroups& types)
{
  Lightning::Core& core = Lightning::Core::GetInstance();
  Lightning::BoundType* mathType = core.MathType;

  String realName = core.RealType->ToString();
  String real2Name = core.Real2Type->ToString();
  String real3Name = core.Real3Type->ToString();

  for (size_t i = 1; i < types.mRealVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mRealVectorTypes[i];
    String lightningTypeName = lightningType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Distance", lightningTypeName, lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Distance>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Length", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Length>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Normalize", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Normalize>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ReflectAcrossPlane", lightningTypeName, lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Reflect>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Refract", lightningTypeName, lightningTypeName, realName),
                              BasicExtensionFunction<GLSLstd450Refract>);
  }

  extLibrary->CreateExtInst(GetStaticFunction(mathType, "Cross", real3Name, real3Name),
                            BasicExtensionFunction<GLSLstd450Cross>);
}

void CreateFloatMatrixFunctions(SpirVExtensionLibrary* extLibrary, LightningTypeGroups& types)
{
  Lightning::Core& core = Lightning::Core::GetInstance();
  Lightning::BoundType* mathType = core.MathType;

  for (u32 i = 2; i <= 4; ++i)
  {
    Lightning::BoundType* lightningType = types.GetMatrixType(i, i);
    String lightningTypeName = lightningType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Invert", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450MatrixInverse>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Determinant", lightningTypeName),
                              BasicExtensionFunction<GLSLstd450Determinant>);
  }
}

// Registers callback functions for all of the glsl 450 extension library
// instructions that exist in lightning
void RegisterGlsl450Extensions(LightningShaderIRLibrary* shaderLibrary,
                               SpirVExtensionLibrary* extLibrary,
                               LightningTypeGroups& types)
{
  Lightning::Core& core = Lightning::Core::GetInstance();
  Lightning::BoundType* mathType = core.MathType;

  extLibrary->mName = "GLSL.std.450";
  extLibrary->mOwningLibrary = shaderLibrary;

  GenerateAngleAndTrigFunctions(extLibrary, types);
  GenerateExponentialFunctions(extLibrary, types);
  GenerateCommonIntFunctions(extLibrary, types);
  GenerateCommonFloatFunctions(extLibrary, types);
  GenerateGeometricFloatFunctions(extLibrary, types);
  CreateFloatMatrixFunctions(extLibrary, types);
}

// Simple helper to look up a zich function via name and type and then map it to
// a glsl extension instruction
void AddGlslIntrinsic(Lightning::LibraryBuilder& builder,
                      Lightning::BoundType* type,
                      SpirVExtensionLibrary* extLibrary,
                      int glslOpId,
                      StringParam fnName,
                      const Lightning::ParameterArray& parameters,
                      Lightning::BoundType* returnType)
{
  Lightning::Function* fn = builder.AddBoundFunction(
      type, fnName, UnTranslatedBoundFunction, parameters, returnType, Lightning::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveGlslExtensionFunction;
  fn->ComplexUserData.WriteObject(ExtensionLibraryUserData(glslOpId, extLibrary));
}

/// Adds all relevant glsl extension operations to the ShaderIntrinsics type,
/// including non-supported instructions in lightning.
void AddGlslExtensionIntrinsicOps(Lightning::LibraryBuilder& builder,
                                  SpirVExtensionLibrary* extLibrary,
                                  Lightning::BoundType* type,
                                  LightningTypeGroups& types)
{
  Lightning::BoundType* realType = types.mRealVectorTypes[0];
  Lightning::BoundType* real3Type = types.mRealVectorTypes[2];

  // Reals
  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mRealVectorTypes[i];

    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Round, "Round", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450RoundEven, "RoundEven", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Trunc, "Trunc", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FAbs, "FAbs", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FSign, "FSign", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Floor, "Floor", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Ceil, "Ceil", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Fract, "Fract", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Radians, "Radians", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Degrees, "Degrees", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Sin, "Sin", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Cos, "Cos", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Tan, "Tan", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Sinh, "Sinh", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Cosh, "Cosh", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Tanh, "Tanh", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Asin, "ASin", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Acos, "ACos", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Atan, "ATan", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Asinh, "ASinh", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Acosh, "ACosh", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Atanh, "ATanh", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450Atan2, "ATan2", TwoParameters(lightningType, "y", lightningType, "x"), lightningType);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450Pow, "Pow", TwoParameters(lightningType, "base", lightningType, "exp"), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Exp, "Exp", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Log, "Log", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Exp2, "Exp2", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Log2, "Log2", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Sqrt, "Sqrt", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450InverseSqrt, "InverseSqrt", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FMin, "FMin", TwoParameters(lightningType, lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FMax, "FMax", TwoParameters(lightningType, lightningType), lightningType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450FClamp,
                     "FClamp",
                     ThreeParameters(lightningType, "value", lightningType, "minValue", lightningType, "maxValue"),
                     lightningType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450FMix,
                     "FMix",
                     ThreeParameters(lightningType, "start", lightningType, "end", lightningType, "t"),
                     lightningType);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450Step, "Step", TwoParameters(lightningType, "y", lightningType, "x"), lightningType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450SmoothStep,
                     "SmoothStep",
                     ThreeParameters(lightningType, "start", lightningType, "end", lightningType, "t"),
                     lightningType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450Fma,
                     "Fma",
                     ThreeParameters(lightningType, "a", lightningType, "b", lightningType, "c"),
                     lightningType);

    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Length, "Length", OneParameter(lightningType), realType);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450Distance, "Distance", TwoParameters(lightningType, lightningType), realType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Normalize, "Normalize", OneParameter(lightningType), lightningType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450FaceForward,
                     "FaceForward",
                     ThreeParameters(lightningType, "n", lightningType, "i", lightningType, "nRef"),
                     lightningType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450Reflect,
                     "Reflect",
                     TwoParameters(lightningType, "i", lightningType, "n"),
                     lightningType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450Refract,
                     "Refract",
                     ThreeParameters(lightningType, "i", lightningType, "n", realType, "eta"),
                     lightningType);

    // Causes SpirV-Cross exceptions
    // AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450NMin, "NMin",
    // TwoParameters(lightningType, lightningType), lightningType);
    // AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450NMax, "NMax",
    // TwoParameters(lightningType, lightningType), lightningType);
    // AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450NClamp, "NClamp",
    // ThreeParameters(lightningType, "value", lightningType, "minValue", lightningType,
    // "maxValue"), lightningType);

    // Requires pointer types
    // AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Modf, "Modf",
    // TwoParameters(lightningType, "value"), lightningType);
  }

  // Integer
  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mIntegerVectorTypes[i];

    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SAbs, "SAbs", OneParameter(lightningType, "value"), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SSign, "SSign", OneParameter(lightningType, "value"), lightningType);

    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SMin, "SMin", TwoParameters(lightningType, lightningType), lightningType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SMax, "SMax", TwoParameters(lightningType, lightningType), lightningType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450SClamp,
                     "SClamp",
                     ThreeParameters(lightningType, "value", lightningType, "minValue", lightningType, "maxValue"),
                     lightningType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450FindILsb,
                     "FindLeastSignificantBit",
                     OneParameter(lightningType, "value"),
                     lightningType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450FindSMsb,
                     "FindMostSignificantBit",
                     OneParameter(lightningType, "value"),
                     lightningType);
  }

  // Matrices
  for (u32 i = 2; i <= 4; ++i)
  {
    Lightning::BoundType* lightningType = types.GetMatrixType(i, i);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450Determinant, "Determinant", OneParameter(lightningType), realType);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450MatrixInverse, "MatrixInverse", OneParameter(lightningType), lightningType);
  }

  AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Cross, "Cross", TwoParameters(real3Type, real3Type), real3Type);
}

} // namespace Plasma
