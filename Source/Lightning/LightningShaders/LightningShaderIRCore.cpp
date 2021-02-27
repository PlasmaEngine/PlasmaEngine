// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "LightningShaderIRCore.hpp"
#include "ShaderIRLibraryTranslation.hpp"

namespace Plasma
{

LightningShaderIRCore* LightningShaderIRCore::mInstance = nullptr;

void LightningShaderIRCore::InitializeInstance()
{
  ReturnIf(mInstance != nullptr, , "Can't initialize a static library more than once");
  mInstance = new LightningShaderIRCore();
}

void LightningShaderIRCore::Destroy()
{
  delete mInstance;
  mInstance = nullptr;
}

LightningShaderIRCore& LightningShaderIRCore::GetInstance()
{
  ErrorIf(mInstance == nullptr, "Attempted to get an uninitialized singleton static library");

  return *mInstance;
}

LightningShaderIRCore::LightningShaderIRCore()
{
	mGlsl450ExtensionsLibrary = nullptr;

	Lightning::Core& core = Lightning::Core::GetInstance();
	mLightningTypes.mVoidType = core.VoidType;

	mLightningTypes.mRealVectorTypes.PushBack(core.RealType);
    mLightningTypes.mRealVectorTypes.PushBack(core.Real2Type);
    mLightningTypes.mRealVectorTypes.PushBack(core.Real3Type);
    mLightningTypes.mRealVectorTypes.PushBack(core.Real4Type);

	// Make all of the matrix types
	for (u32 y = 2; y <= 4; ++y)
	{
	for (u32 x = 2; x <= 4; ++x)
	{
        Lightning::BoundType* basisType = mLightningTypes.mRealVectorTypes[x - 1];
        String matrixName = BuildString("Real", ToString(y), "x", ToString(x));
        Lightning::BoundType* zilchMatrixType = core.GetLibrary()->BoundTypes.FindValue(matrixName, nullptr);
        mLightningTypes.mRealMatrixTypes.PushBack(zilchMatrixType);
	}
	}

    mLightningTypes.mIntegerVectorTypes.PushBack(core.IntegerType);
    mLightningTypes.mIntegerVectorTypes.PushBack(core.Integer2Type);
    mLightningTypes.mIntegerVectorTypes.PushBack(core.Integer3Type);
    mLightningTypes.mIntegerVectorTypes.PushBack(core.Integer4Type);

	mLightningTypes.mBooleanVectorTypes.PushBack(core.BooleanType);
    mLightningTypes.mBooleanVectorTypes.PushBack(core.Boolean2Type);
    mLightningTypes.mBooleanVectorTypes.PushBack(core.Boolean3Type);
    mLightningTypes.mBooleanVectorTypes.PushBack(core.Boolean4Type);

	mLightningTypes.mQuaternionType = core.QuaternionType;
}

void LightningShaderIRCore::Parse(LightningSpirVFrontEnd* translator)
{
  LightningShaderIRLibrary* shaderLibrary = new LightningShaderIRLibrary();
  shaderLibrary->mLightningLibrary = Lightning::Core::GetInstance().GetLibrary();
  mLibraryRef = shaderLibrary;
  translator->mLibrary = shaderLibrary;

  Lightning::Core& core = Lightning::Core::GetInstance();
  Lightning::BoundType* mathType = core.MathType;
  translator->MakeStructType(shaderLibrary, core.MathType->Name, core.MathType, spv::StorageClass::StorageClassGeneric);

  Lightning::BoundType* lightningIntType = core.IntegerType;
  String intTypeName = lightningIntType->ToString();

  ShaderTypeGroups& types = mShaderTypes;
  MakeMathTypes(translator, shaderLibrary, types);

  // Add all static/instance functions for primitive types
  RegisterPrimitiveFunctions(translator, shaderLibrary, types, types.mRealVectorTypes[0]);
  RegisterPrimitiveFunctions(translator, shaderLibrary, types, types.mIntegerVectorTypes[0]);
  RegisterPrimitiveFunctions(translator, shaderLibrary, types, types.mBooleanVectorTypes[0]);

  // Add all static/instance functions for vector types
  RegisterVectorFunctions(translator, shaderLibrary, types, types.mRealVectorTypes);
  RegisterVectorFunctions(translator, shaderLibrary, types, types.mIntegerVectorTypes);
  RegisterVectorFunctions(translator, shaderLibrary, types, types.mBooleanVectorTypes);

  // Add all static/instance functions for matrix types (only float matrices
  // exist)
  RegisterMatrixFunctions(translator, shaderLibrary, types, types.mRealMatrixTypes);
  // Also the quaternion type
  RegisterQuaternionFunctions(translator, shaderLibrary, types, types.mQuaternionType);

  // Add a bunch of math ops (a lot on the math class)
  RegisterArithmeticOps(translator, shaderLibrary, mLightningTypes);
  RegisterConversionOps(translator, shaderLibrary, mLightningTypes);
  RegisterLogicalOps(translator, shaderLibrary, mLightningTypes);
  RegisterBitOps(translator, shaderLibrary, mLightningTypes);
  RegisterColorsOps(translator, shaderLibrary, mLightningTypes);

  // Add special extension functions such as Matrix.Determinant and Sin
  mGlsl450ExtensionsLibrary = new SpirVExtensionLibrary();
  shaderLibrary->mExtensionLibraries.PushBack(mGlsl450ExtensionsLibrary);
  RegisterGlsl450Extensions(shaderLibrary, mGlsl450ExtensionsLibrary, mLightningTypes);
  shaderLibrary->mTranslated = true;
}

LightningShaderIRLibraryRef LightningShaderIRCore::GetLibrary()
{
  return mLibraryRef;
}

void LightningShaderIRCore::MakeMathTypes(LightningSpirVFrontEnd* translator,
                                      LightningShaderIRLibrary* shaderLibrary,
                                      ShaderTypeGroups& types)
{
  Lightning::Core& core = Lightning::Core::GetInstance();

  types.mVoidType =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Void, 0, nullptr, core.VoidType, false);

