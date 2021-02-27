// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningShaderIROp::LightningShaderIROp(OpType opType) : ILightningShaderIR(mStaticBaseType)
{
  mOpType = opType;
}

bool LightningShaderIROp::IsTerminator()
{
  bool isTerminator = (mOpType == OpType::OpReturn) || (mOpType == OpType::OpReturnValue) ||
                      (mOpType == OpType::OpBranch) || (mOpType == OpType::OpBranchConditional);
  return isTerminator;
}

bool LightningShaderIROp::IsResultPointerType()
{
  if (mResultType == nullptr)
    return false;
  return mResultType->IsPointerType();
}

LightningShaderIRType* LightningShaderIROp::GetValueType()
{
  return mResultType->GetValueType();
}

LightningShaderIRType* LightningShaderIROp::GetPointerType()
{
  return mResultType->GetPointerType();
}

BasicBlock::BasicBlock() : ILightningShaderIR(mStaticBaseType)
{
  mTerminatorOp = nullptr;
  mBlockType = BlockType::Direct;

  mMergePoint = nullptr;
  mContinuePoint = nullptr;
}

BasicBlock::~BasicBlock()
{
  DeleteObjectsIn(mLines);
  DeleteObjectsIn(mLocalVariables);
}

void BasicBlock::AddOp(ILightningShaderIR* op)
{
  mLines.PushBack(op);
}

EntryPointInfo::EntryPointInfo()
{
  mFragmentType = FragmentType::None;
  mEntryPointFn = nullptr;
  mGlobalsInitializerFunction = nullptr;
}

LightningShaderIRFunction::LightningShaderIRFunction() : ILightningShaderIR(mStaticBaseType)
{
  mMeta = nullptr;
}

LightningShaderIRFunction::~LightningShaderIRFunction()
{
  DeleteObjectsIn(mBlocks.All());
  // mMeta owned by the type's meta
}

LightningShaderIRType* LightningShaderIRFunction::GetReturnType()
{
  // The return type is always the first sub-type
  return mFunctionType->GetSubType(0);
}

LightningShaderIRType::LightningShaderIRType() : ILightningShaderIR(mStaticBaseType)
{
  mComponentType = nullptr;
  mLightningType = nullptr;
  mComponents = 1;
  mBaseType = ShaderIRTypeBaseType::Unknown;
  mPointerType = nullptr;
  mDereferenceType = nullptr;
  mMeta = nullptr;
  mEntryPoint = nullptr;
  mAutoDefaultConstructor = nullptr;
  mStorageClass = spv::StorageClassGeneric;
  mHasMainFunction = false;
}

LightningShaderIRType::~LightningShaderIRType()
{
  delete mEntryPoint;
}

LightningShaderIRFunction* LightningShaderIRType::CreateFunction(LightningShaderIRLibrary* library)
{
  LightningShaderIRFunction* function = new LightningShaderIRFunction();
  mFunctions.PushBack(function);
  library->mOwnedFunctions.PushBack(function);
  return function;
}

void LightningShaderIRType::AddMember(LightningShaderIRType* memberType, StringParam memberName)
{
  mParameters.PushBack(memberType);

  u32 index = static_cast<u32>(mParameters.Size()) - 1;
  mMemberNamesToIndex[memberName] = index;

  // Either use the use the lightning type name if possible, otherwise
  // this type doesn't actually exist as a lightning type
  // (e.g. RuntimeArray internal type) so use the spirv name instead.
  String memberTypeName = memberType->mName;
  if (memberType->mLightningType != nullptr)
    memberTypeName = memberType->mLightningType->ToString();

  mMemberKeysToIndex[ShaderFieldKey(memberName, memberTypeName)] = index;
}

