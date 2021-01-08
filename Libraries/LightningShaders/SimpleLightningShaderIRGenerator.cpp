// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void SimplifiedShaderReflectionData::CreateReflectionData(LightningShaderIRLibrary* shaderLibrary,
                                                          ShaderStageDescription& stageDef,
                                                          Array<PassResultRef>& passResults)
{
  if (passResults.Empty())
    return;

  CreateUniformReflectionData(shaderLibrary, stageDef, passResults);
  CreateSamplerAndImageReflectionData(shaderLibrary, stageDef, passResults);
  CreateSimpleOpaqueTypeReflectionData(shaderLibrary, stageDef, passResults);
}

const ShaderResourceReflectionData* SimplifiedShaderReflectionData::FindUniformReflectionData(const LightningShaderIRType* fragmentType,
                                                                                        StringParam propertyName) const
{
  String fragmentName = fragmentType->mMeta->mLightningName;
  // Find the fragment's lookup data
  FragmentLookup* fragmentLookup = mFragmentLookup.FindPointer(fragmentName);
  if (fragmentLookup == nullptr)
    return nullptr;

  // Find the uniform data for the given property name
  UniformReflectionData* uniformData = fragmentLookup->mMaterialUniforms.FindPointer(propertyName);
  if (uniformData == nullptr)
    return nullptr;

  // Use that to index into the active uniform buffers
  size_t uniformBufferIndex = uniformData->mBufferIndex;
  if (uniformBufferIndex >= mReflection.mUniforms.Size())
    return nullptr;

  // Finally, index into the target uniform buffer to get the member data
  const ShaderStageResource& uniformBuffer = mReflection.mUniforms[uniformBufferIndex];
  size_t memberIndex = uniformData->mMemberIndex;
  if (memberIndex >= uniformBuffer.mMembers.Size())
    return nullptr;

  return &uniformBuffer.mMembers[memberIndex];
}

void SimplifiedShaderReflectionData::FindSampledImageReflectionData(const LightningShaderIRType* fragmentType,
                                                                    StringParam propertyName,
                                                                    Array<const ShaderResourceReflectionData*>& results) const
{
  FragmentLookup* fragmentLookup = mFragmentLookup.FindPointer(fragmentType->mMeta->mLightningName);
  if (fragmentLookup == nullptr)
    return;

  PopulateSamplerAndImageData(fragmentLookup->mSampledImages, propertyName, results);
}

void SimplifiedShaderReflectionData::FindImageReflectionData(const LightningShaderIRType* fragmentType,
                                                             StringParam propertyName,
                                                             Array<const ShaderResourceReflectionData*>& results) const
{
  FragmentLookup* fragmentLookup = mFragmentLookup.FindPointer(fragmentType->mMeta->mLightningName);
  if (fragmentLookup == nullptr)
    return;

  PopulateSamplerAndImageData(fragmentLookup->mImages, propertyName, results);
}

void SimplifiedShaderReflectionData::FindSamplerReflectionData(const LightningShaderIRType* fragmentType,
                                                               StringParam propertyName,
                                                               Array<const ShaderResourceReflectionData*>& results) const
{
  FragmentLookup* fragmentLookup = mFragmentLookup.FindPointer(fragmentType->mMeta->mLightningName);
  if (fragmentLookup == nullptr)
    return;

  PopulateSamplerAndImageData(fragmentLookup->mSamplers, propertyName, results);
}

const ShaderResourceReflectionData* SimplifiedShaderReflectionData::FindStorageImage(const LightningShaderIRType* fragmentType,
                                                                               StringParam propertyName) const
{
  FragmentLookup* fragmentLookup = mFragmentLookup.FindPointer(fragmentType->mMeta->mLightningName);
  if (fragmentLookup == nullptr)
    return nullptr;

  // Find the uniform data for the given property name
  StorageImageRemappingData* storageImageData = fragmentLookup->mStorageImages.FindPointer(propertyName);
  if (storageImageData == nullptr)
    return nullptr;

  return &mReflection.mStorageImages[storageImageData->mIndex].mReflectionData;
}

const ShaderResourceReflectionData* SimplifiedShaderReflectionData::FindStructedStorageBuffer(const LightningShaderIRType* fragmentType,
                                                                                        StringParam propertyName) const
{
  FragmentLookup* fragmentLookup = mFragmentLookup.FindPointer(fragmentType->mMeta->mLightningName);
  if (fragmentLookup == nullptr)
    return nullptr;

  // Find the uniform data for the given property name
  StructuredStorageBufferRemappingData* ssboData = fragmentLookup->mStructedStorageBuffers.FindPointer(propertyName);
  if (ssboData == nullptr)
    return nullptr;

  return &mReflection.mStructedStorageBuffers[ssboData->mIndex].mReflectionData;
}