  LightningShaderIRType* floatType =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Float, 1, nullptr, core.RealType);
  LightningShaderIRType* float2Type =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 2, floatType, core.Real2Type);
  LightningShaderIRType* float3Type =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 3, floatType, core.Real3Type);
  LightningShaderIRType* float4Type =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 4, floatType, core.Real4Type);

  types.mRealVectorTypes.PushBack(floatType);
  types.mRealVectorTypes.PushBack(float2Type);
  types.mRealVectorTypes.PushBack(float3Type);
  types.mRealVectorTypes.PushBack(float4Type);

  // Make all of the matrix types
  for (size_t y = 2; y <= 4; ++y)
  {
    for (size_t x = 2; x <= 4; ++x)
    {
      LightningShaderIRType* basisType = types.mRealVectorTypes[x - 1];
      String matrixName = BuildString("Real", ToString(y), "x", ToString(x));
      Lightning::BoundType* lightningMatrixType = core.GetLibrary()->BoundTypes.FindValue(matrixName, nullptr);
      LightningShaderIRType* shaderMatrixType =
          translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Matrix, y, basisType, lightningMatrixType);
      types.mRealMatrixTypes.PushBack(shaderMatrixType);
    }
  }

  LightningShaderIRType* intType =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Int, 1, nullptr, core.IntegerType);
  LightningShaderIRType* int2Type =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 2, intType, core.Integer2Type);
  LightningShaderIRType* int3Type =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 3, intType, core.Integer3Type);
  LightningShaderIRType* int4Type =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 4, intType, core.Integer4Type);
  types.mIntegerVectorTypes.PushBack(intType);
  types.mIntegerVectorTypes.PushBack(int2Type);
  types.mIntegerVectorTypes.PushBack(int3Type);
  types.mIntegerVectorTypes.PushBack(int4Type);

  LightningShaderIRType* boolType =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Bool, 1, nullptr, core.BooleanType);
  LightningShaderIRType* bool2Type =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 2, boolType, core.Boolean2Type);
  LightningShaderIRType* bool3Type =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 3, boolType, core.Boolean3Type);
  LightningShaderIRType* bool4Type =
      translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 4, boolType, core.Boolean4Type);
  types.mBooleanVectorTypes.PushBack(boolType);
  types.mBooleanVectorTypes.PushBack(bool2Type);
  types.mBooleanVectorTypes.PushBack(bool3Type);
  types.mBooleanVectorTypes.PushBack(bool4Type);

  // Make quaternion a struct type. Ideally quaternion would just be a vec4
  // type, but it's illegal to declare multiple vec4 types. This causes a lot of
  // complications in translating core functionality.
  Lightning::BoundType* lightningQuaternion = core.QuaternionType;
  String quaternionTypeName = lightningQuaternion->ToString();
  LightningShaderIRType* quaternionType =
      translator->MakeStructType(shaderLibrary, quaternionTypeName, core.QuaternionType, spv::StorageClassFunction);
  quaternionType->mDebugResultName = quaternionType->mName = quaternionTypeName;
  quaternionType->AddMember(float4Type, "Data");
  types.mQuaternionType = quaternionType;
}