String LightningShaderIRType::GetMemberName(u32 memberIndex)
{
  // Currently we only store names to indices but not the other way around.
  // For now just iterate through this map to find the member index.
  // A type shouldn't be too big and this is rarely done so this is acceptable
  // for now.
  for (auto range = mMemberNamesToIndex.All(); !range.Empty(); range.PopFront())
  {
    auto pair = range.Front();
    if (pair.second == memberIndex)
      return pair.first;
  }
  return String();
}

u32 LightningShaderIRType::FindMemberIndex(const String& memberName)
{
  return mMemberNamesToIndex.FindValue(memberName, cInvalidIndex);
}

LightningShaderIRType* LightningShaderIRType::GetSubType(u32 index) const
{
  bool supportsSubTypes = mBaseType == ShaderIRTypeBaseType::Struct || mBaseType == ShaderIRTypeBaseType::Function;

  ErrorIf(!supportsSubTypes,
          "Type '%s' does not support sub-types. The parameters on this type "
          "are not guaranteed to be types.",
          mName.c_str());

  ILightningShaderIR* param = mParameters[index];
  LightningShaderIRType* subType = param->As<LightningShaderIRType>();
  return subType;
}

u32 LightningShaderIRType::GetSubTypeCount()
{
  bool supportsSubTypes = mBaseType == ShaderIRTypeBaseType::Struct || mBaseType == ShaderIRTypeBaseType::Function;

  ErrorIf(!supportsSubTypes,
          "Type '%s' does not support sub-types. The parameters on this type "
          "are not guaranteed to be types.",
          mName.c_str());

  return static_cast<u32>(mParameters.Size());
}

u32 LightningShaderIRType::GetByteSize() const
{
  // Force everything to 4 bytes
  if (mBaseType == ShaderIRTypeBaseType::Bool || mBaseType == ShaderIRTypeBaseType::Int ||
      mBaseType == ShaderIRTypeBaseType::Float)
    return 4;
  // Don't pad matrices to weird byte alignments
  else if (mBaseType == ShaderIRTypeBaseType::Vector)
    return mComponents * mComponentType->GetByteSize();
  else if (mBaseType == ShaderIRTypeBaseType::Matrix)
  {
    return GetByteAlignment() * mComponents;
  }
  else if (mBaseType == ShaderIRTypeBaseType::FixedArray)
  {
    // The actual size of a fixed array is the number of elements times the
    // array stride. The array stride is the size of the contained item rounded
    // up based upon the max alignment
    LightningShaderIRType* elementType = mParameters[0]->As<LightningShaderIRType>();
    u32 elementByteSize = elementType->GetByteSize();
    u32 alignment = GetByteAlignment();
    u32 itemSize = GetSizeAfterAlignment(elementByteSize, alignment);
    return itemSize * mComponents;
  }
  else if (mBaseType == ShaderIRTypeBaseType::Struct)
  {
    // Each element has to actually be properly aligned in order to get the
    // correct size though otherwise this can drift very wildly.
    // For example struct { float A; vec3 B; float C; vec3 D; }
    // Is actually size 16 + 16 + 16 + 12 = 60 due to vec3 having
    // to be aligned on 16 byte boundaries.
    u32 size = 0;
    u32 parameterCount = static_cast<u32>(mParameters.Size());
    for(u32 i = 0; i < parameterCount; ++i)
    {
      LightningShaderIRType* memberType = GetSubType(i);
      u32 alignment = memberType->GetByteAlignment();
      u32 memberSize = memberType->GetByteSize();
      // Fix the current offset to be at the required alignment for this member.
      size = GetSizeAfterAlignment(size, alignment);
      // Then add the member size exactly as is (no padding
      // is required unless another element follows)
      size += memberSize;
    }
    // Vulkan Spec: A struct has a base alignment equal to the largest base
    // alignment of any of its memebers rounded up to a multiple of 16.
    size = GetSizeAfterAlignment(size, 16);
    return size;
  }
  Error("Unknown type for byte size");
  return 0;
}