void SimplifiedShaderReflectionData::CreateUniformReflectionData(LightningShaderIRLibrary* shaderLibrary,
                                                                 ShaderStageDescription& stageDef,
                                                                 Array<PassResultRef>& passResults)
{
  HashMap<String, UniformReflectionData> memberRemappings;
  HashMap<String, SimpleResourceRemappingData> bufferRenames;
  ShaderStageInterfaceReflection& firstStageData = passResults[0]->mReflectionData;
  ShaderStageInterfaceReflection* lastStageData = &passResults[passResults.Size() -1]->mReflectionData;

  // Walk all uniforms in the first stage, building a mapping of the
  // buffer names and the member names within each buffer
  for (size_t i = 0; i < firstStageData.mUniforms.Size(); ++i)
  {
    ShaderStageResource& uniformBuffer = firstStageData.mUniforms[i];
    String resourceName = uniformBuffer.mReflectionData.mInstanceName;
    // Make a map of the buffer to itself for now (if there's only
    // 1 pipeline stage then this should still be true)
    SimpleResourceRemappingData& resourceRemapping = bufferRenames[resourceName];
    resourceRemapping.mName = resourceName;
    resourceRemapping.mIndex = i;
    resourceRemapping.mActive = true;

    // Map the buffer and index of each member variable
    AutoDeclare(range, uniformBuffer.mLookupMap.All());
    for (; !range.Empty(); range.PopFront())
    {
      AutoDeclare(pair, range.Front());
      String memberName = pair.first;
      size_t memberIndex = pair.second;
      ShaderResourceReflectionData& memberReflection = uniformBuffer.mMembers[memberIndex];
      UniformReflectionData& memberRemappingData = memberRemappings[memberName];
      memberRemappingData.mBufferIndex = i;
      memberRemappingData.mMemberIndex = memberIndex;
    }
  }

  // Figure out how uniform buffers changed between pipeline stages. Currently
  // this only supports changes at the top level of a buffer (buffer renames or
  // buffers disappearing) and doesn't support member changes (member
  // re-orderings, buffer splits, etc...)
  for (size_t pipelineIndex = 1; pipelineIndex < passResults.Size(); ++pipelineIndex)
  {
    ShaderStageInterfaceReflection& reflectionData = passResults[pipelineIndex]->mReflectionData;

    // For each uniform buffer in the current stage, build a map of its name to
    // index
    HashMap<String, size_t> indexMap;
    for (size_t i = 0; i < reflectionData.mUniforms.Size(); ++i)
    {
      ShaderStageResource& uniformBuffer = reflectionData.mUniforms[i];
      String bufferName = uniformBuffer.mReflectionData.mInstanceName;
      indexMap[bufferName] = i;
    }

    // We need to walk all remapped buffer names and determine if
    // an old buffer exists in the new stage and if so at what index
    AutoDeclare(range, bufferRenames.All());
    for (; !range.Empty(); range.PopFront())
    {
      AutoDeclare(pair, range.Front());
      String firstName = pair.first;
      SimpleResourceRemappingData& resourceRemapping = pair.second;
      size_t* newIndex = indexMap.FindPointer(resourceRemapping.mName);

      // The buffer doesn't exist, mark it as inactive. This buffer
      // could have disappeared for a number of reasons, such as optimization.
      if (newIndex == nullptr || resourceRemapping.mActive == false)
      {
        bufferRenames[firstName].mActive = false;
        continue;
      }

      // The buffer does exist, update its name and index
      String newName = reflectionData.mUniforms[*newIndex].mReflectionData.mInstanceName;
      resourceRemapping.mName = newName;
      resourceRemapping.mIndex = *newIndex;
    }
  }

  // Now we need to map fragment properties to actual uniform buffer memory.
  // To do this we walk all fragments, mapping a property name to the first
  // uniform buffer index. With this index we can jump to the final uniform
  // buffer index the we pre-calculated above.
  AutoDeclare(fragRange, stageDef.mFragmentDescriptions->All());
  for (; !fragRange.Empty(); fragRange.PopFront())
  {
    LightningShaderIRCompositor::ShaderFragmentDescription* fragDesc = fragRange.Front().second;

    FragmentLookup& fragLookup = mFragmentLookup[fragDesc->mMeta->mLightningName];
    AutoDeclare(propRange, fragDesc->mFieldDescriptions.All());
    for (; !propRange.Empty(); propRange.PopFront())
    {
      LightningShaderIRCompositor::ShaderFieldDescription& fieldDesc = propRange.Front().second;

      // Find the remapping data for this fragment's property
      // (make sure to use the property name which is the compositor's result
      // name)
      UniformReflectionData* memberRemappingData = memberRemappings.FindPointer(fieldDesc.mFieldPropertyName);
      if (memberRemappingData == nullptr)
        continue;

      // Now we have the first pipeline stage's uniform buffer index for this
      // property.
      size_t firstStageBufferIndex = memberRemappingData->mBufferIndex;

      // We now need to trace this uniform buffer to the final stage to see
      // where this property ends up
      String spirvBufferName = firstStageData.mUniforms[firstStageBufferIndex].mReflectionData.mInstanceName;
      SimpleResourceRemappingData* resourceRemapping = bufferRenames.FindPointer(spirvBufferName);
      // The property didn't make it to a final uniform buffer (or one that's
      // inactive)
      if (resourceRemapping == nullptr || !resourceRemapping->mActive)
        continue;

      // Store for the current fragment where the given property ended up
      String originalFieldName = fieldDesc.mMeta->mLightningName;
      UniformReflectionData& uniformData = fragLookup.mMaterialUniforms[originalFieldName];
      uniformData.mBufferIndex = resourceRemapping->mIndex;
      uniformData.mMemberIndex = memberRemappingData->mMemberIndex;
    }
  }

  // Store the final stage's reflection data (where we actually find member
  // offsets from)
  mReflection = *lastStageData;
}