void LightningShaderIRCore::RegisterPrimitiveFunctions(LightningSpirVFrontEnd* translator,
                                                   LightningShaderIRLibrary* shaderLibrary,
                                                   ShaderTypeGroups& types,
                                                   LightningShaderIRType* shaderType)
{
  Lightning::BoundType* intType = types.mIntegerVectorTypes[0]->mLightningType;
  Lightning::BoundType* lightningType = shaderType->mLightningType;
  String lightningTypeName = lightningType->ToString();
  String intTypeName = intType->ToString();

  TypeResolvers& primitiveTypeResolvers = shaderLibrary->mTypeResolvers[lightningType];
  primitiveTypeResolvers.mDefaultConstructorResolver = &TranslatePrimitiveDefaultConstructor;
  primitiveTypeResolvers.mBackupConstructorResolver = &TranslateBackupPrimitiveConstructor;
  primitiveTypeResolvers.RegisterBackupFieldResolver(&ScalarBackupFieldResolver);
  primitiveTypeResolvers.RegisterFunctionResolver(GetStaticProperty(lightningType, "Count")->Get, ResolveVectorTypeCount);
  primitiveTypeResolvers.RegisterFunctionResolver(GetInstanceProperty(lightningType, "Count")->Get, ResolveVectorTypeCount);
  primitiveTypeResolvers.RegisterFunctionResolver(
      GetMemberOverloadedFunction(lightningType, Lightning::OperatorGet, intTypeName), ResolvePrimitiveGet);
  primitiveTypeResolvers.RegisterFunctionResolver(
      GetMemberOverloadedFunction(lightningType, Lightning::OperatorSet, intTypeName, lightningTypeName), ResolvePrimitiveSet);
}

void LightningShaderIRCore::RegisterVectorFunctions(LightningSpirVFrontEnd* translator,
                                                LightningShaderIRLibrary* shaderLibrary,
                                                ShaderTypeGroups& types,
                                                Array<LightningShaderIRType*>& vectorTypes)
{
  Lightning::BoundType* intType = types.mIntegerVectorTypes[0]->mLightningType;
  String intTypeName = intType->ToString();

  for (size_t i = 1; i < vectorTypes.Size(); ++i)
  {
    LightningShaderIRType* shaderType = vectorTypes[i];
    Lightning::BoundType* lightningType = shaderType->mLightningType;
    String lightningTypeName = lightningType->ToString();
    LightningShaderIRType* componentType = GetComponentType(shaderType);
    Lightning::BoundType* lightningComponentType = componentType->mLightningType;
    String lightningComponentTypeName = lightningComponentType->ToString();

    TypeResolvers& primitiveTypeResolvers = shaderLibrary->mTypeResolvers[lightningType];
    primitiveTypeResolvers.mDefaultConstructorResolver = &TranslateCompositeDefaultConstructor;
    primitiveTypeResolvers.RegisterBackupFieldResolver(&VectorBackupFieldResolver);
    primitiveTypeResolvers.RegisterConstructorResolver(lightningType->GetDefaultConstructor(),
                                                       TranslateCompositeDefaultConstructor);
    primitiveTypeResolvers.RegisterConstructorResolver(GetConstructor(lightningType, lightningComponentTypeName),
                                                       TranslateCompositeSplatConstructor);
    primitiveTypeResolvers.RegisterFunctionResolver(GetStaticProperty(lightningType, "Count")->Get, ResolveVectorTypeCount);
    primitiveTypeResolvers.RegisterFunctionResolver(GetInstanceProperty(lightningType, "Count")->Get,
                                                    ResolveVectorTypeCount);
    primitiveTypeResolvers.RegisterFunctionResolver(
        GetMemberOverloadedFunction(lightningType, Lightning::OperatorGet, intTypeName), ResolveVectorGet);
    primitiveTypeResolvers.RegisterFunctionResolver(
        GetMemberOverloadedFunction(lightningType, Lightning::OperatorSet, intTypeName, lightningComponentTypeName),
        ResolveVectorSet);
    primitiveTypeResolvers.RegisterBackupSetterResolver(VectorBackupPropertySetter);
    primitiveTypeResolvers.mBackupConstructorResolver = &TranslateBackupCompositeConstructor;
  }
}

