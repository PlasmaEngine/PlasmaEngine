// MIT Licensed (see LICENSE.md).

#pragma once

namespace Plasma
{

LightningShaderGenerator* CreateLightningShaderGenerator();

DeclareEnum5(LightningFragmentType, CoreVertex, RenderPass, PostProcess, Protected, Fragment);
typedef HashMap<String, LightningFragmentType::Enum> LightningFragmentTypeMap;

// User data written per shader type. Used to deal with efficient re-compiling
// of fragments/shaders.
class FragmentUserData
{
public:
  FragmentUserData(){};
  FragmentUserData(StringParam resourceName) : mResourceName(resourceName){};
  String mResourceName;
};

class LightningShaderGenerator : public Lightning::EventHandler
{
public:
  typedef HashMap<LibraryRef, LightningShaderIRLibraryRef> ExternalToInternalLibraryMap;
  typedef ExternalToInternalLibraryMap::valuerange InternalLibraryRange;
  typedef LightningShaderIRCompositor::ShaderDefinition ShaderDefinition;
  typedef Lightning::Ref<ShaderTranslationPassResult> TranslationPassResultRef;
  typedef Lightning::Ref<SimplifiedShaderReflectionData> SimplifiedReflectionRef;
  typedef Lightning::Ref<LightningSpirVFrontEnd> LightningSpirVFrontEndRef;

  LightningShaderGenerator();
  ~LightningShaderGenerator();

  void Initialize();
  void InitializeSpirV();

  LibraryRef BuildFragmentsLibrary(Module& dependencies,
                                   Array<LightningDocumentResource*>& fragments,
                                   StringParam libraryName = "Fragments");
  bool Commit(LightningCompileEvent* e);
  void MapFragmentTypes();

  bool BuildShaders(ShaderSet& shaders,
                    HashMap<String, UniqueComposite>& composites,
                    Array<ShaderEntry>& shaderEntries,
                    Array<ShaderDefinition>* compositeShaderDefs = nullptr);
  bool CompilePipeline(LightningShaderIRType* shaderType,
                       ShaderPipelineDescription& pipeline,
                       Array<TranslationPassResultRef>& pipelineResults);

  ShaderInput
  CreateShaderInput(StringParam fragmentName, StringParam inputName, ShaderInputType::Enum type, AnyParam value);

  void OnLightningFragmentCompilationError(Lightning::ErrorEvent* event);
  void OnLightningFragmentTypeParsed(Lightning::ParseEvent* event);
  void OnLightningFragmentTranslationError(TranslationErrorEvent* event);
  void OnLightningFragmentValidationError(ValidationErrorEvent* event);

  LightningShaderIRLibraryRef GetInternalLibrary(LibraryRef library);
  LightningShaderIRLibraryRef GetCurrentInternalLibrary(LibraryRef library);
  LightningShaderIRLibraryRef GetPendingInternalLibrary(LibraryRef library);
  InternalLibraryRange GetCurrentInternalLibraries();
  LightningShaderIRLibraryRef GetCurrentInternalProjectLibrary();
  LightningShaderIRLibraryRef GetPendingInternalProjectLibrary();

  // The shared settings to be used through all of the compositing/translation
  LightningShaderSpirVSettingsRef mSpirVSettings;
  // The front-end translator used.
  LightningSpirVFrontEndRef mFrontEndTranslator;

  // Core libraries built once.
  LightningShaderIRLibraryRef mCoreLibrary;
  LightningShaderIRLibraryRef mShaderIntrinsicsLibrary;

  LightningShaderIRProject mFragmentsProject;

  Array<String> mCoreVertexFragments;
  Array<String> mRenderPassFragments;

  LightningFragmentTypeMap mFragmentTypes;

  // Plasma libraries to internal libraries.
  ExternalToInternalLibraryMap mCurrentToInternal;
  ExternalToInternalLibraryMap mPendingToPendingInternal;

  HashMap<Library*, LightningFragmentTypeMap> mPendingFragmentTypes;

  HashMap<String, u32> mSamplerAttributeValues;
};

} // namespace Plasma
