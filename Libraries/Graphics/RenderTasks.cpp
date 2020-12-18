// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(RenderTasksUpdate);
DefineEvent(RenderTasksUpdateInternal);
} // namespace Events

LightningDefineType(GraphicalRangeInterface, builder, type)
{
  PlasmaBindDocumented();

  LightningBindMethod(Add);
  LightningBindMethod(Clear);
  LightningBindGetterProperty(Count);

  LightningBindDefaultCopyDestructor();
  type->CreatableInScript = true;
}

void GraphicalRangeInterface::Add(Graphical* graphical)
{
  if (graphical != nullptr)
    mGraphicals.PushBack(graphical);
}

void GraphicalRangeInterface::Clear()
{
  mGraphicals.Clear();
}

uint GraphicalRangeInterface::GetCount()
{
  return mGraphicals.Size();
}

LightningDefineType(SubRenderGroupPass, builder, type)
{
  PlasmaBindDocumented();
  LightningBindMethod(Reset);
  LightningBindMethod(SetDefaultSettings);
  LightningBindMethod(AddSubSettings);
  LightningBindMethod(ExcludeSubRenderGroup);
}

SubRenderGroupPass::SubRenderGroupPass(RenderTasksEvent* renderTasksEvent, RenderGroup& baseRenderGroup) :
    mRenderTasksEvent(renderTasksEvent)
{
  Reset(baseRenderGroup);
}

void SubRenderGroupPass::Reset(RenderGroup& baseRenderGroup)
{
  mBaseRenderGroup = baseRenderGroup;
  mSubData.Clear();

  // Must always have a base entry at index 0, also serves as defaults if set.
  SubRenderGroupPass::SubData& subData = mSubData.PushBack();
  subData.mRenderGroup = mBaseRenderGroup;
  subData.mRender = false;
}

void SubRenderGroupPass::SetDefaultSettings(GraphicsRenderSettings& defaultSettings, MaterialBlock& defaultPass)
{
  if (mSubData[0].mRender)
    return DoNotifyException("Error", "Defaults have already been set.");

  if (!ValidateSettings(defaultSettings, defaultPass))
    return;

  SubRenderGroupPass::SubData& subData = mSubData[0];
  subData.mRenderSettings = defaultSettings;
  subData.mRenderPass = defaultPass;
  subData.mRender = true;
}

void SubRenderGroupPass::AddSubSettings(GraphicsRenderSettings& subSettings,
                                        RenderGroup& subGroup,
                                        MaterialBlock& subPass)
{
  if (!ValidateRenderGroup(subGroup))
    return;

  if (!ValidateSettings(subSettings, subPass))
    return;

  SubRenderGroupPass::SubData& subData = mSubData.PushBack();
  subData.mRenderSettings = subSettings;
  subData.mRenderGroup = &subGroup;
  subData.mRenderPass = subPass;
  subData.mRender = true;
}

void SubRenderGroupPass::ExcludeSubRenderGroup(RenderGroup& subGroup)
{
  if (!ValidateRenderGroup(subGroup))
    return;

  SubRenderGroupPass::SubData& subData = mSubData.PushBack();
  subData.mRenderGroup = &subGroup;
  subData.mRender = false;
}

bool SubRenderGroupPass::ValidateSettings(GraphicsRenderSettings& renderSettings, MaterialBlock& renderPass)
{
  LightningFragmentType::Enum fragmentType = PL::gEngine->has(GraphicsEngine)->GetFragmentType(&renderPass);
  if (fragmentType != LightningFragmentType::RenderPass)
    return DoNotifyException("Error", "Fragment is not a [RenderPass]"), false;

  if (!RenderTaskHelper(mRenderTasksEvent->mRenderTasks->mRenderTaskBuffer).ValidateRenderTargets(renderSettings))
    return false;

  return true;
}

bool SubRenderGroupPass::ValidateRenderGroup(RenderGroup& renderGroup)
{
  if (!mBaseRenderGroup->IsSubRenderGroup(&renderGroup))
    return DoNotifyException("Error",
                             String::Format("'%s' is not a child of the base RenderGroup '%s'.",
                                            renderGroup.Name.c_str(),
                                            mBaseRenderGroup->Name.c_str())),
           false;

  for (size_t i = 1; i < mSubData.Size(); ++i)
  {
    if (&renderGroup == *mSubData[i].mRenderGroup)
      return DoNotifyException(
                 "Error",
                 String::Format("Settings or exclusion for '%s' have already been added.", renderGroup.Name.c_str())),
             false;
  }

  return true;
}

