// MIT Licensed (see LICENSE.md).
// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "ShaderErrors.hpp"

namespace Lightning
{

BoundType* InstantiateFixedArray(LibraryBuilder& builder,
                                 StringParam baseName,
                                 StringParam fullyQualifiedName,
                                 const Array<Constant>& templateTypes,
                                 const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;
  // Bind the arraytype
  BoundType* arrayType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);

  // Bind all of the array's functions and properties (stubbed out since we're
  // only using this for translation)
  builder.AddBoundConstructor(arrayType, Plasma::DummyBoundFunction, ParameterArray());
  builder.AddBoundFunction(arrayType,
                           "Add",
                           Plasma::DummyBoundFunction,
                           OneParameter(templateType),
                           core.VoidType,
                           Lightning::FunctionOptions::None);
  builder.AddBoundFunction(arrayType,
                           "Get",
                           Plasma::DummyBoundFunction,
                           OneParameter(core.IntegerType, "index"),
                           templateType,
                           Lightning::FunctionOptions::None);
  builder.AddBoundFunction(arrayType,
                           "Set",
                           Plasma::DummyBoundFunction,
                           TwoParameters(core.IntegerType, "index", templateType, "value"),
                           core.VoidType,
                           Lightning::FunctionOptions::None);
  builder.AddBoundGetterSetter(
      arrayType, "Count", core.IntegerType, nullptr, Plasma::DummyBoundFunction, Lightning::MemberOptions::None);

  return arrayType;
}

BoundType* InstantiateRuntimeArray(LibraryBuilder& builder,
                                   StringParam baseName,
                                   StringParam fullyQualifiedName,
                                   const Array<Constant>& templateTypes,
                                   const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;
  // Bind the arraytype
  BoundType* arrayType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  Attribute* storageAttribute = arrayType->AddAttribute(Plasma::SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassStorageBuffer);
  arrayType->AddAttribute(Plasma::SpirVNameSettings::mNonCopyableAttributeName);

  // Bind all of the array's functions and properties (stubbed out since we're
  // only using this for translation)
  builder.AddBoundFunction(arrayType,
                           "Get",
                           Plasma::DummyBoundFunction,
                           OneParameter(core.IntegerType, "index"),
                           templateType,
                           Lightning::FunctionOptions::None);
  builder.AddBoundFunction(arrayType,
                           "Set",
                           Plasma::DummyBoundFunction,
                           TwoParameters(core.IntegerType, "index", templateType, "value"),
                           core.VoidType,
                           Lightning::FunctionOptions::None);
  builder.AddBoundGetterSetter(
      arrayType, "Count", core.IntegerType, nullptr, Plasma::DummyBoundFunction, Lightning::MemberOptions::None);

  return arrayType;
}

BoundType* InstantiateGeometryInput(LibraryBuilder& builder,
                                    StringParam baseName,
                                    StringParam fullyQualifiedName,
                                    const Array<Constant>& templateTypes,
                                    const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;

  BoundType* selfType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  // Bind all of the functions and properties (stubbed out since we're only
  // using this for translation)
  builder.AddBoundConstructor(selfType, Plasma::UnTranslatedBoundFunction, ParameterArray());
  builder.AddBoundFunction(selfType,
                           "Get",
                           Plasma::UnTranslatedBoundFunction,
                           OneParameter(core.IntegerType),
                           templateType,
                           Lightning::FunctionOptions::None);
  builder.AddBoundFunction(selfType,
                           "Set",
                           Plasma::UnTranslatedBoundFunction,
                           TwoParameters(core.IntegerType, templateType),
                           core.VoidType,
                           Lightning::FunctionOptions::None);
  builder.AddBoundGetterSetter(
      selfType, "Count", core.IntegerType, nullptr, Plasma::UnTranslatedBoundFunction, Lightning::MemberOptions::None);

  Lightning::HandleOf<GeometryStreamUserData> handle = LightningAllocate(GeometryStreamUserData);
  handle->Set((spv::ExecutionMode)(uintptr_t)userData);
  selfType->Add(*handle);

  return selfType;
}

BoundType* InstantiateGeometryOutput(LibraryBuilder& builder,
                                     StringParam baseName,
                                     StringParam fullyQualifiedName,
                                     const Array<Constant>& templateTypes,
                                     const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;

  BoundType* selfType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  selfType->CreatableInScript = true;
  // Bind all of the functions and properties (stubbed out since we're only
  // using this for translation)
  builder.AddBoundFunction(selfType,
                           "Append",
                           Plasma::UnTranslatedBoundFunction,
                           TwoParameters(templateType, core.IntegerType),
                           core.VoidType,
                           Lightning::FunctionOptions::None);
  builder.AddBoundFunction(selfType,
                           "Restart",
                           Plasma::UnTranslatedBoundFunction,
                           ParameterArray(),
                           core.VoidType,
                           Lightning::FunctionOptions::None);

  Lightning::HandleOf<GeometryStreamUserData> handle = LightningAllocate(GeometryStreamUserData);
  handle->Set((spv::ExecutionMode)(uintptr_t)userData);
  selfType->Add(*handle);

  return selfType;
}