void SimplifiedShaderReflectionData::CreateSamplerAndImageReflectionData(LightningShaderIRLibrary* shaderLibrary,
                                                                         ShaderStageDescription& stageDef,
                                                                         Array<PassResultRef>& passResults)
{
  ShaderStageInterfaceReflection& firstPassData = passResults[0]->mReflectionData;
  ShaderStageInterfaceReflection& lastPassData = passResults[passResults.Size() - 1]->mReflectionData;

  // Copy final data results
  mReflection.mImages = lastPassData.mImages;
  mReflection.mSampledImages = lastPassData.mSampledImages;
  mReflection.mSamplers = lastPassData.mSamplers;

  // Map final buffer names to indices
  NameToIndexMap samplerIndices;
  NameToIndexMap imageIndices;
  NameToIndexMap sampledImageIndices;
  for (size_t i = 0; i < lastPassData.mSamplers.Size(); ++i)
    samplerIndices[lastPassData.mSamplers[i].mReflectionData.mInstanceName] = i;
  for (size_t i = 0; i < lastPassData.mImages.Size(); ++i)
    imageIndices[lastPassData.mImages[i].mReflectionData.mInstanceName] = i;
  for (size_t i = 0; i < lastPassData.mSampledImages.Size(); ++i)
    sampledImageIndices[lastPassData.mSampledImages[i].mReflectionData.mInstanceName] = i;

  // Walk all fragments looking for sampled image like properties
  AutoDeclare(fragRange, stageDef.mFragmentDescriptions->All());
  for (; !fragRange.Empty(); fragRange.PopFront())
  {
    LightningShaderIRCompositor::ShaderFragmentDescription* fragDesc = fragRange.Front().second;

    FragmentLookup& fragLookup = mFragmentLookup[fragDesc->mMeta->mLightningName];
    AutoDeclare(propRange, fragDesc->mFieldDescriptions.All());
    for (; !propRange.Empty(); propRange.PopFront())
    {
      LightningShaderIRCompositor::ShaderFieldDescription& fieldDesc = propRange.Front().second;
      String fieldName = fieldDesc.mMeta->mLightningName;
      String propertyName = fieldDesc.mFieldPropertyName;
      LightningShaderIRType* shaderType = shaderLibrary->FindType(fieldDesc.mMeta->mLightningType);
      // This is a sampled image. Walk the sampled image properties
      if (shaderType->mBaseType == ShaderIRTypeBaseType::SampledImage)
      {
        // Get the first stage's data for the sampled image
        SampledImageRemappings* resourceMappings = firstPassData.mSampledImageRemappings.FindPointer(propertyName);
        if (resourceMappings == nullptr)
          continue;

        // Walk the pipeline stages to find the final multi-to-multi mappings.
        SampledImageRemappings results;
        RecursivelyBuildSamplerAndImageMappings(passResults, 1, *resourceMappings, results);
        // Build the final name to index mapping
        BuildFinalSampledImageMappings(
            &results, samplerIndices, imageIndices, sampledImageIndices, fragLookup.mSampledImages[fieldName]);
      }
      else if (shaderType->mBaseType == ShaderIRTypeBaseType::Sampler)
      {
        SampledImageRemappings* resourceMappings = firstPassData.mSamplerRemappings.FindPointer(propertyName);
        if (resourceMappings == nullptr)
          continue;

        SampledImageRemappings results;
        RecursivelyBuildSamplerAndImageMappings(passResults, 1, *resourceMappings, results);
        BuildFinalSampledImageMappings(
            &results, samplerIndices, imageIndices, sampledImageIndices, fragLookup.mSamplers[fieldName]);
      }
      else if (shaderType->mBaseType == ShaderIRTypeBaseType::Image)
      {
        // Make sure this image type isn't a storage image
        LightningShaderIRImageType imageType;
        if (!imageType.Load(shaderType) || imageType.IsStorageImage())
          continue;

        SampledImageRemappings* resourceMappings = firstPassData.mImageRemappings.FindPointer(propertyName);
        if (resourceMappings == nullptr)
          continue;

        SampledImageRemappings results;
        RecursivelyBuildSamplerAndImageMappings(passResults, 1, *resourceMappings, results);
        BuildFinalSampledImageMappings(
            &results, samplerIndices, imageIndices, sampledImageIndices, fragLookup.mImages[fieldName]);
      }
    }
  }
}

void SimplifiedShaderReflectionData::RecursivelyBuildSamplerAndImageMappings(Array<PassResultRef>& passResults,
                                                                             size_t passIndex,
                                                                             SampledImageRemappings& inputMappings,
                                                                             SampledImageRemappings& outputMappings)
{
  // Base case. Just copy the inputs to the outputs.
  if (passIndex >= passResults.Size())
  {
    outputMappings = inputMappings;
    return;
  }

  // Walk all image, sampler, and sampled image remappings recursively.
  // Technically an image could merge into a sampled image and then split later
  // so we have to walk everything.
  ShaderStageInterfaceReflection& reflectionData = passResults[passIndex]->mReflectionData;

  typedef Array<String>::range rangeType;
  for (rangeType range = inputMappings.mImageRemappings.All(); !range.Empty(); range.PopFront())
  {
    String name = range.Front();
    SampledImageRemappings& sourceMappings = reflectionData.mImageRemappings[name];
    SampledImageRemappings sourceResults;
    RecursivelyBuildSamplerAndImageMappings(passResults, passIndex + 1, sourceMappings, sourceResults);
    MergeRemappings(outputMappings, sourceResults);
  }

  for (rangeType range = inputMappings.mSamplerRemappings.All(); !range.Empty(); range.PopFront())
  {
    String name = range.Front();
    SampledImageRemappings& sourceMappings = reflectionData.mSamplerRemappings[name];
    SampledImageRemappings sourceResults;
    RecursivelyBuildSamplerAndImageMappings(passResults, passIndex + 1, sourceMappings, sourceResults);
    MergeRemappings(outputMappings, sourceResults);
  }

  for (rangeType range = inputMappings.mSampledImageRemappings.All(); !range.Empty(); range.PopFront())
  {
    String name = range.Front();
    SampledImageRemappings& sourceMappings = reflectionData.mSampledImageRemappings[name];
    SampledImageRemappings sourceResults;
    RecursivelyBuildSamplerAndImageMappings(passResults, passIndex + 1, sourceMappings, sourceResults);
    MergeRemappings(outputMappings, sourceResults);
  }
}

void SimplifiedShaderReflectionData::MergeRemappings(SampledImageRemappings& dest, SampledImageRemappings& source)
{
  dest.mImageRemappings.Insert(dest.mImageRemappings.End(), source.mImageRemappings.All());
  dest.mSamplerRemappings.Insert(dest.mSamplerRemappings.End(), source.mSamplerRemappings.All());
  dest.mSampledImageRemappings.Insert(dest.mSampledImageRemappings.End(), source.mSampledImageRemappings.All());
}