u32 LightningShaderIRType::GetByteAlignment() const
{
  if (mBaseType == ShaderIRTypeBaseType::Bool || mBaseType == ShaderIRTypeBaseType::Int ||
      mBaseType == ShaderIRTypeBaseType::Float)
    return 4;
  else if (mBaseType == ShaderIRTypeBaseType::Vector)
  {
    u32 components = mComponents;
    // Real3 has to be aligned to 16 bytes per Vulkan spec.
    if (components == 3)
      components = 4;
    return components * mComponentType->GetByteAlignment();
  }
  else if (mBaseType == ShaderIRTypeBaseType::Matrix)
  {
    // Via opengl/dx matrix types are treated as an array of the vector types
    // where the vector types are padded up to vec4s. This happens for
    // efficiency reason (at least with uniform buffers).
    LightningShaderIRType* scalarType = mComponentType->mComponentType;
    return 4 * scalarType->GetByteAlignment();
  }
  else if (mBaseType == ShaderIRTypeBaseType::FixedArray)
  {
    // Via opengl/dx array of the vector types where the vector
    // types are padded up to vec4s. This happens for efficiency reason (at
    // least with uniform buffers).
    LightningShaderIRType* elementType = mParameters[0]->As<LightningShaderIRType>();
    if (elementType->mBaseType == ShaderIRTypeBaseType::Int || elementType->mBaseType == ShaderIRTypeBaseType::Float ||
        elementType->mBaseType == ShaderIRTypeBaseType::Bool || elementType->mBaseType == ShaderIRTypeBaseType::Vector)
      return 16;
    return elementType->GetByteAlignment();
  }
  else if (mBaseType == ShaderIRTypeBaseType::Struct)
  {
    // The alignment of a struct is the max alignment of all of its members
    u32 alignment = 0;
    u32 parameterCount = static_cast<u32>(mParameters.Size());
    for(u32 i = 0; i < parameterCount; ++i)
    {
      LightningShaderIRType* elementType = GetSubType(i);
      alignment = Math::Max(elementType->GetByteAlignment(), alignment);
    }
    return alignment;
  }
  // Ignore structs for now
  Error("Unknown type for byte size");
  return 0;
}

ShaderIRTypeBaseType::Enum LightningShaderIRType::GetBasePrimitiveType() const
{
  if (mBaseType == ShaderIRTypeBaseType::Bool || mBaseType == ShaderIRTypeBaseType::Int ||
      mBaseType == ShaderIRTypeBaseType::Float)
    return mBaseType;
  else if (mBaseType == ShaderIRTypeBaseType::Vector || mBaseType == ShaderIRTypeBaseType::Matrix)
    return mComponentType->GetBasePrimitiveType();
  return mBaseType;
}

ShaderIRAttribute* LightningShaderIRType::FindFirstAttribute(StringParam attributeName) const
{
  if (mMeta == nullptr)
    return nullptr;
  return mMeta->mAttributes.FindFirstAttribute(attributeName);
}

bool LightningShaderIRType::IsPointerType()
{
  return mBaseType == ShaderIRTypeBaseType::Pointer;
}

bool LightningShaderIRType::IsGlobalType() const
{
  // Find the storage class attribute
  ShaderIRAttribute* storageClassAttribute = FindFirstAttribute(SpirVNameSettings::mStorageClassAttribute);
  if (storageClassAttribute == nullptr)
    return false;

  // Check the value of the storage class
  spv::StorageClass storageClass = (spv::StorageClass)storageClassAttribute->mParameters[0].GetIntValue();
  if (storageClass == spv::StorageClass::StorageClassUniformConstant || storageClass == spv::StorageClassUniform ||
      storageClass == spv::StorageClassStorageBuffer)
    return true;
  return false;
}

LightningShaderIRType* LightningShaderIRType::GetValueType()
{
  if (mBaseType == ShaderIRTypeBaseType::Pointer)
    return mDereferenceType;
  return this;
}