LightningDefineType(RenderTasksEvent, builder, type)
{
  PlasmaBindDocumented();
  PlasmaBindEvent(Events::RenderTasksUpdate, RenderTasksEvent);

  LightningBindGetter(ViewportSize);
  LightningBindGetter(CameraViewportCog);

  LightningBindOverloadedMethod(GetRenderTarget,
                            LightningInstanceOverload(HandleOf<RenderTarget>, IntVec2, TextureFormat::Enum));
  LightningBindOverloadedMethod(
      GetRenderTarget, LightningInstanceOverload(HandleOf<RenderTarget>, IntVec2, TextureFormat::Enum, SamplerSettings&));
  LightningBindOverloadedMethod(GetRenderTarget, LightningInstanceOverload(HandleOf<RenderTarget>, HandleOf<Texture>));

  LightningBindMethod(CreateSubRenderGroupPass);

  LightningBindOverloadedMethod(AddRenderTaskClearTarget, LightningInstanceOverload(void, RenderTarget*, Vec4));
  LightningBindOverloadedMethod(AddRenderTaskClearTarget, LightningInstanceOverload(void, RenderTarget*, float));
  LightningBindOverloadedMethod(AddRenderTaskClearTarget, LightningInstanceOverload(void, RenderTarget*, float, uint));
  LightningBindOverloadedMethod(AddRenderTaskClearTarget, LightningInstanceOverload(void, RenderTarget*, float, uint, uint));
  LightningBindOverloadedMethod(AddRenderTaskClearTarget,
                            LightningInstanceOverload(void, RenderTarget*, RenderTarget*, Vec4, float));
  LightningBindOverloadedMethod(AddRenderTaskClearTarget,
                            LightningInstanceOverload(void, RenderTarget*, RenderTarget*, Vec4, float, uint));
  LightningBindOverloadedMethod(AddRenderTaskClearTarget,
                            LightningInstanceOverload(void, RenderTarget*, RenderTarget*, Vec4, float, uint, uint));
  LightningBindOverloadedMethod(AddRenderTaskClearTarget, LightningInstanceOverload(void, GraphicsRenderSettings&, Vec4));
  LightningBindOverloadedMethod(AddRenderTaskClearTarget,
                            LightningInstanceOverload(void, GraphicsRenderSettings&, Vec4, float));
  LightningBindOverloadedMethod(AddRenderTaskClearTarget,
                            LightningInstanceOverload(void, GraphicsRenderSettings&, Vec4, float, uint));
  LightningBindOverloadedMethod(AddRenderTaskClearTarget,
                            LightningInstanceOverload(void, GraphicsRenderSettings&, Vec4, float, uint, uint));

  LightningBindOverloadedMethod(AddRenderTaskRenderPass,
                            LightningInstanceOverload(void, GraphicsRenderSettings&, RenderGroup&, MaterialBlock&, String&));
  LightningBindOverloadedMethod(
      AddRenderTaskRenderPass,
      LightningInstanceOverload(void, GraphicsRenderSettings&, GraphicalRangeInterface&, MaterialBlock&, String&));

  LightningBindMethod(AddRenderTaskSubRenderGroupPass);

  LightningBindOverloadedMethod(AddRenderTaskPostProcess, LightningInstanceOverload(void, RenderTarget*, Material&, String&));
  LightningBindOverloadedMethod(AddRenderTaskPostProcess, LightningInstanceOverload(void, RenderTarget*, MaterialBlock&, String&));
  LightningBindOverloadedMethod(AddRenderTaskPostProcess, LightningInstanceOverload(void, GraphicsRenderSettings&, Material&, String&));
  LightningBindOverloadedMethod(AddRenderTaskPostProcess,
                            LightningInstanceOverload(void, GraphicsRenderSettings&, MaterialBlock&, String&));

  LightningBindOverloadedMethod(GetFinalTarget,
                            LightningInstanceOverload(HandleOf<RenderTarget>, IntVec2, TextureFormat::Enum));
  LightningBindOverloadedMethod(
      GetFinalTarget, LightningInstanceOverload(HandleOf<RenderTarget>, IntVec2, TextureFormat::Enum, SamplerSettings&));
}