void SimplifiedShaderReflectionData::BuildFinalSampledImageMappings(SampledImageRemappings* resourceMappings,
                                                                    NameToIndexMap& samplerIndices,
                                                                    NameToIndexMap& imageIndices,
                                                                    NameToIndexMap& sampledImageIndices,
                                                                    SampledImageRemappingData& results)
{
  if (resourceMappings == nullptr)
    return;

  // Convert the final resource mapping names to indices based upon the given
  // name to index maps. Do this for image, samplers, and sampled images.

  for (size_t i = 0; i < resourceMappings->mImageRemappings.Size(); ++i)
  {
    String name = resourceMappings->mImageRemappings[i];
    size_t* resultIndex = imageIndices.FindPointer(name);
    if (resultIndex != nullptr)
      results.mImageIds.PushBack(*resultIndex);
  }
  for (size_t i = 0; i < resourceMappings->mSamplerRemappings.Size(); ++i)
  {
    String name = resourceMappings->mSamplerRemappings[i];
    size_t* resultIndex = samplerIndices.FindPointer(name);
    if (resultIndex != nullptr)
      results.mSamplerIds.PushBack(*resultIndex);
  }
  for (size_t i = 0; i < resourceMappings->mSampledImageRemappings.Size(); ++i)
  {
    String name = resourceMappings->mSampledImageRemappings[i];
    size_t* resultIndex = sampledImageIndices.FindPointer(name);
    if (resultIndex != nullptr)
      results.mSampledImageIds.PushBack(*resultIndex);
  }
}

void SimplifiedShaderReflectionData::PopulateSamplerAndImageData(HashMap<String, SampledImageRemappingData>& searchMap,
                                                                 StringParam propertyName,
                                                                 Array<const ShaderResourceReflectionData*>& results) const
{
  // Find the property in the given search map
  SampledImageRemappingData* remapData = searchMap.FindPointer(propertyName);
  if (remapData == nullptr)
    return;

  // Copy all of the image, sampler, and sampled image ids. Technically not all
  // of this is relevant (e.g. an image can't produce a sampler) but for code
  // re-use this is much cleaner.
  for (size_t i = 0; i < remapData->mImageIds.Size(); ++i)
  {
    size_t imageIndex = remapData->mImageIds[i];
    results.PushBack(&mReflection.mImages[imageIndex].mReflectionData);
  }
  for (size_t i = 0; i < remapData->mSamplerIds.Size(); ++i)
  {
    size_t samplerIndex = remapData->mSamplerIds[i];
    results.PushBack(&mReflection.mSamplers[samplerIndex].mReflectionData);
  }
  for (size_t i = 0; i < remapData->mSampledImageIds.Size(); ++i)
  {
    size_t index = remapData->mSampledImageIds[i];
    results.PushBack(&mReflection.mSampledImages[index].mReflectionData);
  }
}

void SimplifiedShaderReflectionData::CreateSimpleOpaqueTypeReflectionData(LightningShaderIRLibrary* shaderLibrary,
                                                                          ShaderStageDescription& stageDef,
                                                                          Array<PassResultRef>& passResults)
{
  // @JoshD: Currently hardcode this to only look at the last stage.
  // Currently nothing renames variables between stages so this is reasonable.
  ShaderStageInterfaceReflection* lastStageData = &passResults.Back()->mReflectionData;
  Array<ShaderStageResource>& storageImages = lastStageData->mStorageImages;
  Array<ShaderStageResource>& storageBuffers = lastStageData->mStructedStorageBuffers;
  HashMap<String, StructuredStorageBufferRemappingData> storageBufferMappings;
  HashMap<String, StorageImageRemappingData> storageImageMappings;

  // Map all structured storage buffers by name to their index in the final
  // stage
  for (size_t i = 0; i < storageBuffers.Size(); ++i)
  {
    ShaderStageResource& storageBufferResource = storageBuffers[i];
    String resourceName = storageBufferResource.mReflectionData.mInstanceName;
    storageBufferMappings[resourceName].mIndex = i;
  }
  // Map all storage images by name to their index in the final stage
  for (size_t i = 0; i < storageImages.Size(); ++i)
  {
    ShaderStageResource& resource = storageImages[i];
    String resourceName = resource.mReflectionData.mInstanceName;
    storageImageMappings[resourceName].mIndex = i;
  }

  // Walk all fragments looking for properties to map to simple opaque types
  AutoDeclare(fragRange, stageDef.mFragmentDescriptions->All());
  for (; !fragRange.Empty(); fragRange.PopFront())
  {
    LightningShaderIRCompositor::ShaderFragmentDescription* fragDesc = fragRange.Front().second;

    FragmentLookup& fragLookup = mFragmentLookup[fragDesc->mMeta->mLightningName];
    AutoDeclare(propRange, fragDesc->mFieldDescriptions.All());
    for (; !propRange.Empty(); propRange.PopFront())
    {
      LightningShaderIRCompositor::ShaderFieldDescription& fieldDesc = propRange.Front().second;
      String fieldName = fieldDesc.mMeta->mLightningName;
      String propertyName = fieldDesc.mFieldPropertyName;
      LightningShaderIRType* shaderType = shaderLibrary->FindType(fieldDesc.mMeta->mLightningType);

      // Check if this is a runtime array
      LightningShaderIRRuntimeArrayType runtimeArrayType;
      if (runtimeArrayType.Load(shaderType))
      {
        StructuredStorageBufferRemappingData* remappingData = storageBufferMappings.FindPointer(propertyName);
        // Store the remapped data if it exists
        if (remappingData != nullptr)
          fragLookup.mStructedStorageBuffers[fieldName] = *remappingData;
        else
        {
          remappingData = storageBufferMappings.FindPointer(fieldName);
          if(remappingData != nullptr)
            fragLookup.mStructedStorageBuffers[fieldName] = *remappingData;
        }
        continue;
      }
      // Check if this is a storage image
      LightningShaderIRImageType imageType;
      if (imageType.Load(shaderType) && imageType.IsStorageImage())
      {
        StorageImageRemappingData* remappingData = storageImageMappings.FindPointer(propertyName);
        // Store the remapped data if it exists
        if (remappingData != nullptr)
          fragLookup.mStorageImages[fieldName] = *remappingData;
        continue;
      }
    }
  }
}

