// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(ImportOptionsModified);
}

String SanitizeContentFilename(StringParam filename)
{
  String sanitizedName = Cog::SanitizeName(FilePath::GetFileNameWithoutExtension(filename));
  return BuildString(sanitizedName, ".", FilePath::GetExtension(filename));
}

LightningDefineType(ImageOptions, builder, type)
{
  // These options are referred to directly by pointer on the import options
  // (unsafe for script)
  type->HandleManager = LightningManagerId(PointerManager);

  PlasmaBindExpanded();
  LightningBindFieldProperty(mImportImages)->AddAttribute(PropertyAttributes::cInvalidatesObject);
}

ImageOptions::ImageOptions(ImportOptions* owner) : mOwner(owner)
{
  mImportImages = ImageImport::Textures;
}

LightningDefineType(GeometryOptions, builder, type)
{
  // These options are referred to directly by pointer on the import options
  // (unsafe for script)
  type->HandleManager = LightningManagerId(PointerManager);

  PlasmaBindExpanded();

  LightningBindFieldProperty(mImportMeshes)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  LightningBindFieldProperty(mGenerateSmoothNormals)
      ->AddAttributeChainable(PropertyAttributes::cInvalidatesObject)
      ->PlasmaFilterBool(mImportMeshes);
  LightningBindFieldProperty(mSmoothingAngleDegreesThreshold)->Add(new ShowNormalGenerationOptionsFilter());
  LightningBindFieldProperty(mGenerateTangentSpace)->PlasmaFilterBool(mImportMeshes);
  LightningBindFieldProperty(mInvertUvYAxis)->PlasmaFilterBool(mImportMeshes);
  LightningBindFieldProperty(mFlipWindingOrder)->PlasmaFilterBool(mImportMeshes);
  LightningBindFieldProperty(mPhysicsImport)->PlasmaFilterBool(mImportMeshes);

  LightningBindFieldProperty(mCollapsePivots);
  LightningBindFieldProperty(mImportAnimations);
  LightningBindFieldProperty(mCreateArchetype);
  LightningBindFieldProperty(mImportTextures);

  LightningBindFieldProperty(mOriginOffset);
  LightningBindFieldProperty(mScaleConversion)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  LightningBindFieldProperty(mScaleFactor)
      ->PlasmaFilterEquality(mScaleConversion, ScaleConversion::Enum, ScaleConversion::Custom);
  LightningBindFieldProperty(mChangeBasis)->AddAttribute(PropertyAttributes::cInvalidatesObject);

  LightningBindFieldProperty(mXBasisTo)->PlasmaFilterBool(mChangeBasis);
  LightningBindFieldProperty(mYBasisTo)->PlasmaFilterBool(mChangeBasis);
  LightningBindFieldProperty(mZBasisTo)->PlasmaFilterBool(mChangeBasis);
}

GeometryOptions::GeometryOptions(ImportOptions* owner) :
    mOwner(owner),
    mImportMeshes(true),
    mCombineMeshes(false),
    mOriginOffset(0.f, 0.f, 0.f),
    mScaleConversion(ScaleConversion::Custom),
    mScaleFactor(1),
    mChangeBasis(false),
    mXBasisTo(BasisType::PositiveX),
    mYBasisTo(BasisType::PositiveY),
    mZBasisTo(BasisType::PositiveZ),
    mGenerateSmoothNormals(false),
    mSmoothingAngleDegreesThreshold(30.f),
    mGenerateTangentSpace(true),
    mInvertUvYAxis(false),
    mFlipWindingOrder(false),
    mPhysicsImport(PhysicsImport::NoMesh),
    mCollapsePivots(false),
    mImportAnimations(false),
    mCreateArchetype(true),
    mImportTextures(false)
{
}

// ShowNormalGenerationOptionsFilter
LightningDefineType(ShowNormalGenerationOptionsFilter, builder, type)
{
}

bool ShowNormalGenerationOptionsFilter::Filter(Member* prop, HandleParam instance)
{
  GeometryOptions* options = instance.Get<GeometryOptions*>(GetOptions::AssertOnNull);
  return options->mImportMeshes && options->mGenerateSmoothNormals;
}

LightningDefineType(AudioOptions, builder, type)
{
  // These options are referred to directly by pointer on the import options
  // (unsafe for script)
  type->HandleManager = LightningManagerId(PointerManager);

  PlasmaBindExpanded();
  LightningBindFieldProperty(mGenerateCue);
  LightningBindFieldProperty(mGroupCueName);
  LightningBindFieldProperty(mStreamingMode);
}

AudioOptions::AudioOptions(ImportOptions* owner) : mOwner(owner)
{
  mGenerateCue = AudioCueImport::None;
  mGroupCueName = "NoName";
  mStreamingMode = AudioFileLoadType::Auto;
}

