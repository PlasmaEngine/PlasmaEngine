// MIT Licensed (see LICENSE.md).

#pragma once

namespace Plasma
{

DeclareEnum3(UniqueCompositeOp, Add, Remove, Modify);

namespace Events
{
DeclareEvent(ShaderInputsModified);
}

class ShaderInputsEvent : public Event
{
public:
  LightningDeclareType(ShaderInputsEvent, TypeCopyMode::ReferenceType);
  BoundType* mType;
};

class ShaderMetaProperty
{
public:
  String mMetaPropertyName;
  String mFragmentName;
  String mInputName;
};

/// Interface for static utilities of the Graphics engine.
class GraphicsStatics
{
public:
  LightningDeclareType(GraphicsStatics, TypeCopyMode::ReferenceType);

  /// Information about the active graphics hardware.
  static GraphicsDriverSupport* GetDriverSupport();
};

class TextureToFile
{
public:
  TextureToFile()
  {
  }
  TextureToFile(HandleOf<Texture> texture, StringParam filename) : mTexture(texture), mFilename(filename)
  {
  }
  HandleOf<Texture> mTexture;
  String mFilename;
};

/// System object for graphics.
class GraphicsEngine : public System
{
public:
  LightningDeclareType(GraphicsEngine, TypeCopyMode::ReferenceType);

  GraphicsEngine();
  ~GraphicsEngine();

  cstr GetName() override;
  void Initialize(SystemInitializer& initializer) override;
  void Update(bool debugger) override;

  void OnEngineShutdown(Event* event);

  void AddSpace(GraphicsSpace* space);
  void RemoveSpace(GraphicsSpace* space);

  // Show loading progress
  void StartProgress(Event* event);
  void UpdateProgress(ProgressEvent* event);
  void EndProgress(Event* event);
  void OnProjectLoaded(ObjectEvent* event);
  void OnNoProjectLoaded(Event* event);
  void SetSplashscreenLoading();
  void EndProgressDelayTerminate();

  void OnOsWindowMinimized(Event* event);
  void OnOsWindowRestored(Event* event);

  void OnProjectCogModified(Event* event);
  void SetVerticalSync(bool verticalSync);

  uint GetRenderGroupCount();
  void UpdateRenderGroups();

  void CheckTextureYInvert(Texture* texture);

  // Methods for making RendererJobs
  void AddRendererJob(RendererJob* rendererJob);
  void CreateRenderer(OsWindow* mainWindow);
  void DestroyRenderer();
  void AddMaterial(Material* material);
  void AddMesh(Mesh* mesh);
  void AddTexture(Texture* texture, bool subImage = false, uint xOffset = 0, uint yOffset = 0);
  void RemoveMaterial(Material* material);
  void RemoveMesh(Mesh* mesh);
  void RemoveTexture(Texture* texture);
  void SetLazyShaderCompilation(bool lazyShaderCompilation);

  void OnRenderGroupAdded(ResourceEvent* event);
  void OnRenderGroupModified(ResourceEvent* event);
  void OnRenderGroupRemoved(ResourceEvent* event);

  void OnMaterialAdded(ResourceEvent* event);
  void OnMaterialModified(ResourceEvent* event);
  void OnMaterialRemoved(ResourceEvent* event);

  void OnLightningFragmentAdded(ResourceEvent* event);
  void OnLightningFragmentModified(ResourceEvent* event);
  void OnLightningFragmentRemoved(ResourceEvent* event);

  void OnMeshAdded(ResourceEvent* event);
  void OnMeshModified(ResourceEvent* event);
  void OnMeshRemoved(ResourceEvent* event);

  void OnTextureAdded(ResourceEvent* event);
  void OnTextureModified(ResourceEvent* event);
  void OnTextureRemoved(ResourceEvent* event);

  void OnResourcesAdded(ResourceEvent* event);
  void OnResourcesRemoved(ResourceEvent* event);

  void AddComposite(Material* material);
  void RemoveComposite(StringParam compositeName);

  Shader*
  GetOrCreateShader(StringParam coreVertex, StringParam composite, StringParam renderPass, ShaderMap& shaderMap);
  void FindShadersToCompile(Array<String>& coreVertexRange,
                            Array<String>& compositeRange,
                            Array<String>& renderPassRange,
                            ShaderSetMap& testMap,
                            uint index,
                            ShaderSet& shaders);
  void FindShadersToRemove(Array<String>& elementRange, ShaderSetMap& testMap, ShaderSet& shaders);