SimpleLightningShaderIRGenerator::SimpleLightningShaderIRGenerator(FrontEndTranslatorType* frontEndTranslator) :
    mFragmentProject("Fragments"),
    mShaderProject("Shaders")
{
  SetupEventConnections();

  SpirVNameSettings nameSettings;
  LoadNameSettings(nameSettings);
  LightningShaderSpirVSettings* settings = CreateUnitTestSettings(nameSettings);

  Initialize(frontEndTranslator, settings);
}

SimpleLightningShaderIRGenerator::SimpleLightningShaderIRGenerator(FrontEndTranslatorType* frontEndTranslator,
                                                           LightningShaderSpirVSettings* settings) :
    mFragmentProject("Fragments"),
    mShaderProject("Shaders")
{
  SetupEventConnections();
  Initialize(frontEndTranslator, settings);
}

SimpleLightningShaderIRGenerator::~SimpleLightningShaderIRGenerator()
{
  delete mFrontEndTranslator;
  ShaderIntrinsicsStaticLightningLibrary::Destroy();
  Lightning::ShaderIntrinsicsLibrary::Destroy();
  LightningShaderIRCore::Destroy();
}

void SimpleLightningShaderIRGenerator::SetupDependencies(StringParam extensionsDirectoryPath)
{
  // Create the core library and parse it
  LightningShaderIRCore::InitializeInstance();
  LightningShaderIRCore& coreLibrary = LightningShaderIRCore::GetInstance();
  coreLibrary.Parse(mFrontEndTranslator);
  mCoreLibrary = coreLibrary.GetLibrary();

  Lightning::ShaderIntrinsicsLibrary::InitializeInstance();

  // Create the intrinsics library and parse it
  ShaderIntrinsicsStaticLightningLibrary::InitializeInstance();
  ShaderIntrinsicsStaticLightningLibrary& shaderIntrinsics = ShaderIntrinsicsStaticLightningLibrary::GetInstance();
  shaderIntrinsics.Parse(mFrontEndTranslator);
  mShaderIntrinsicsLibrary = shaderIntrinsics.GetLibrary();

  LightningShaderIRModuleRef dependencies = new LightningShaderIRModule();
  dependencies->PushBack(mShaderIntrinsicsLibrary);

  // Load all of the extension lightning fragments
  LightningShaderIRProject extensionProject("Extensions");
  EventConnect(&extensionProject, Events::TranslationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);
  EventConnect(&extensionProject, Lightning::Events::CompilationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);
  EventConnect(&extensionProject, Events::ValidationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);

  RecursivelyLoadDirectory(extensionsDirectoryPath, extensionProject);

  mExtensionsLibraryRef = extensionProject.CompileAndTranslate(dependencies, mFrontEndTranslator);
}

void SimpleLightningShaderIRGenerator::SetPipeline(ShaderPipelineDescription* pipeline)
{
  mPipeline = pipeline;
}

void SimpleLightningShaderIRGenerator::ClearAll()
{
  ClearFragmentsProjectAndLibrary();
}

void SimpleLightningShaderIRGenerator::AddFragmentCode(StringParam fragmentCode, StringParam fileName, void* userData)
{
  mFragmentProject.AddCodeFromString(fragmentCode, fileName, userData);
}

bool SimpleLightningShaderIRGenerator::CompileAndTranslateFragments()
{
  // Make sure that our dependencies have been built
  if (mExtensionsLibraryRef == nullptr)
  {
    Error("Cannot compile fragments before dependency libraries are setup. "
          "Call SetupDependencies first.");
    return false;
  }

  // Create the dependencies for the fragment library
  LightningShaderIRModuleRef fragmentDependencies = new LightningShaderIRModule();
  fragmentDependencies->PushBack(mShaderIntrinsicsLibrary);
  fragmentDependencies->PushBack(mExtensionsLibraryRef);

  // Compile and translate the fragments project into a library
  auto newLibrary= mFragmentProject.CompileAndTranslate(fragmentDependencies, mFrontEndTranslator);
  if(newLibrary == nullptr)
    return false;
  mFragmentLibraryRef = newLibrary;
  return mFragmentLibraryRef != nullptr;
}

void SimpleLightningShaderIRGenerator::ClearFragmentsProjectAndLibrary()
{
  mFragmentProject.Clear();
  mFragmentLibraryRef = nullptr;
}

bool SimpleLightningShaderIRGenerator::ComposeShader(LightningShaderIRCompositor::ShaderDefinition& shaderDef,
                                                 ShaderCapabilities& capabilities)
{
  LightningShaderIRCompositor compositor;
  EventConnect(&compositor, Events::TranslationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);
  EventConnect(&compositor, Lightning::Events::CompilationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);
  EventConnect(&compositor, Events::ValidationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);

  bool success = compositor.Composite(shaderDef, capabilities, mSettings);

  mShaderDefinitionMap[shaderDef.mShaderName] = shaderDef;

  return success;
}

  bool SimpleLightningShaderIRGenerator::ComposeComputeShader(LightningShaderIRCompositor::ShaderDefinition& shaderDef, ShaderCapabilities& capabilities, LightningShaderIRCompositor::ComputeShaderProperties* computeProperties)
{
  LightningShaderIRCompositor compositor;
  EventConnect(&compositor, Events::TranslationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);
  EventConnect(&compositor, Lightning::Events::CompilationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);
  EventConnect(&compositor, Events::ValidationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);

  bool success = compositor.CompositeCompute(shaderDef, computeProperties, capabilities, mSettings);

  mShaderDefinitionMap[shaderDef.mShaderName] = shaderDef;

  return success;
}

void SimpleLightningShaderIRGenerator::AddShaderCode(StringParam shaderCode, StringParam fileName, void* userData)
{
  mShaderProject.AddCodeFromString(shaderCode, fileName, userData);
}