void LightningShaderIRCore::RegisterMatrixFunctions(LightningSpirVFrontEnd* translator,
                                                LightningShaderIRLibrary* shaderLibrary,
                                                ShaderTypeGroups& types,
                                                Array<LightningShaderIRType*>& matrixTypes)
{
  Lightning::BoundType* intType = types.mIntegerVectorTypes[0]->mLightningType;
  String intTypeName = intType->ToString();

  for (size_t i = 0; i < matrixTypes.Size(); ++i)
  {
    LightningShaderIRType* shaderType = matrixTypes[i];
    Lightning::BoundType* lightningType = shaderType->mLightningType;
    // Get the component (vector row) type
    LightningShaderIRType* componentType = GetComponentType(shaderType);
    Lightning::BoundType* lightningComponentType = componentType->mLightningType;
    String lightningComponentTypeName = lightningComponentType->ToString();
    // Get the scalar type of the matrix
    LightningShaderIRType* scalarType = GetComponentType(componentType);
    Lightning::BoundType* lightningScalarType = scalarType->mLightningType;

    TypeResolvers& matrixTypeResolver = shaderLibrary->mTypeResolvers[lightningType];
    // Constructors
    matrixTypeResolver.mDefaultConstructorResolver = &TranslateMatrixDefaultConstructor;
    matrixTypeResolver.RegisterConstructorResolver(lightningType->GetDefaultConstructor(),
                                                   TranslateMatrixDefaultConstructor);
    matrixTypeResolver.RegisterConstructorResolver(GetConstructor(lightningType, lightningScalarType->ToString()),
                                                   TranslateCompositeSplatConstructor);
    // Constructor for each scalar entry in the matrix
    Array<String> constructorParams;
    for (size_t i = 0; i < shaderType->mComponents * componentType->mComponents; ++i)
      constructorParams.PushBack(lightningScalarType->ToString());
    matrixTypeResolver.RegisterConstructorResolver(GetConstructor(lightningType, constructorParams),
                                                   TranslateMatrixFullConstructor);

    // Fields (M00, etc...)
    matrixTypeResolver.RegisterBackupFieldResolver(&MatrixBackupFieldResolver);

    matrixTypeResolver.RegisterFunctionResolver(GetMemberOverloadedFunction(lightningType, Lightning::OperatorGet, intTypeName),
                                                ResolveMatrixGet);
    matrixTypeResolver.RegisterFunctionResolver(
        GetMemberOverloadedFunction(lightningType, Lightning::OperatorSet, intTypeName, lightningComponentTypeName),
        ResolveMatrixSet);
  }
}

void LightningShaderIRCore::RegisterQuaternionFunctions(LightningSpirVFrontEnd* translator,
                                                    LightningShaderIRLibrary* shaderLibrary,
                                                    ShaderTypeGroups& types,
                                                    LightningShaderIRType* quaternionType)
{
  Lightning::BoundType* intType = types.mIntegerVectorTypes[0]->mLightningType;
  String intTypeName = intType->ToString();

  Lightning::BoundType* lightningType = quaternionType->mLightningType;
  String lightningTypeName = lightningType->ToString();
  // While quaternion's component type is technically vec4, all operations
  // behave as if it's real
  LightningShaderIRType* componentType = types.mRealVectorTypes[0];
  Lightning::BoundType* lightningComponentType = componentType->mLightningType;
  String lightningComponentTypeName = lightningComponentType->ToString();

  TypeResolvers& primitiveTypeResolvers = shaderLibrary->mTypeResolvers[lightningType];
  primitiveTypeResolvers.mDefaultConstructorResolver = &TranslateQuaternionDefaultConstructor;
  primitiveTypeResolvers.RegisterBackupFieldResolver(&QuaternionBackupFieldResolver);
  primitiveTypeResolvers.RegisterConstructorResolver(lightningType->GetDefaultConstructor(),
                                                     TranslateQuaternionDefaultConstructor);
  primitiveTypeResolvers.RegisterConstructorResolver(GetConstructor(lightningType, lightningComponentTypeName),
                                                     TranslateQuaternionSplatConstructor);
  primitiveTypeResolvers.RegisterFunctionResolver(GetStaticProperty(lightningType, "Count")->Get,
                                                  ResolveQuaternionTypeCount);
  primitiveTypeResolvers.RegisterFunctionResolver(GetInstanceProperty(lightningType, "Count")->Get,
                                                  ResolveQuaternionTypeCount);
  primitiveTypeResolvers.RegisterFunctionResolver(
      GetMemberOverloadedFunction(lightningType, Lightning::OperatorGet, intTypeName), ResolveQuaternionGet);
  primitiveTypeResolvers.RegisterFunctionResolver(
      GetMemberOverloadedFunction(lightningType, Lightning::OperatorSet, intTypeName, lightningComponentTypeName),
      ResolveQuaternionSet);
  primitiveTypeResolvers.RegisterBackupSetterResolver(QuaternionBackupPropertySetter);
  primitiveTypeResolvers.mBackupConstructorResolver = &TranslateBackupQuaternionConstructor;
}

} // namespace Plasma
