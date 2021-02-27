// MIT Licensed (see LICENSE.md).

#pragma once

namespace Plasma
{

/// Lightning shader fragment file Resource.
class LightningFragment : public LightningDocumentResource
{
public:
  LightningDeclareType(LightningFragment, TypeCopyMode::ReferenceType);

  LightningFragment();

  // DocumentResource Interface.
  void ReloadData(StringRange data) override;

  // ICodeInspector Interface.
  void AttemptGetDefinition(ICodeEditor* editor, size_t cursorPosition, CodeDefinition& definition) override;

  void GetKeywords(Array<Completion>& keywordsOut);
  void AddKeywords(Array<Completion>& keywordsOut, const Array<String>& keyswords, HashSet<String>& keywordsToSkip);
  void AddKeywords(Array<Completion>& keywordsOut, const HashMap<String, AttributeInfo>& keyswordsToTest);

  // LightningDocumentResource Interface.
  void GetLibraries(Array<Lightning::LibraryRef>& libraries) override;
  void GetLibrariesRecursive(Array<LibraryRef>& libraries, ResourceLibrary* library);
};

class LightningFragmentLoader : public ResourceLoader
{
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override;
  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) override;
  void ReloadFromFile(Resource* resource, ResourceEntry& entry) override;
};

class LightningFragmentManager : public ResourceManager
{
public:
  DeclareResourceManager(LightningFragmentManager, LightningFragment);

  LightningFragmentManager(BoundType* resourceType);
  ~LightningFragmentManager();

  /// ResourceManager Interface
  void ValidateNewName(Status& status, StringParam name, BoundType* optionalType);
  void ValidateRawName(Status& status, StringParam name, BoundType* optionalType);

  /// Get the template file for the requested resource type
  String GetTemplateSourceFile(ResourceAdd& resourceAdd) override;

  /// Helper to dispatch script errors.
  void DispatchScriptError(StringParam eventId,
                           StringParam shortMessage,
                           StringParam fullMessage,
                           const Lightning::CodeLocation& location);

  // We ignore duplicate exceptions until the version is incremented
  HashSet<String> mDuplicateExceptions;
  int mLastExceptionVersion;
};

} // namespace Plasma