bool SimpleLightningShaderIRGenerator::CompileAndTranslateShaders()
{
  // Make sure that our dependencies have been built
  if (mFragmentLibraryRef == nullptr)
  {
    Error("Cannot compile shader before fragments successfully compile.");
    return false;
  }

  // Create the dependencies for the fragment library
  LightningShaderIRModuleRef dependencies = new LightningShaderIRModule();
  dependencies->PushBack(mFragmentLibraryRef);

  // Compile and translate the fragments project into a library
  auto newLibrary = mShaderProject.CompileAndTranslate(dependencies, mFrontEndTranslator);
  if(newLibrary == nullptr)
    return false;
  mShaderLibraryRef = newLibrary;
  return mShaderLibraryRef != nullptr;
}

void SimpleLightningShaderIRGenerator::ClearShadersProjectAndLibrary()
{
  mShaderProject.Clear();
  mShaderLibraryRef = nullptr;
  mShaderDefinitionMap.Clear();
}

bool SimpleLightningShaderIRGenerator::CompilePipeline()
{
  ShaderPipelineDescription* pipeline = mPipeline;
  if (pipeline == nullptr)
    return false;

  return CompilePipeline(*pipeline);
}

bool SimpleLightningShaderIRGenerator::CompilePipeline(ShaderPipelineDescription& pipeline)
{
  // Shader library needs to be compiled to try and build shaders from it.
  if (mShaderLibraryRef == nullptr)
    return false;

  bool result = true;
  // Walk all shader definitions stored and compile each shader stage
  AutoDeclare(range, mShaderDefinitionMap.All());
  for (; !range.Empty(); range.PopFront())
  {
    String shaderName = range.Front().first;
    LightningShaderIRCompositor::ShaderDefinition& shaderDef = range.Front().second;

    for (size_t i = 0; i < FragmentType::Size; ++i)
    {
      LightningShaderIRCompositor::ShaderStageDescription& stageDef = shaderDef.mResults[i];
      // This stage wasn't active since it produced no code. Skip it.
      if (stageDef.mShaderCode.Empty())
        continue;

      result &= CompilePipeline(stageDef, pipeline);
    }
  }

  return result;
}

LightningShaderIRType* SimpleLightningShaderIRGenerator::FindFragmentType(StringParam typeName)
{
  LightningShaderIRLibrary* fragmentLibrary = mFragmentLibraryRef;
  if (fragmentLibrary == nullptr)
    return nullptr;

  return fragmentLibrary->FindType(typeName, false);
}

LightningShaderIRType* SimpleLightningShaderIRGenerator::FindShaderType(StringParam typeName)
{
  LightningShaderIRLibrary* shaderLibrary = mShaderLibraryRef;
  if (shaderLibrary == nullptr)
    return nullptr;

  return shaderLibrary->FindType(typeName, false);
}

ShaderTranslationPassResult* SimpleLightningShaderIRGenerator::FindTranslationResult(const LightningShaderIRType* shaderType)
{
  ShaderTranslationResult* translationResult = mShaderResults.FindPointer(shaderType->mMeta->mLightningName);
  if (translationResult == nullptr)
    return nullptr;

  return translationResult->mFinalPassData;
}

SimplifiedShaderReflectionData*
SimpleLightningShaderIRGenerator::FindSimplifiedReflectionResult(const LightningShaderIRType* shaderType)
{
  ShaderTranslationResult* translationResult = mShaderResults.FindPointer(shaderType->mMeta->mLightningName);
  if (translationResult == nullptr)
    return nullptr;

  return translationResult->mReflectionData;
}

void SimpleLightningShaderIRGenerator::LoadNameSettings(SpirVNameSettings& nameSettings)
{
  // Any unique name settings here
  nameSettings.mAllowedClassAttributes.Insert("Protected", AttributeInfo(true));
  nameSettings.mAllowedClassAttributes.Insert("PostProcess", AttributeInfo(true));
  nameSettings.mAllowedClassAttributes.Insert("RenderPass", AttributeInfo(true));
  nameSettings.mAllowedClassAttributes.Insert("CoreVertex", AttributeInfo(true));
}

LightningShaderSpirVSettings* SimpleLightningShaderIRGenerator::CreateUnitTestSettings(SpirVNameSettings& nameSettings)
{
  LightningShaderSpirVSettings* settings = new LightningShaderSpirVSettings(nameSettings);

  Lightning::BoundType* realType = LightningTypeId(Lightning::Real);
  Lightning::BoundType* real2Type = LightningTypeId(Lightning::Real2);
  Lightning::BoundType* real3Type = LightningTypeId(Lightning::Real3);
  Lightning::BoundType* real4Type = LightningTypeId(Lightning::Real4);
  Lightning::BoundType* intType = LightningTypeId(Lightning::Integer);
  Lightning::BoundType* int4Type = LightningTypeId(Lightning::Integer4);
  Lightning::BoundType* real4x4Type = LightningTypeId(Lightning::Real4x4);

  // Setup uniform buffers
  UniformBufferDescription frameData(0);
  frameData.mDebugName = "Frame Data";
  frameData.AddField(realType, "LogicTime");
  frameData.AddField(realType, "FrameTime");
  settings->AddUniformBufferDescription(frameData);

  UniformBufferDescription cameraData(1);
  cameraData.mDebugName = "CameraData";
  cameraData.AddField(realType, "NearPlane");
  cameraData.AddField(realType, "FarPlane");
  cameraData.AddField(real2Type, "ViewportSize");
  settings->AddUniformBufferDescription(cameraData);

  UniformBufferDescription transformData(2);
  transformData.mDebugName = "TransformData";
  transformData.AddField(real4x4Type, "LocalToWorld");
  transformData.AddField(real4x4Type, nameSettings.mPerspectiveToApiPerspectiveName);
  settings->AddUniformBufferDescription(transformData);

  settings->AutoSetDefaultUniformBufferDescription();

  //// Add some default vertex definitions (glsl attributes)
  settings->mVertexDefinitions.AddField(realType, "Scalar");
  settings->mVertexDefinitions.AddField(real2Type, "Uv");
  settings->mVertexDefinitions.AddField(real3Type, "LocalNormal");
  settings->mVertexDefinitions.AddField(real3Type, "LocalPosition");
  settings->mVertexDefinitions.AddField(real4Type, "Color");
  settings->mVertexDefinitions.AddField(real4Type, "Aux0");
  settings->mVertexDefinitions.AddField(int4Type, "BoneIndices");

  // Set lightning fragment names for spirv built-ins
  settings->SetHardwareBuiltInName(spv::BuiltInPosition, nameSettings.mApiPerspectivePositionName);

  settings->SetMaxSimultaneousRenderTargets(4);
  settings->SetRenderTargetName("Target0", 0);
  settings->SetRenderTargetName("Target1", 1);
  settings->SetRenderTargetName("Target2", 2);
  settings->SetRenderTargetName("Target3", 3);

  // Set custom callbacks in both the compositor and entry point code generation
  // for dealing with perspective position vs. api perspective position.
  settings->mCallbackSettings.SetCompositeCallback(&LightningShaderIRCompositor::ApiPerspectivePositionCallback, nullptr);
  settings->mCallbackSettings.SetAppendCallback(&EntryPointGeneration::PerspectiveTransformAppendVertexCallback,
                                                nullptr);

  settings->Finalize();

  return settings;
}

