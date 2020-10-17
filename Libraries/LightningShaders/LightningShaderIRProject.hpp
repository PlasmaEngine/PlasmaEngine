// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class BaseShaderIRTranslator;

/// A project full of lightning code to be compiled into a library.
class LightningShaderIRProject : public ShaderCompilationErrors
{
public:
  LightningShaderIRProject(StringParam projectName);

  void AddCodeFromString(StringParam code, StringParam codeLocation, void* userData = nullptr);
  void AddCodeFromFile(StringParam filePath, void* userData = nullptr);

  /// Clear the code from this library
  void Clear();

  bool CompileTree(Lightning::Module& lightningDependencies,
                   Lightning::LibraryRef& lightningLibrary,
                   Lightning::SyntaxTree& syntaxTree,
                   Lightning::Array<Lightning::UserToken>& tokensOut);
  /// Compiles and translates this project into a library.
  LightningShaderIRLibraryRef CompileAndTranslate(LightningShaderIRModuleRef& dependencies, BaseShaderIRTranslator* translator);

  struct CodeEntry
  {
    String mCode;
    String mCodeLocation;
    void* mUserData;
  };

  // Private Interface
  void BuildLightningProject(Lightning::Project& lightningProject);
  void PopulateLightningModule(Lightning::Module& lightningDependencies, LightningShaderIRModuleRef& dependencies);
  void CollectLibraryDefaultValues(LightningShaderIRLibraryRef libraryRef, Lightning::Module& lightningModule);

  Array<CodeEntry> mCodeEntries;
  String mProjectName;
  /// Has this project been successfully compiled into a library? (Not really
  /// used at the moment...)
  bool mCompiledSuccessfully;

  /// A pointer to any data the user wants to attach
  mutable const void* UserData;

  /// Any user data that cant simply be represented by a pointer
  /// Data can be written to the buffer and will be properly destructed
  /// when this object is destroyed (must be read in the order it's written)
  mutable Lightning::DestructibleBuffer ComplexUserData;
};

} // namespace Plasma