LightningShaderIRType* LightningShaderIRType::GetPointerType()
{
  if (mBaseType == ShaderIRTypeBaseType::Pointer)
    return this;
  return mPointerType;
}

LightningShaderIRType* GetComponentType(LightningShaderIRType* compositeType)
{
  bool isMathType = compositeType->mBaseType == ShaderIRTypeBaseType::Int ||
                    compositeType->mBaseType == ShaderIRTypeBaseType::Float ||
                    compositeType->mBaseType == ShaderIRTypeBaseType::Vector ||
                    compositeType->mBaseType == ShaderIRTypeBaseType::Matrix;
  ErrorIf(!isMathType,
          "Invalid type to find component type on. Only math types "
          "(scalars/vectors/matrices) are allowed");
  return compositeType->mComponentType;
}

bool IsScalarType(LightningShaderIRType* compositeType)
{
  bool isScalarType = compositeType->mBaseType == ShaderIRTypeBaseType::Int ||
                      compositeType->mBaseType == ShaderIRTypeBaseType::Float ||
                      compositeType->mBaseType == ShaderIRTypeBaseType::Bool;
  return isScalarType;
}

LightningShaderIRType* GetImageTypeFromSampledImage(LightningShaderIRType* samplerType)
{
  if (samplerType->mBaseType != ShaderIRTypeBaseType::SampledImage)
  {
    Error("Given type was expected to be a SampledImage but was '%s'", samplerType->mName.c_str());
    return nullptr;
  }

  ILightningShaderIR* imageArg = samplerType->mParameters[0];
  LightningShaderIRType* imageType = imageArg->As<LightningShaderIRType>();
  ErrorIf(imageType->mBaseType != ShaderIRTypeBaseType::Image, "Sampler's image type parameter isn't an image");
  return imageType;
}

u32 GetStride(LightningShaderIRType* type, float baseAlignment)
{
  u32 typeSize = type->GetByteSize();
  u32 stride = static_cast<u32>(baseAlignment * Math::Ceil(typeSize / baseAlignment));
  return stride;
}

  u32 GetSizeAfterAlignment(u32 size, u32 baseAlignment)
{
  // Get the remainder to add
  u32 remainder = baseAlignment - (size % baseAlignment);
  // Mod with the required alignment to get offset 0 when needed
  u32 alignmentOffset = remainder % baseAlignment;
  size += alignmentOffset;
  return size;
}

TemplateTypeKey GenerateTemplateTypeKey(Lightning::BoundType* lightningType)
{
  StringBuilder builder;
  builder.Append(lightningType->TemplateBaseName);
  builder.Append("[");

  size_t size = lightningType->TemplateArguments.Size();
  for (size_t i = 0; i < size; ++i)
  {
    Lightning::Constant& arg = lightningType->TemplateArguments[i];
    builder.Append(Lightning::ConstantType::Names[arg.Type]);
    if (i != size - 1)
      builder.Append(",");
  }
  builder.Append("]");
  return builder.ToString();
}

String GenerateSpirVPropertyName(StringParam fieldName, StringParam ownerType)
{
  return BuildString(fieldName, "_", ownerType);
}

String GenerateSpirVPropertyName(StringParam fieldName, LightningShaderIRType* ownerType)
{
  return GenerateSpirVPropertyName(fieldName, ownerType->mName);
}

Array<String> GetOpcodeNames()
{
  // Get all opcodes from spirv (hardcode the language for now...)
  spv_opcodes_t opCodeNames;
  spvGetOpcodeNames(SPV_ENV_UNIVERSAL_1_2, &opCodeNames);

  // Convert each opcode to string
  Array<String> results;
  for (size_t i = 0; i < opCodeNames.count; ++i)
  {
    results.PushBack(opCodeNames.opcode_names[i]);
  }
  spvDestroyOpcodeNames(&opCodeNames);

  return results;
}

} // namespace Plasma