LightningShaderSpirVSettings* SimpleLightningShaderIRGenerator::CreatePlasmaSettings(SpirVNameSettings& nameSettings)
{
  nameSettings.mPerspectiveToApiPerspectiveName = "PlasmaPerspectiveToApiPerspective";
  LightningShaderSpirVSettings* settings = new LightningShaderSpirVSettings(nameSettings);

  Lightning::BoundType* realType = LightningTypeId(Lightning::Real);
  Lightning::BoundType* real2Type = LightningTypeId(Lightning::Real2);
  Lightning::BoundType* real3Type = LightningTypeId(Lightning::Real3);
  Lightning::BoundType* real4Type = LightningTypeId(Lightning::Real4);
  Lightning::BoundType* intType = LightningTypeId(Lightning::Integer);
  Lightning::BoundType* int4Type = LightningTypeId(Lightning::Integer4);
  Lightning::BoundType* real3x3Type = LightningTypeId(Lightning::Real3x3);
  Lightning::BoundType* real4x4Type = LightningTypeId(Lightning::Real4x4);

  // Vertex Attributes
  settings->mVertexDefinitions.AddField(real3Type, "LocalPosition");
  settings->mVertexDefinitions.AddField(real3Type, "LocalNormal");
  settings->mVertexDefinitions.AddField(real3Type, "LocalTangent");
  settings->mVertexDefinitions.AddField(real3Type, "LocalBitangent");
  settings->mVertexDefinitions.AddField(real2Type, "Uv");
  settings->mVertexDefinitions.AddField(real2Type, "UvAux");
  settings->mVertexDefinitions.AddField(real4Type, "Color");
  settings->mVertexDefinitions.AddField(real4Type, "ColorAux");
  settings->mVertexDefinitions.AddField(intType, "BoneIndices");
  settings->mVertexDefinitions.AddField(real4Type, "BoneWeights");
  settings->mVertexDefinitions.AddField(real4Type, "Aux0");
  settings->mVertexDefinitions.AddField(real4Type, "Aux1");
  settings->mVertexDefinitions.AddField(real4Type, "Aux2");
  settings->mVertexDefinitions.AddField(real4Type, "Aux3");
  settings->mVertexDefinitions.AddField(real4Type, "Aux4");
  settings->mVertexDefinitions.AddField(real4Type, "Aux5");

  // Setup uniform buffers
  UniformBufferDescription frameData(0);
  frameData.mDebugName = "Frame Data";
  frameData.AddField(realType, "LogicTime");
  frameData.AddField(realType, "FrameTime");
  settings->AddUniformBufferDescription(frameData);

  UniformBufferDescription cameraData(1);
  cameraData.mDebugName = "CameraData";
  cameraData.AddField(realType, "NearPlane");
  cameraData.AddField(realType, "FarPlane");
  cameraData.AddField(real2Type, "ViewportSize");
  cameraData.AddField(real2Type, "InverseViewportSize");
  cameraData.AddField(real3Type, "ObjectWorldPosition");
  settings->AddUniformBufferDescription(cameraData);

  // Transformations
  UniformBufferDescription transformData(2);
  transformData.mDebugName = "TransformData";
  transformData.AddField(real4x4Type, "LocalToWorld");
  transformData.AddField(real4x4Type, "WorldToLocal");
  transformData.AddField(real4x4Type, "WorldToView");
  transformData.AddField(real4x4Type, "ViewToWorld");
  transformData.AddField(real4x4Type, "LocalToView");
  transformData.AddField(real4x4Type, "ViewToLocal");
  transformData.AddField(real3x3Type, "LocalToViewNormal");
  transformData.AddField(real3x3Type, "ViewToLocalNormal");
  transformData.AddField(real3x3Type, "LocalToWorldNormal");
  transformData.AddField(real3x3Type, "WorldToLocalNormal");
  transformData.AddField(real4x4Type, "LocalToPerspective");
  transformData.AddField(real4x4Type, "ViewToPerspective");
  transformData.AddField(real4x4Type, "PerspectiveToView");
  transformData.AddField(real4x4Type, nameSettings.mPerspectiveToApiPerspectiveName);
  settings->AddUniformBufferDescription(transformData);

  settings->AutoSetDefaultUniformBufferDescription();

  // Set lightning fragment names for spirv built-ins
  settings->SetHardwareBuiltInName(spv::BuiltInPosition, nameSettings.mApiPerspectivePositionName);

  settings->SetMaxSimultaneousRenderTargets(4);
  settings->SetRenderTargetName("Target0", 0);
  settings->SetRenderTargetName("Target1", 1);
  settings->SetRenderTargetName("Target2", 2);
  settings->SetRenderTargetName("Target3", 3);

  // Set custom callbacks in both the compositor and entry point code generation
  // for dealing with perspective position vs. api perspective position.
  settings->mCallbackSettings.SetCompositeCallback(&LightningShaderIRCompositor::ApiPerspectivePositionCallback, nullptr);
  settings->mCallbackSettings.SetAppendCallback(&EntryPointGeneration::PerspectiveTransformAppendVertexCallback,
                                                nullptr);

  settings->Finalize();

  return settings;
}