RenderTasksEvent::RenderTasksEvent() :
    mViewportSize(IntVec2::cZero),
    mRenderTasks(nullptr),
    mFinalTexture(nullptr),
    mGraphicsSpace(nullptr),
    mCamera(nullptr)
{
}

RenderTasksEvent::~RenderTasksEvent()
{
  for (size_t i = 0; i < mSubRenderGroupPasses.Size(); ++i)
    delete mSubRenderGroupPasses[i];
}

Cog* RenderTasksEvent::GetCameraViewportCog()
{
  return mCameraViewportCog;
}

IntVec2 RenderTasksEvent::GetViewportSize()
{
  return mViewportSize;
}

HandleOf<RenderTarget> RenderTasksEvent::GetRenderTarget(IntVec2 size, TextureFormat::Enum format)
{
  SamplerSettings samplerSettings;
  return GetRenderTarget(size, format, samplerSettings);
}

HandleOf<RenderTarget> RenderTasksEvent::GetRenderTarget(IntVec2 size,
                                                         TextureFormat::Enum format,
                                                         SamplerSettings& samplerSettings)
{
  // need to determine a good size limit
  size = Math::Clamp(size, IntVec2(1, 1), IntVec2(4096, 4096));
  return PL::gEngine->has(GraphicsEngine)->GetRenderTarget((uint)size.x, (uint)size.y, format, samplerSettings);
}

HandleOf<RenderTarget> RenderTasksEvent::GetRenderTarget(HandleOf<Texture> texture)
{
  return PL::gEngine->has(GraphicsEngine)->GetRenderTarget(texture);
}