  void AddToShaderMaps(ShaderSet& shaders);
  void RemoveFromShaderMaps(ShaderSet& shaders);
  void RemoveFromShaderMap(ShaderSetMap& shaderMap, StringParam elementName, Shader* shader);

  void ForceCompileAllShaders();

  void ProcessModifiedScripts(LibraryRef library);

  LightningFragmentType::Enum GetFragmentType(MaterialBlock* materialBlock);

  HandleOf<RenderTarget> GetRenderTarget(uint width,
                                         uint height,
                                         TextureFormat::Enum format,
                                         SamplerSettings samplerSettings = SamplerSettings());
  HandleOf<RenderTarget> GetRenderTarget(HandleOf<Texture> texture);
  void ClearRenderTargets();

  void WriteTextureToFile(HandleOf<Texture> texture, StringParam filename);

  void ModifiedFragment(LightningFragmentType::Enum type, StringParam name);
  void RemovedFragment(LightningFragmentType::Enum type, StringParam name);

  // ResourceLibraries don't know how to compile fragment libraries, so we do it
  // here.
  void OnCompileLightningFragments(LightningCompileFragmentEvent* event);
  void OnScriptsCompiledPrePatch(LightningCompileEvent* event);
  void OnScriptsCompiledCommit(LightningCompileEvent* event);
  void OnScriptsCompiledPostPatch(LightningCompileEvent* event);
  void OnScriptCompilationFailed(Event* event);

  void UpdateUniqueComposites(Material* material, UniqueCompositeOp::Enum uniqueCompositeOp);

  void CompileShaders();

  typedef InList<GraphicsSpace, &GraphicsSpace::EngineLink> GraphicsSpaceList;
  GraphicsSpaceList mSpaces;

  // Render data
  RenderQueues mRenderQueues[2];
  RenderTasks mRenderTasks[2];
  // GraphicsEngine writes to back, Renderer reads from front
  RenderQueues* mRenderQueuesBack;
  RenderTasks* mRenderTasksBack;
  RenderQueues* mRenderQueuesFront;
  RenderTasks* mRenderTasksFront;

  bool mNewLibrariesCommitted;

  uint mFrameCounter;

  uint mRenderGroupCount;
  bool mUpdateRenderGroupCount;

  HandleOf<Cog> mProjectCog;
  bool mVerticalSync;

  bool mEngineShutdown;

  RenderTargetManager mRenderTargetManager;

  LightningShaderGenerator* mShaderGenerator;

  Array<Resource*> mRenderGroups;

  Array<Material*> mAddedMaterials;
  Array<Material*> mAddedMaterialsForComposites;
  Array<RenderGroup*> mAddedRenderGroups;

  // Renderer thread
  Thread mRendererThread;
  RendererThreadJobQueue* mRendererJobQueue;
  DoRenderTasksJob* mDoRenderTasksJob;
  RendererJobQueue* mReturnJobQueue;
  ShowProgressJob* mShowProgressJob;

  // Shader building
  ShaderMap mCompositeShaders;
  ShaderMap mPostProcessShaders;

  ShaderSetMap mShaderCoreVertexMap;
  ShaderSetMap mShaderCompositeMap;
  ShaderSetMap mShaderRenderPassMap;

  HashMap<String, UniqueComposite> mUniqueComposites;

  HashSet<String> mModifiedComposites;
  HashSet<String> mRemovedComposites;

  Array<String> mModifiedFragmentFiles;
  Array<String> mRemovedFragmentFiles;

  Array<String> mModifiedCoreVertex;
  Array<String> mModifiedRenderPass;
  Array<String> mModifiedPostProcess;

  Array<String> mRemovedCoreVertex;
  Array<String> mRemovedRenderPass;
  Array<String> mRemovedPostProcess;

  // Map of meta properties with the ShaderInput attribute
  HashMap<String, Array<ShaderMetaProperty>> mComponentShaderProperties;

  Array<TextureToFile> mDelayedTextureToFile;
};

class SaveToImageJob : public Job
{
public:
  void Execute() override;
  byte* mImage;
  uint mWidth;
  uint mHeight;
  TextureFormat::Enum mFormat;
  ImageSaveFormat::Enum mImageType;
  String mFilename;
};

} // namespace Plasma