void SimpleLightningShaderIRGenerator::Initialize(FrontEndTranslatorType* translator, LightningShaderSpirVSettings* settings)
{
  mPipeline = nullptr;
  mFrontEndTranslator = translator;
  mSettings = settings;
  SetupTranslator(mFrontEndTranslator);
}

void SimpleLightningShaderIRGenerator::SetupEventConnections()
{
  // Add event connections for errors on the fragment and shader projects
  EventConnect(&mFragmentProject, Events::TranslationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);
  EventConnect(&mFragmentProject, Lightning::Events::CompilationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);
  EventConnect(&mFragmentProject, Events::ValidationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);
  EventConnect(&mShaderProject, Events::TranslationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);
  EventConnect(&mShaderProject, Lightning::Events::CompilationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);
  EventConnect(&mShaderProject, Events::ValidationError, &SimpleLightningShaderIRGenerator::OnForwardEvent, this);
}

void SimpleLightningShaderIRGenerator::SetupTranslator(FrontEndTranslatorType* frontEndTranslator)
{
  frontEndTranslator->SetSettings(mSettings);
  frontEndTranslator->Setup();
}

bool SimpleLightningShaderIRGenerator::CompilePipeline(LightningShaderIRCompositor::ShaderStageDescription& stageDef,
                                                   ShaderPipelineDescription& pipeline)
{
  if (stageDef.mShaderCode.Empty())
    return false;

  LightningShaderIRType* shaderType = mShaderLibraryRef->FindType(stageDef.mClassName);
  if (shaderType == nullptr)
    return false;

  String shaderTypeName = shaderType->mMeta->mLightningName;
  ShaderTranslationResult& translationResult = mShaderResults[shaderTypeName];

  // Compile the pipeline into the given results
  Array<TranslationPassResultRef> pipelineResults;
  CompilePipeline(shaderType, pipeline, pipelineResults, translationResult.mDebugResults);
  // Store the final results from the pipeline
  translationResult.mFinalPassData = pipelineResults.Back();

  // Create the simplified reflection data
  SimplifiedShaderReflectionData* simplifiedReflectionData = new SimplifiedShaderReflectionData();
  simplifiedReflectionData->CreateReflectionData(shaderType->mShaderLibrary, stageDef, pipelineResults);
  translationResult.mReflectionData = simplifiedReflectionData;

  return true;
}

bool SimpleLightningShaderIRGenerator::CompilePipeline(LightningShaderIRType* shaderType,
                                                   ShaderPipelineDescription& pipeline,
                                                   Array<TranslationPassResultRef>& pipelineResults,
                                                   Array<TranslationPassResultRef>& debugResults)
{
  if (shaderType == nullptr)
    return false;

  ShaderTranslationPassResult* binaryBackendData = new ShaderTranslationPassResult();
  pipelineResults.PushBack(binaryBackendData);

  // Convert from the in-memory format of spir-v to actual binary (array of
  // words)
  ShaderByteStreamWriter byteWriter(&binaryBackendData->mByteStream);
  LightningShaderSpirVBinaryBackend binaryBackend;
  binaryBackend.TranslateType(shaderType, byteWriter, binaryBackendData->mReflectionData);

  // Run each tool in the pipeline
  for (size_t i = 0; i < pipeline.mToolPasses.Size(); ++i)
  {
    LightningShaderIRTranslationPass* translationPass = pipeline.mToolPasses[i];

    ShaderTranslationPassResult* prevPassData = pipelineResults.Back();
    ShaderTranslationPassResult* toolData = new ShaderTranslationPassResult();
    pipelineResults.PushBack(toolData);

    translationPass->RunTranslationPass(*prevPassData, *toolData);
  }

  ShaderTranslationPassResult* lastPassData = pipelineResults.Back();

  for (size_t i = 0; i < pipeline.mDebugPasses.Size(); ++i)
  {
    LightningShaderIRTranslationPass* debugBackend = pipeline.mDebugPasses[i];

    ShaderTranslationPassResult* prevPassData = pipelineResults.Back();
    ShaderTranslationPassResult* resultData = new ShaderTranslationPassResult();
    debugResults.PushBack(resultData);

    debugBackend->RunTranslationPass(*prevPassData, *resultData);
  }

  // Run the final backend
  ShaderTranslationPassResult* backendResult = new ShaderTranslationPassResult();
  pipelineResults.PushBack(backendResult);
  pipeline.mBackend->RunTranslationPass(*lastPassData, *backendResult);

  return true;
}

void SimpleLightningShaderIRGenerator::RecursivelyLoadDirectory(StringParam path, LightningShaderIRProject& project)
{
  // This should really use the FileExtensionManager to find files of the
  // correct extension but we can't see that right now. These scripts are
  // manually created as .lightningFrag so it should be fine for now.
  FileRange fileRange(path);
  for (; !fileRange.Empty(); fileRange.PopFront())
  {
    FileEntry entry = fileRange.FrontEntry();
    String filePath = entry.GetFullPath();
    String fileExt = FilePath::GetExtension(filePath);

    if (DirectoryExists(filePath))
    {
      RecursivelyLoadDirectory(filePath, project);
      continue;
    }

    if (fileExt == "lightningFrag")
      project.AddCodeFromFile(filePath, nullptr);
  }
}

void SimpleLightningShaderIRGenerator::OnForwardEvent(Lightning::EventData* e)
{
  EventSend(this, e->EventName, e);
}

} // namespace Plasma
