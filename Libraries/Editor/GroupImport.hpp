// MIT Licensed (see LICENSE.md).
#pragma once
#include "Engine/EngineEvents.hpp"

namespace Plasma
{   
void RunGroupImport(ImportOptions& options);
void GroupImport();
void OpenGroupImport(Array<String>& files);
void LoadDroppedFiles(Array<HandleOfString>& files);

// GroupImportWindow
class GroupImportWindow : public Composite
{
public:
  typedef GroupImportWindow LightningSelf;

  ImportOptions* mOptions;
  PropertyView* mPropertyView;
  TextButton* mImportButton;
  TextButton* mCancelButton;
  ListBox* mListBox;
  Composite* mParentWindow;
  StringSource mStrings;

  GroupImportWindow(Composite* parent, ImportOptions* options);
  float GetPropertyGridHeight();
  void OnOptionsModified(Event* event);
  void RebuildTree();
  void UpdateListBoxSource();
  void OnPressed(Event* event);
  void OnCancel(Event* event);
};

// ImportCallback
class ImportCallback : public SafeId32EventObject
{
public:
  typedef ImportCallback LightningSelf;

  void Open();
  void OnFilesSelected(OsFileSelection* fileSelection);
};

struct ImportJobProperties : Object
{
public:
    LightningDeclareType(ImportJobProperties, TypeCopyMode::ReferenceType);
    
    ImportJobProperties();
    
    ContentLibrary* mLibrary;
    ResourceLibrary* mResourceLibrary;
    Array<ContentItem*> mContentToBuild;
    ImportOptions* mOptions;
};
    
 class ImportJob : public BackgroundTaskJob
 {
 public:
     typedef ImportJob LightningSelf;

     ImportJob(ImportJobProperties jobProperties);

     /// Job Interface.
     void Execute() override;
     int Cancel() override;

     void UpdateTaskProgress(float percentComplete, StringParam progressText);
     void OnImportFinished(PostImportEvent* e);

     ImportJobProperties mJobProperties;

 private:
     HandleOf<ResourcePackage> BuildContentItems(Status& status, ContentItemArray& toBuild, ContentLibrary* library);
 };
} // namespace Plasma