LightningDefineType(ConflictOptions, builder, type)
{
  // These options are referred to directly by pointer on the import options
  // (unsafe for script)
  type->HandleManager = LightningManagerId(PointerManager);

  PlasmaBindExpanded();
  LightningBindGetterSetterProperty(Action);
}

ConflictOptions::ConflictOptions(ImportOptions* owner) : mOwner(owner)
{
  mAction = ConflictAction::Replace;
}

void ConflictOptions::SetAction(ConflictAction::Enum action)
{
  mAction = action;
  EventDispatcher* dispatcher = mOwner->GetDispatcher();
  Event e;
  dispatcher->Dispatch(Events::ImportOptionsModified, &e);
}

ConflictAction::Enum ConflictOptions::GetAction()
{
  return mAction;
}

LightningDefineType(ImportOptions, builder, type)
{
  // These options are referred to directly by pointer on the import options
  // (unsafe for script)
  type->HandleManager = LightningManagerId(PointerManager);

  LightningBindFieldProperty(mImageOptions);
  LightningBindFieldProperty(mGeometryOptions);
  LightningBindFieldProperty(mAudioOptions);
  LightningBindFieldProperty(mConflictOptions);
  PlasmaBindExpanded();
}

ImportOptions::ImportOptions()
{
  mImageOptions = nullptr;
  mGeometryOptions = nullptr;
  mAudioOptions = nullptr;
  mConflictOptions = nullptr;
  mLibrary = nullptr;
}

ImportOptions::~ImportOptions()
{
  SafeDelete(mImageOptions);
  SafeDelete(mGeometryOptions);
  SafeDelete(mAudioOptions);
  SafeDelete(mConflictOptions);
}

void ImportOptions::Initialize(Array<String>& files, ContentLibrary* library)
{
  mLibrary = library;
  Array<String> invalidFiles;

  for (uint i = 0; i < files.Size(); ++i)
  {
    String fullPath = files[i];

    // Strip the path
    String originalFilename = FilePath::GetFileName(fullPath.All());
    String fileName = SanitizeContentFilename(originalFilename);

    // Check to see if the filename contained any valid characters
    if (fileName == Lightning::EmptyUpperIdentifier && !originalFilename.Contains(Lightning::EmptyUpperIdentifier))
    {
      invalidFiles.PushBack(originalFilename);
      continue;
    }

    // Check to see if it already exists
    if (mLibrary->FindContentItemByFileName(fileName))
      mConflictedFiles.PushBack(fullPath);
    else
      mFiles.PushBack(fullPath);
  }

  // If there are any invalid filenames create a do notify error message
  if (!invalidFiles.Empty())
  {
    StringBuilder builder;
    builder.Append(invalidFiles.Front());
    for (unsigned i = 1; i < invalidFiles.Size(); ++i)
      builder.AppendFormat(", %s", invalidFiles[i].c_str());

    String errorMessage =
        String::Format("The following files do not contain any valid characters: %s", builder.ToString().c_str());
    DoNotifyError("File Import Failed", errorMessage);
  }

  BuildOptions();
}

template <typename ClassType>
void BuildContentOptions(ClassType** object, ImportOptions* owner, HashSet<String>& set, StringParam ContentType)
{
  if (set.FindValue(ContentType, "None") != "None")
  {
    if (*object == nullptr)
      *object = new ClassType(owner);
  }
  else
  {
    SafeDelete(*object);
  }
}

void InsertExtension(Array<String>& files, HashSet<String>& set)
{
  for (uint i = 0; i < files.Size(); ++i)
  {
    // Get the file extension
    String file = FilePath::GetFileName(files[i].All());
    String extension = FilePath::GetExtension(file);
    extension = extension.ToLower();

    // Check known extensions
    ContentTypeEntry* entry = PL::gContentSystem->CreatorsByExtension.FindPointer(extension);
    if (entry)
      set.Insert(entry->Meta->Name);
  }
}

void ImportOptions::BuildOptions()
{
  // We want to find unique content types (Image, Geometry, Audio)
  HashSet<String> fileSet;

  InsertExtension(mFiles, fileSet);
  if (!mConflictOptions && mConflictedFiles.Size() > 0)
    mConflictOptions = new ConflictOptions(this);

  if (mConflictOptions)
    InsertExtension(mConflictedFiles, fileSet);

  // prepare to build the new files we are importing
  BuildContentOptions(&mImageOptions, this, fileSet, "ImageContent");
  BuildContentOptions(&mGeometryOptions, this, fileSet, "GeometryContent");
  BuildContentOptions(&mAudioOptions, this, fileSet, "AudioContent");
}

bool ImportOptions::ShouldAutoImport()
{
  // If any options are present, we cannot auto import
  return !(mImageOptions || mGeometryOptions || mAudioOptions || mConflictOptions);
}

} // namespace Plasma