BoundType* InstantiateGeometryStreamMover(LibraryBuilder& builder,
                                          StringParam baseName,
                                          StringParam fullyQualifiedName,
                                          const Array<Constant>& templateTypes,
                                          const void* userData)
{
  Type* fromType = templateTypes[0].TypeValue;
  Type* toType = templateTypes[1].TypeValue;

  BoundType* selfType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  selfType->CreatableInScript = false;
  builder.AddBoundFunction(selfType,
                           "Move",
                           Plasma::UnTranslatedBoundFunction,
                           OneParameter(fromType),
                           toType,
                           Lightning::FunctionOptions::Static);

  return selfType;
}

LightningDefineStaticLibrary(ShaderIntrinsicsLibrary)
{
  builder.CreatableInScriptDefault = false;

  LightningInitializeType(UnsignedInt);

  // BoundType Components
  LightningInitializeType(GeometryStreamUserData);
  LightningInitializeType(GeometryFragmentUserData);
  LightningInitializeType(ComputeFragmentUserData);

  // @Nate: These have to be uncommented for new shaders
  LightningInitializeType(ShaderIntrinsics);
  LightningInitializeType(Sampler);
  LightningInitializeType(Image2d);
  LightningInitializeType(StorageImage2d);
  LightningInitializeType(DepthImage2d);
  LightningInitializeType(ImageCube);
  LightningInitializeType(SampledImage2d);
  LightningInitializeType(SampledDepthImage2d);
  LightningInitializeType(SampledImageCube);

  // Bind the fixed array type instantiator (creates the different arrays when
  // instantiated)
	{
		Array<Lightning::TemplateParameter> templateTypes;
		TemplateParameter& typeParam = templateTypes.PushBack();
		typeParam.Name = "Type";
		typeParam.Type = ConstantType::Type;
		TemplateParameter& sizeParam = templateTypes.PushBack();
		sizeParam.Name = "Size";
		sizeParam.Type = ConstantType::Integer;
		builder.AddTemplateInstantiator("FixedArray", InstantiateFixedArray, templateTypes, nullptr);
	}

	{
		Lightning::Array<Lightning::Constant> fixedArrayTemplateParams;
	    fixedArrayTemplateParams.PushBack(LightningTypeId(Lightning::Real4x4));
	    fixedArrayTemplateParams.PushBack(80);

	    Lightning::InstantiatedTemplate templateData = builder.InstantiateTemplate(
	        "FixedArray", fixedArrayTemplateParams, LibraryArray(PlasmaInit, builder.BuiltLibrary));
		Lightning::BoundType* boneTransformsType = templateData.Type;
	}

  // Bind the runtime array type instantiator (creates the different arrays when
  // instantiated)
  {
    String runtimeArrayTypeName = Plasma::SpirVNameSettings::mRuntimeArrayTypeName;
    Array<Lightning::TemplateParameter> templateTypes;
    TemplateParameter& typeParam = templateTypes.PushBack();
    typeParam.Name = "Type";
    typeParam.Type = ConstantType::Type;
    builder.AddTemplateInstantiator(runtimeArrayTypeName, InstantiateRuntimeArray, templateTypes, nullptr);
  }

  // Create the geometry shader input/output types
  {
    Array<Lightning::TemplateParameter> templateTypes;
    TemplateParameter& typeParam = templateTypes.PushBack();
    typeParam.Name = "Type";
    typeParam.Type = ConstantType::Type;

    builder.AddTemplateInstantiator(
        "PointInput", InstantiateGeometryInput, templateTypes, (int*)spv::ExecutionModeInputPoints);
    builder.AddTemplateInstantiator(
        "LineInput", InstantiateGeometryInput, templateTypes, (int*)spv::ExecutionModeInputLines);
    builder.AddTemplateInstantiator(
        "TriangleInput", InstantiateGeometryInput, templateTypes, (int*)spv::ExecutionModeTriangles);

    builder.AddTemplateInstantiator(
        "PointOutput", InstantiateGeometryOutput, templateTypes, (int*)spv::ExecutionModeOutputPoints);
    builder.AddTemplateInstantiator(
        "LineOutput", InstantiateGeometryOutput, templateTypes, (int*)spv::ExecutionModeOutputLineStrip);
    builder.AddTemplateInstantiator(
        "TriangleOutput", InstantiateGeometryOutput, templateTypes, (int*)spv::ExecutionModeOutputTriangleStrip);
  }

  // Create the GeometryStreamMoverType
  {
    Array<Lightning::TemplateParameter> templateTypes;
    TemplateParameter& fromTypeParam = templateTypes.PushBack();
    fromTypeParam.Name = "FromType";
    fromTypeParam.Type = ConstantType::Type;
    TemplateParameter& toTypeParam = templateTypes.PushBack();
    toTypeParam.Name = "ToType";
    toTypeParam.Type = ConstantType::Type;

    builder.AddTemplateInstantiator("GeometryStreamMover", InstantiateGeometryStreamMover, templateTypes, nullptr);
  }
}

} // namespace Lightning

namespace Plasma
{

LightningDefineStaticLibrary(ShaderSettingsLibrary)
{
  builder.CreatableInScriptDefault = false;

  LightningInitializeType(TranslationErrorEvent);
  LightningInitializeType(ValidationErrorEvent);
  LightningInitializeType(SpecializationConstantEvent);
}

} // namespace Plasma