HandleOf<SubRenderGroupPass> RenderTasksEvent::CreateSubRenderGroupPass(RenderGroup& baseGroup)
{
  SubRenderGroupPass* subRenderGroupPass = new SubRenderGroupPass(this, baseGroup);
  mSubRenderGroupPasses.PushBack(subRenderGroupPass);
  return subRenderGroupPass;
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderTarget* colorTarget, Vec4 color)
{
  GraphicsRenderSettings renderSettings;
  renderSettings.SetColorTarget(colorTarget);
  AddRenderTaskClearTarget(renderSettings, color, 0.0f, 0, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderTarget* depthTarget, float depth)
{
  GraphicsRenderSettings renderSettings;
  renderSettings.SetDepthTarget(depthTarget);
  AddRenderTaskClearTarget(renderSettings, Vec4::cZero, depth, 0, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderTarget* depthTarget, float depth, uint stencil)
{
  GraphicsRenderSettings renderSettings;
  renderSettings.SetDepthTarget(depthTarget);
  AddRenderTaskClearTarget(renderSettings, Vec4::cZero, depth, stencil, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderTarget* depthTarget,
                                                float depth,
                                                uint stencil,
                                                uint stencilWriteMask)
{
  GraphicsRenderSettings renderSettings;
  renderSettings.SetDepthTarget(depthTarget);
  AddRenderTaskClearTarget(renderSettings, Vec4::cZero, depth, stencil, stencilWriteMask);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderTarget* colorTarget,
                                                RenderTarget* depthTarget,
                                                Vec4 color,
                                                float depth)
{
  GraphicsRenderSettings renderSettings;
  renderSettings.SetColorTarget(colorTarget);
  renderSettings.SetDepthTarget(depthTarget);
  AddRenderTaskClearTarget(renderSettings, color, depth, 0, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(
    RenderTarget* colorTarget, RenderTarget* depthTarget, Vec4 color, float depth, uint stencil)
{
  GraphicsRenderSettings renderSettings;
  renderSettings.SetColorTarget(colorTarget);
  renderSettings.SetDepthTarget(depthTarget);
  AddRenderTaskClearTarget(renderSettings, color, depth, stencil, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(
    RenderTarget* colorTarget, RenderTarget* depthTarget, Vec4 color, float depth, uint stencil, uint stencilWriteMask)
{
  GraphicsRenderSettings renderSettings;
  renderSettings.SetColorTarget(colorTarget);
  renderSettings.SetDepthTarget(depthTarget);
  AddRenderTaskClearTarget(renderSettings, color, depth, stencil, stencilWriteMask);
}

void RenderTasksEvent::AddRenderTaskClearTarget(GraphicsRenderSettings& renderSettings, Vec4 color)
{
  AddRenderTaskClearTarget(renderSettings, color, 0, 0, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(GraphicsRenderSettings& renderSettings, Vec4 color, float depth)
{
  AddRenderTaskClearTarget(renderSettings, color, depth, 0, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(GraphicsRenderSettings& renderSettings,
                                                Vec4 color,
                                                float depth,
                                                uint stencil)
{
  AddRenderTaskClearTarget(renderSettings, color, depth, stencil, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(
    GraphicsRenderSettings& renderSettings, Vec4 color, float depth, uint stencil, uint stencilWriteMask)
{
  RenderTaskHelper(mRenderTasks->mRenderTaskBuffer)
      .AddRenderTaskClearTarget(renderSettings, color, depth, stencil, stencilWriteMask);
}

void RenderTasksEvent::AddRenderTaskRenderPass(GraphicsRenderSettings& renderSettings,
                                               RenderGroup& renderGroup,
                                               MaterialBlock& renderPass,
											   String& name)
{
	
  LightningFragmentType::Enum fragmentType = PL::gEngine->has(GraphicsEngine)->GetFragmentType(&renderPass);
  if (fragmentType != LightningFragmentType::RenderPass)
    return DoNotifyException("Error", "Fragment is not a [RenderPass]");

  HashSet<Material*> materials;
  renderGroup.GetMaterials(materials);

  uint shaderInputsId = GetUniqueShaderInputsId();

  AddShaderInputs(&renderPass, shaderInputsId);
  forRange (Material* material, materials.All())
    AddShaderInputs(material, shaderInputsId);

  AddShaderInputs(renderSettings.mGlobalShaderInputs, shaderInputsId);

  String renderPassName = LightningVirtualTypeId(&renderPass)->Name;
  RenderTaskHelper(mRenderTasks->mRenderTaskBuffer)
      .AddRenderTaskRenderPass(renderSettings, renderGroup.mSortId, renderPassName, name, shaderInputsId);

  mCamera->mUsedRenderGroupIds.Insert(renderGroup.mSortId);
}

void RenderTasksEvent::AddRenderTaskSubRenderGroupPass(SubRenderGroupPass& subRenderGroupPass)
{
  // Index 0 is always the base task entry and/or the defaults.
  // The base task needs to know how many other tasks proceed it for sub
  // RenderGroup settings, which is all the elements minus the one base task.
  uint subGroupCount = subRenderGroupPass.mSubData.Size() - 1;

  HashSet<Material*> materials;
  subRenderGroupPass.mBaseRenderGroup->GetMaterials(materials);

  RenderTaskHelper renderTaskHelper(mRenderTasks->mRenderTaskBuffer);

  for (size_t i = 0; i < subRenderGroupPass.mSubData.Size(); ++i)
  {
    SubRenderGroupPass::SubData& subData = subRenderGroupPass.mSubData[i];

    // Don't process shader inputs for non render entries.
    uint shaderInputsId = GetUniqueShaderInputsId();
    if (subData.mRender)
    {
      AddShaderInputs(&subData.mRenderPass, shaderInputsId);
      // Okay to use the whole Material set from the base RenderGroup because
      // inputs will just be cached but we need an input range entry for each
      // unique shaderInputsId to find the inputs.
      forRange (Material* material, materials.All())
        AddShaderInputs(material, shaderInputsId);

      AddShaderInputs(subData.mRenderSettings.mGlobalShaderInputs, shaderInputsId);
    }

    String renderPassName = LightningVirtualTypeId(&subData.mRenderPass)->Name;
    String name = "subPass";
    renderTaskHelper.AddRenderTaskRenderPass(subData.mRenderSettings,
                                             subData.mRenderGroup->mSortId,
                                             renderPassName,
                                             name,
                                             shaderInputsId,
                                             subGroupCount,
                                             subData.mRender);

    // Only set the sub count for the first task.
    subGroupCount = 0;
  }

  // Sub RenderGroups are sorted as the base group, only set the base as being
  // used.
  mCamera->mUsedRenderGroupIds.Insert(subRenderGroupPass.mBaseRenderGroup->mSortId);
}

void RenderTasksEvent::AddRenderTaskRenderPass(GraphicsRenderSettings& renderSettings,
                                               GraphicalRangeInterface& graphicalRange,
                                               MaterialBlock& renderPass,
											   String& name)
{
  //if (renderPass.mOwner)
  //{
  //  ProfileScopeTree(renderPass.mOwner-, "RenderTasksUpdate", Color::Cyan)
  //}
	
  LightningFragmentType::Enum fragmentType = PL::gEngine->has(GraphicsEngine)->GetFragmentType(&renderPass);
  if (fragmentType != LightningFragmentType::RenderPass)
    return DoNotifyException("Error", "Fragment is not a [RenderPass]");

  HashSet<Material*> materials;

  // Need to add an index range for this set of entries
  IndexRange indexRange;
  indexRange.start = mGraphicsSpace->mVisibleGraphicals.Size();

  forRange (Graphical* graphical, graphicalRange.mGraphicals.All())
  {
    // Do not allow a graphical from a different space
    if (graphical->GetSpace() != mGraphicsSpace->GetOwner())
      continue;

    // No sort values are needed, these entries are to be rendered in the order
    // given
    graphical->MidPhaseQuery(mGraphicsSpace->mVisibleGraphicals, *mCamera, nullptr);
    materials.Insert(graphical->mMaterial);
  }

  // Skip task if no objects
  indexRange.end = mGraphicsSpace->mVisibleGraphicals.Size();
  if (indexRange.Count() == 0)
    return;

  mCamera->mGraphicalIndexRanges.PushBack(indexRange);

  // Adding to render group counts will make the camera setup a range for nodes
  // that will end up in view block The indexes of this array correspond to
  // render group id's, adding to it acts as a custom render group
  mCamera->mRenderGroupCounts.PushBack(indexRange.Count());
  uint groupId = mCamera->mRenderGroupCounts.Size() - 1;

  uint shaderInputsId = GetUniqueShaderInputsId();

  AddShaderInputs(&renderPass, shaderInputsId);
  forRange (Material* material, materials.All())
    AddShaderInputs(material, shaderInputsId);

  AddShaderInputs(renderSettings.mGlobalShaderInputs, shaderInputsId);

  String renderPassName = LightningVirtualTypeId(&renderPass)->Name;
  RenderTaskHelper(mRenderTasks->mRenderTaskBuffer)
      .AddRenderTaskRenderPass(renderSettings, groupId, renderPassName, name, shaderInputsId);

  mCamera->mUsedRenderGroupIds.Insert(groupId);
}

void RenderTasksEvent::AddRenderTaskPostProcess(RenderTarget* colorTarget, Material& material, String& name)
{
  GraphicsRenderSettings renderSettings;
  renderSettings.SetColorTarget(colorTarget);
  AddRenderTaskPostProcess(renderSettings, material, name);
}

void RenderTasksEvent::AddRenderTaskPostProcess(RenderTarget* colorTarget, MaterialBlock& postProcess, String& name)
{
  GraphicsRenderSettings renderSettings;
  renderSettings.SetColorTarget(colorTarget);
  AddRenderTaskPostProcess(renderSettings, postProcess, name);
}

void RenderTasksEvent::AddRenderTaskPostProcess(GraphicsRenderSettings& renderSettings, Material& material, String& name)
{
  uint shaderInputsId = GetUniqueShaderInputsId();
  AddShaderInputs(&material, shaderInputsId);

  AddShaderInputs(renderSettings.mGlobalShaderInputs.Dereference(), shaderInputsId);

  RenderTaskHelper(mRenderTasks->mRenderTaskBuffer)
      .AddRenderTaskPostProcess(renderSettings, material.mRenderData, shaderInputsId, name);
}

void RenderTasksEvent::AddRenderTaskPostProcess(GraphicsRenderSettings& renderSettings, MaterialBlock& postProcess, String& name)
{
  //if (postProcess.mOwner)
  //{
  //  ProfileScopeTree(postProcess.mOwner->Name, "RenderTasksUpdate", Color::DarkSeaGreen)
  //}
	
  LightningFragmentType::Enum fragmentType = PL::gEngine->has(GraphicsEngine)->GetFragmentType(&postProcess);
  if (fragmentType != LightningFragmentType::PostProcess)
    return DoNotifyException("Error", "Fragment is not a [PostProcess]");

  uint shaderInputsId = GetUniqueShaderInputsId();
  AddShaderInputs(&postProcess, shaderInputsId);

  AddShaderInputs(renderSettings.mGlobalShaderInputs, shaderInputsId);

  String postProcessName = LightningVirtualTypeId(&postProcess)->Name;
  RenderTaskHelper(mRenderTasks->mRenderTaskBuffer)
      .AddRenderTaskPostProcess(renderSettings, postProcessName, shaderInputsId, name);
}

void RenderTasksEvent::AddRenderTaskBackBufferBlit(RenderTarget* colorTarget, ScreenViewport viewport)
{
  RenderTaskHelper(mRenderTasks->mRenderTaskBuffer).AddRenderTaskBackBufferBlit(colorTarget, viewport);
}

HandleOf<RenderTarget> RenderTasksEvent::GetFinalTarget(IntVec2 size, TextureFormat::Enum format)
{
  SamplerSettings samplerSettings;
  return GetFinalTarget(size, format, samplerSettings);
}

HandleOf<RenderTarget> RenderTasksEvent::GetFinalTarget(IntVec2 size,
                                                        TextureFormat::Enum format,
                                                        SamplerSettings& samplerSettings)
{
  size = Math::Clamp(size, IntVec2(1, 1), IntVec2(4096, 4096));

  // Allow changes to CameraViewport's target through this method
  Texture* texture = mFinalTexture;
  texture->mProtected = false;
  texture->SetSize(size);
  texture->SetFormat(format);
  texture->SetAddressingX(samplerSettings.mAddressingX);
  texture->SetAddressingY(samplerSettings.mAddressingY);
  texture->SetFiltering(samplerSettings.mFiltering);
  texture->SetCompareMode(samplerSettings.mCompareMode);
  texture->SetCompareFunc(samplerSettings.mCompareFunc);

  HandleOf<RenderTarget> renderTarget = GetRenderTarget(mFinalTexture);
  texture->mProtected = true;

  return renderTarget;
}

uint RenderTasksEvent::GetUniqueShaderInputsId()
{
  static uint sIncrementalShaderInputsId = 0;
  return ++sIncrementalShaderInputsId;
}

void RenderTasksEvent::AddShaderInputs(Material* material, uint shaderInputsId)
{
  Pair<u64, uint> key((u64)material->mResourceId, shaderInputsId);
  IndexRange range = material->AddShaderInputs(mRenderTasks->mShaderInputs, mRenderTasks->mShaderInputsVersion);
  mRenderTasks->mShaderInputRanges.Insert(key, range);
}

void RenderTasksEvent::AddShaderInputs(MaterialBlock* materialBlock, uint shaderInputsId)
{
  Pair<u64, uint> key(cFragmentShaderInputsId, shaderInputsId);
  IndexRange range = materialBlock->AddShaderInputs(mRenderTasks->mShaderInputs);
  mRenderTasks->mShaderInputRanges.Insert(key, range);
}

void RenderTasksEvent::AddShaderInputs(ShaderInputs* globalInputs, uint shaderInputsId)
{
  if (globalInputs == nullptr)
    return;

  uint start = mRenderTasks->mShaderInputs.Size();
  mRenderTasks->mShaderInputs.Append(globalInputs->mShaderInputs.Values());
  uint end = mRenderTasks->mShaderInputs.Size();

  Pair<u64, uint> key(cGlobalShaderInputsId, shaderInputsId);
  IndexRange range(start, end);
  mRenderTasks->mShaderInputRanges.Insert(key, range);
}

void RenderTasks::Clear()
{
  mRenderTaskRanges.Clear();
  mRenderTaskBuffer.Clear();

  mShaderInputRanges.Clear();
  mShaderInputs.Clear();
}

RenderTaskHelper::RenderTaskHelper(RenderTaskBuffer& buffer) : mBuffer(buffer)
{
}

void RenderTaskHelper::AddRenderTaskClearTarget(
    RenderSettings& renderSettings, Vec4 color, float depth, uint stencil, uint stencilWriteMask)
{
  if (!ValidateRenderTargets(renderSettings))
    return;

  RenderTaskClearTarget* renderTask = NewRenderTask<RenderTaskClearTarget>();
  renderTask->mId = RenderTaskType::ClearTarget;
  renderTask->mRenderSettings = renderSettings;
  renderTask->mColor = color;
  renderTask->mDepth = depth;
  renderTask->mStencil = stencil;
  renderTask->mStencilWriteMask = stencilWriteMask;
}

void RenderTaskHelper::AddRenderTaskRenderPass(RenderSettings& renderSettings,
                                               uint renderGroupIndex,
                                               StringParam renderPassName,
											   StringParam displayName,
                                               uint shaderInputsId,
                                               uint subRenderGroupCount,
                                               bool render)
{
  // Don't validate targets if not rendering this group, none of the settings
  // are made or used in this case.
  if (render && !ValidateRenderTargets(renderSettings))
    return;

  RenderTaskRenderPass* renderTask = NewRenderTask<RenderTaskRenderPass>();
  renderTask->mId = RenderTaskType::RenderPass;
  renderTask->mRenderSettings = renderSettings;
  renderTask->mRenderGroupIndex = renderGroupIndex;
  renderTask->mRenderPassName = renderPassName;
  renderTask->mRenderPassDisplayName = displayName;
  renderTask->mShaderInputsId = shaderInputsId;
  renderTask->mSubRenderGroupCount = subRenderGroupCount;
  renderTask->mRender = render;
}

void RenderTaskHelper::AddRenderTaskPostProcess(RenderSettings& renderSettings,
                                                StringParam postProcessName,
                                                uint shaderInputsId,
												StringParam name)
{
  if (!ValidateRenderTargets(renderSettings))
    return;

  RenderTaskPostProcess* renderTask = NewRenderTask<RenderTaskPostProcess>();
  renderTask->mId = RenderTaskType::PostProcess;
  renderTask->mRenderSettings = renderSettings;
  renderTask->mDisplayName = name;
  renderTask->mPostProcessName = postProcessName;
  renderTask->mMaterialRenderData = nullptr;
  renderTask->mShaderInputsId = shaderInputsId;
}

void RenderTaskHelper::AddRenderTaskPostProcess(RenderSettings& renderSettings,
                                                MaterialRenderData* materialRenderData,
                                                uint shaderInputsId,
												String& name)
{
  if (!ValidateRenderTargets(renderSettings))
    return;

  RenderTaskPostProcess* renderTask = NewRenderTask<RenderTaskPostProcess>();
  renderTask->mId = RenderTaskType::PostProcess;
  renderTask->mRenderSettings = renderSettings;
  renderTask->mPostProcessName = String();
  renderTask->mDisplayName = name;
  renderTask->mMaterialRenderData = materialRenderData;
  renderTask->mShaderInputsId = shaderInputsId;
}

void RenderTaskHelper::AddRenderTaskBackBufferBlit(RenderTarget* colorTarget, ScreenViewport viewport)
{
  // Don't need to validate render target, only used internally
  RenderTaskBackBufferBlit* renderTask = NewRenderTask<RenderTaskBackBufferBlit>();
  renderTask->mId = RenderTaskType::BackBufferBlit;
  Texture* texture = colorTarget->mTexture;
  renderTask->mColorTarget = texture->mRenderData;
  renderTask->mViewport = viewport;
}

void RenderTaskHelper::AddRenderTaskTextureUpdate(Texture* texture)
{
  RenderTaskTextureUpdate* renderTask = NewRenderTask<RenderTaskTextureUpdate>();
  renderTask->mId = RenderTaskType::TextureUpdate;
  renderTask->mRenderData = texture->mRenderData;
  renderTask->mWidth = texture->mWidth;
  renderTask->mHeight = texture->mHeight;
  renderTask->mType = texture->mType;
  renderTask->mFormat = texture->mFormat;
  renderTask->mAddressingX = texture->mAddressingX;
  renderTask->mAddressingY = texture->mAddressingY;
  renderTask->mFiltering = texture->mFiltering;
  renderTask->mCompareMode = texture->mCompareMode;
  renderTask->mCompareFunc = texture->mCompareFunc;
  renderTask->mAnisotropy = texture->mAnisotropy;
  renderTask->mMipMapping = texture->mMipMapping;

  texture->mDirty = false;
}

bool RenderTaskHelper::ValidateRenderTargets(RenderSettings& renderSettings)
{
  static Array<IntVec2> sizes;
  sizes.Clear();

  uint targetCount = renderSettings.mSingleColorTarget ? 1 : 8;

  // Format validations
  for (uint i = 0; i < targetCount; ++i)
  {
    Texture* texture = (Texture*)renderSettings.mColorTextures[i];
    if (texture != nullptr)
    {
      if (IsColorFormat(texture->mFormat))
        sizes.PushBack(texture->GetSize());
      else
        return DoNotifyException("Error", "Invalid color attachment."), false;
    }
  }

  Texture* depthTexture = (Texture*)renderSettings.mDepthTexture;
  if (depthTexture != nullptr)
  {
    if (IsDepthFormat(depthTexture->mFormat))
      sizes.PushBack(depthTexture->GetSize());
    else
      return DoNotifyException("Error", "Invalid depth attachment."), false;
  }

  // Size validations
  if (sizes.Empty())
    return DoNotifyException("Error", "No RenderTargets."), false;

  for (uint i = 1; i < sizes.Size(); ++i)
    if (sizes[i] != sizes[i - 1])
      return DoNotifyException("Error", "Mismatching RenderTarget sizes."), false;

  renderSettings.mTargetsWidth = (uint)sizes[0].x;
  renderSettings.mTargetsHeight = (uint)sizes[0].y;

  // Add render tasks for dirty textures
  for (uint i = 0; i < targetCount; ++i)
  {
    Texture* texture = (Texture*)renderSettings.mColorTextures[i];
    if (texture != nullptr && texture->mDirty)
      AddRenderTaskTextureUpdate(texture);
  }

  if (depthTexture && depthTexture->mDirty)
    AddRenderTaskTextureUpdate(depthTexture);

  return true;
}

void RenderTasksUpdateHelper(RenderTasksUpdateData& update)
{
  Array<RenderTaskRange>& renderTaskRanges = update.mEvent->mRenderTasks->mRenderTaskRanges;
  RenderTaskBuffer& renderTaskBuffer = update.mEvent->mRenderTasks->mRenderTaskBuffer;

  RenderTaskRange range;
  range.mTaskIndex = renderTaskBuffer.mCurrentIndex;

  uint startingTaskCount = renderTaskBuffer.mTaskCount;

  update.mCamera->mUsedRenderGroupIds.Clear();

  // get tasks from renderer script
  update.mDispatcher->Dispatch(Events::RenderTasksUpdate, update.mEvent);

  range.mTaskCount = renderTaskBuffer.mTaskCount - startingTaskCount;

  // check for render tasks and texture output
  if (range.mTaskCount == 0)
  {
    // undo the added render tasks that aren't going to be used
    renderTaskBuffer.mCurrentIndex = range.mTaskIndex;
    renderTaskBuffer.mTaskCount -= range.mTaskCount;
    return;
  }

  update.mTaskCount = range.mTaskCount;

  range.mRenderOrder = update.mRenderOrder;
  renderTaskRanges.PushBack(range);
  uint rangeIndex = renderTaskRanges.Size() - 1;

  // signal to the camera that its data is needed for the RenderQueues
  update.mCamera->mRenderQueuesDataNeeded = true;

  // tell GraphicsSpace and the Camera the index of the RenderTasks object so
  // that they can later tell the RenderTasks what the FrameBlock and ViewBlock
  // index is
  update.mGraphicsSpace->mRenderTaskRangeIndices.PushBack(rangeIndex);
  update.mCamera->mRenderTaskRangeIndices.PushBack(rangeIndex);
}

} // namespace Plasma
