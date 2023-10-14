// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(GeometryResourceEntry, builder, type)
{
  type->HandleManager = LightningManagerId(PointerManager);
}

void GeometryResourceEntry::Serialize(Serializer& stream)
{
  SerializeNameDefault(mName, String());
  SerializeNameDefault(mResourceId, (ResourceId)0);
}

void GeometryResourceEntry::SetDefaults()
{
  mName = String();
  mResourceId = 0;
}

bool GeometryResourceEntry::operator==(const GeometryResourceEntry& other)
{
  return mName == other.mName;
}

Vec3 GetBasisVector(BasisType::Enum basisEnum)
{
  switch (basisEnum)
  {
  case BasisType::PositiveX:
    return Vec3(1.f, 0.f, 0.f);
  case BasisType::NegativeX:
    return Vec3(-1.f, 0.f, 0.f);
  case BasisType::PositiveY:
    return Vec3(0.f, 1.f, 0.f);
  case BasisType::NegativeY:
    return Vec3(0.f, -1.f, 0.f);
  case BasisType::PositiveZ:
    return Vec3(0.f, 0.f, 1.f);
  case BasisType::NegativeZ:
    return Vec3(0.f, 0.f, -1.f);
  }
  return Vec3();
}

void SetGeometryContentImportOptions(GeometryImport* importOptions, GeometryOptions* geoOptions)
{
  importOptions->mCollapsePivots = geoOptions->mCollapsePivots;
  importOptions->mOriginOffset = geoOptions->mOriginOffset;
  importOptions->mScaleFactor = geoOptions->mScaleFactor;
  importOptions->mChangeBasis = geoOptions->mChangeBasis;
  importOptions->mXBasisTo = geoOptions->mXBasisTo;
  importOptions->mYBasisTo = geoOptions->mYBasisTo;
  importOptions->mZBasisTo = geoOptions->mZBasisTo;
}

void SetGeometryContentMeshBuilderOptions(MeshBuilder* meshBuilder, GeometryOptions* geoOptions)
{
  meshBuilder->mGenerateSmoothNormals = geoOptions->mGenerateSmoothNormals;
  meshBuilder->mSmoothingAngleDegreesThreshold = geoOptions->mSmoothingAngleDegreesThreshold;
  meshBuilder->mGenerateTangentSpace = geoOptions->mGenerateTangentSpace;
  meshBuilder->mInvertUvYAxis = geoOptions->mInvertUvYAxis;
  meshBuilder->mFlipWindingOrder = geoOptions->mFlipWindingOrder;
}

void SetGeometryContentPhysicsMeshBuilderOptions(PhysicsMeshBuilder* physicsBuilder, GeometryOptions* geoOptions)
{
  if (geoOptions->mPhysicsImport == PhysicsImport::StaticMesh)
    physicsBuilder->MeshBuilt = PhysicsMeshType::PhysicsMesh;
  else
    physicsBuilder->MeshBuilt = PhysicsMeshType::ConvexMesh;
}

ContentItem* MakeGeometryContent(ContentInitializer& initializer)
{
  String filename = initializer.Filename;
  String metaFile = FilePath::CombineWithExtension(initializer.Library->SourcePath, filename, ".meta");

  GeometryContent* newGeo = new GeometryContent();

  // Always add Geometry Import
  GeometryImport* import = new GeometryImport();
  import->Generate(initializer);
  newGeo->AddComponent(import);

  if (initializer.Options == nullptr)
  {
    // Just add mesh and return
    MeshBuilder* meshBuilder = new MeshBuilder();
    meshBuilder->Generate(initializer);
    newGeo->AddComponent(meshBuilder);
    return newGeo;
  }

  ImportOptions* options = initializer.Options;
  SetGeometryContentImportOptions(import, options->mGeometryOptions);

  // initializer.Options is present
  if (options->mGeometryOptions->mImportMeshes)
  {
    GeometryOptions* geoOptions = options->mGeometryOptions;
    MeshBuilder* meshBuilder = new MeshBuilder();
    meshBuilder->Generate(initializer);

    // builder options
    // meshBuilder->mCombineMeshes = geoOptions->mCombineMeshes;
    SetGeometryContentMeshBuilderOptions(meshBuilder, geoOptions);

    newGeo->AddComponent(meshBuilder);
  }

  // PhysicsImport::Enum physicsImportType =
  // options->mGeometryOptions->mPhysicsImport;
  if (options->mGeometryOptions->mPhysicsImport != PhysicsImport::NoMesh)
  {
    PhysicsMeshBuilder* meshBuilder = new PhysicsMeshBuilder();
    meshBuilder->Generate(initializer);

    SetGeometryContentPhysicsMeshBuilderOptions(meshBuilder, options->mGeometryOptions);
    newGeo->AddComponent(meshBuilder);
  }

  if (options->mGeometryOptions->mImportAnimations)
  {
    AnimationBuilder* animationBuilder = new AnimationBuilder();
    animationBuilder->Generate(initializer);
    newGeo->AddComponent(animationBuilder);
  }

  if (options->mGeometryOptions->mCreateArchetype)
  {
    GeneratedArchetype* archetypeGeometry = new GeneratedArchetype();
    archetypeGeometry->Generate(initializer);
    newGeo->AddComponent(archetypeGeometry);
  }

  if (options->mGeometryOptions->mImportTextures)
  {
    TextureContent* textureContent = new TextureContent();
    textureContent->Generate(initializer);
    newGeo->AddComponent(textureContent);
  }

  if (options->mGeometryOptions->mImportMaterials)
  {
      MaterialContent* materialContent = new MaterialContent();
      materialContent->Generate(initializer);
      newGeo->AddComponent(materialContent);
  }

  return newGeo;
}

ContentItem* UpdateGeometryContent(ContentItem* existingItem, ContentInitializer& initializer)
{
  String filename = initializer.Filename;
  String metaFile = FilePath::CombineWithExtension(initializer.Library->SourcePath, filename, ".meta");

  if (!FileExists(metaFile))
    return nullptr;

  GeometryContent* geometryContent = Type::DebugOnlyDynamicCast<GeometryContent*>(existingItem);

  // we have loaded our existing geometry content and need to update the meta
  // file for our rebuild
  GeometryImport* import = geometryContent->has(GeometryImport);

  if (initializer.Options == nullptr && !geometryContent->has(MeshBuilder))
  {
    // Just add mesh and return
    MeshBuilder* meshBuilder = new MeshBuilder();
    meshBuilder->Generate(initializer);
    geometryContent->AddComponent(meshBuilder);
    return geometryContent;
  }

  ImportOptions* options = initializer.Options;
  SetGeometryContentImportOptions(import, options->mGeometryOptions);

  // initializer.Options is present
  if (options->mGeometryOptions->mImportMeshes && !geometryContent->has(MeshBuilder))
  {
    GeometryOptions* geoOptions = options->mGeometryOptions;
    MeshBuilder* meshBuilder = new MeshBuilder();
    meshBuilder->Generate(initializer);

    // builder options
    // meshBuilder->mCombineMeshes = geoOptions->mCombineMeshes;
    SetGeometryContentMeshBuilderOptions(meshBuilder, geoOptions);
    geometryContent->AddComponent(meshBuilder);
  }
  else if (!options->mGeometryOptions->mImportMeshes && geometryContent->has(MeshBuilder))
  {
    // we no longer have this option selected, delete it
    geometryContent->RemoveComponent(LightningTypeId(MeshBuilder));
  }
  else if (geometryContent->has(MeshBuilder))
  {
    GeometryOptions* geoOptions = options->mGeometryOptions;
    MeshBuilder* meshBuilder = geometryContent->has(MeshBuilder);
    SetGeometryContentMeshBuilderOptions(meshBuilder, geoOptions);
  }

  if ((options->mGeometryOptions->mPhysicsImport != PhysicsImport::NoMesh) && !geometryContent->has(PhysicsMeshBuilder))
  {
    PhysicsMeshBuilder* meshBuilder = new PhysicsMeshBuilder();
    meshBuilder->Generate(initializer);

    SetGeometryContentPhysicsMeshBuilderOptions(meshBuilder, options->mGeometryOptions);
    geometryContent->AddComponent(meshBuilder);
  }
  else if ((options->mGeometryOptions->mPhysicsImport == PhysicsImport::NoMesh) &&
           geometryContent->has(PhysicsMeshBuilder))
  {
    // we no longer have this option selected, delete it
    geometryContent->RemoveComponent(LightningTypeId(PhysicsMeshBuilder));
  }
  else if (geometryContent->has(PhysicsMeshBuilder))
  {
    PhysicsMeshBuilder* meshBuilder = geometryContent->has(PhysicsMeshBuilder);
    SetGeometryContentPhysicsMeshBuilderOptions(meshBuilder, options->mGeometryOptions);
  }

  if (options->mGeometryOptions->mImportAnimations && !geometryContent->has(AnimationBuilder))
  {
    AnimationBuilder* animationBuilder = new AnimationBuilder();
    animationBuilder->Generate(initializer);
    geometryContent->AddComponent(animationBuilder);
  }
  else if (!options->mGeometryOptions->mImportAnimations && geometryContent->has(AnimationBuilder))
  {
    // we no longer have this option selected, delete it
    geometryContent->RemoveComponent(LightningTypeId(AnimationBuilder));
  }

  if (options->mGeometryOptions->mCreateArchetype && !geometryContent->has(GeneratedArchetype))
  {
    GeneratedArchetype* archetypeGeometry = new GeneratedArchetype();
    archetypeGeometry->Generate(initializer);
    geometryContent->AddComponent(archetypeGeometry);
  }
  else if (!options->mGeometryOptions->mCreateArchetype && geometryContent->has(GeneratedArchetype))
  {
    // we no longer have this option selected, delete it;
    geometryContent->RemoveComponent(LightningTypeId(GeneratedArchetype));
  }

  if (options->mGeometryOptions->mImportTextures && !geometryContent->has(TextureContent))
  {
    TextureContent* textureContent = new TextureContent();
    textureContent->Generate(initializer);
    geometryContent->AddComponent(textureContent);
  }
  else if (!options->mGeometryOptions->mImportTextures && geometryContent->has(TextureContent))
  {
    // we no longer have this option selected, delete it
    geometryContent->RemoveComponent(LightningTypeId(TextureContent));
  }

  if (options->mGeometryOptions->mImportMaterials && !geometryContent->has(MaterialContent))
  {
      MaterialContent* materialContent = new MaterialContent();
      materialContent->Generate(initializer);
      geometryContent->AddComponent(materialContent);
  }
  else if (!options->mGeometryOptions->mImportMaterials && geometryContent->has(MaterialContent))
  {
      // we no longer have this option selected, delete it
      geometryContent->RemoveComponent(LightningTypeId(MaterialContent));
  }

  return geometryContent;
}

LightningDefineType(GeometryContent, builder, type)
{
}

GeometryContent::GeometryContent()
{
  EditMode = ContentEditMode::ContentItem;
}

GeometryContent::GeometryContent(StringParam inputFilename)
{
  EditMode = ContentEditMode::ContentItem;
  Filename = FilePath::GetFileName(inputFilename);
}

LightningDefineType(GeometryImport, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);
  PlasmaBindDependency(GeometryContent);

  LightningBindFieldProperty(mCollapsePivots);
  LightningBindFieldProperty(mOriginOffset);
  LightningBindFieldProperty(mScaleFactor);
  LightningBindFieldProperty(mChangeBasis)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  LightningBindFieldProperty(mXBasisTo)->PlasmaFilterEquality(mChangeBasis, bool, true);
  LightningBindFieldProperty(mYBasisTo)->PlasmaFilterEquality(mChangeBasis, bool, true);
  LightningBindFieldProperty(mZBasisTo)->PlasmaFilterEquality(mChangeBasis, bool, true);
}

void GeometryImport::Serialize(Serializer& stream)
{
  SerializeNameDefault(mCollapsePivots, false);
  SerializeNameDefault(mOriginOffset, Vec3::cZero);
  SerializeNameDefault(mScaleFactor, 1.0f);
  SerializeNameDefault(mChangeBasis, false);
  SerializeEnumNameDefault(BasisType, mXBasisTo, BasisType::PositiveX);
  SerializeEnumNameDefault(BasisType, mYBasisTo, BasisType::PositiveY);
  SerializeEnumNameDefault(BasisType, mZBasisTo, BasisType::PositiveZ);
}

void GeometryImport::Generate(ContentInitializer& initializer)
{
  mCollapsePivots = false;
  mOriginOffset = Vec3::cZero;
  mScaleFactor = 1.0f;
  mChangeBasis = false;
  mXBasisTo = BasisType::PositiveX;
  mYBasisTo = BasisType::PositiveY;
  mZBasisTo = BasisType::PositiveZ;
}

void GeometryImport::ComputeTransforms()
{
  mChangeOfBasis = Mat3::cIdentity;
  if (mChangeBasis)
  {
    mChangeOfBasis.SetBasis(0, GetBasisVector(mXBasisTo));
    mChangeOfBasis.SetBasis(1, GetBasisVector(mYBasisTo));
    mChangeOfBasis.SetBasis(2, GetBasisVector(mZBasisTo));
    mChangeOfBasis.Transpose();
    if (!mChangeOfBasis.SafeInvert())
    {
      PlasmaPrint("Geometry Processor: Change of basis invalid. Falling back to no "
             "change of basis.\n");
      mChangeOfBasis.SetIdentity();
    }
  }

  mTransform.BuildTransform(mOriginOffset, mChangeOfBasis, Vec3(mScaleFactor));
  mNormalTransform.BuildTransform(mChangeOfBasis, Vec3(1.0f / mScaleFactor));
}

LightningDefineType(PhysicsMeshBuilder, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);
  PlasmaBindDependency(GeometryContent);
  PlasmaBindDependency(MeshBuilder);

  LightningBindGetterSetterProperty(MeshBuilt);

  type->CreatableInScript = false;
}

bool PhysicsMeshBuilder::NeedsBuilding(BuildOptions& options)
{
  if (Meshes.Empty())
    return true;

  // Just check the first file
  String outputFile = GetOutputFile(0);
  String destFile = FilePath::Combine(options.OutputPath, outputFile);
  String sourceFile = FilePath::Combine(options.SourcePath, mOwner->Filename);
  return CheckFileAndMeta(options, sourceFile, destFile);
}

const String NormalMeshExtension = ".physmesh";
const String ConvexMeshExtension = ".convexmesh";

String PhysicsMeshBuilder::GetOutputFile(uint index)
{
  String extension = NormalMeshExtension;
  if (MeshBuilt == PhysicsMeshType::ConvexMesh)
    extension = ConvexMeshExtension;

  return BuildString(Meshes[index].mName, extension);
}

void PhysicsMeshBuilder::Generate(ContentInitializer& initializer)
{
  MeshBuilt = PhysicsMeshType::PhysicsMesh;
}

void PhysicsMeshBuilder::SetMeshBuilt(PhysicsMeshType::Enum type)
{
  // If they're different types, we need to re-generate Id's for each entry
  // We need to re-generate it so that when the content is reloaded, it will
  // be reloaded into new resources (as the type has changed)
  if (MeshBuilt != type)
  {
    ResourceId baseId = GenerateUniqueId64();

    for (uint i = 0; i < Meshes.Size(); ++i)
    {
      GeometryResourceEntry& entry = Meshes[i];

      // Increment each
      entry.mResourceId = baseId + i;
    }
  }

  MeshBuilt = type;
}

PhysicsMeshType::Enum PhysicsMeshBuilder::GetMeshBuilt()
{
  return MeshBuilt;
}

void PhysicsMeshBuilder::Serialize(Serializer& stream)
{
  SerializeNameDefault(Meshes, Array<GeometryResourceEntry>());
  SerializeEnumName(PhysicsMeshType, MeshBuilt);
}

void PhysicsMeshBuilder::BuildListing(ResourceListing& listing)
{
  for (uint i = 0; i < Meshes.Size(); ++i)
  {
    GeometryResourceEntry& entry = Meshes[i];
    String outputFile = GetOutputFile(i);

    if (MeshBuilt == PhysicsMeshType::PhysicsMesh)
      listing.PushBack(ResourceEntry(0, "PhysicsMesh", entry.mName, outputFile, entry.mResourceId, this->mOwner, this));
    else
      listing.PushBack(ResourceEntry(0, "ConvexMesh", entry.mName, outputFile, entry.mResourceId, this->mOwner, this));
  }
}

LightningDefineType(AnimationClip, builder, type)
{
  type->HandleManager = LightningManagerId(PointerManager);
  LightningBindFieldProperty(mName);
  LightningBindFieldProperty(mStartFrame);
  LightningBindFieldProperty(mEndFrame);
  LightningBindFieldProperty(mAnimationIndex);
}

void AnimationClip::Serialize(Serializer& stream)
{
  SerializeNameDefault(mName, String("Default"));
  SerializeNameDefault(mStartFrame, 0);
  SerializeNameDefault(mEndFrame, 0);
  SerializeNameDefault(mAnimationIndex, 0);
}

void AnimationClip::SetDefaults()
{
  mName = "Default";
  mStartFrame = 0;
  mEndFrame = 0;
  mAnimationIndex = 0;
}

LightningDefineType(AnimationBuilder, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);
  PlasmaBindDependency(GeometryContent);

  LightningBindFieldProperty(mClips);
}

void AnimationBuilder::Serialize(Serializer& stream)
{
  SerializeNameDefault(mClips, Array<AnimationClip>());
  SerializeNameDefault(mAnimations, Array<GeometryResourceEntry>());
}

void AnimationBuilder::Generate(ContentInitializer& initializer)
{
  Name = initializer.Name;
}

bool AnimationBuilder::NeedsBuilding(BuildOptions& options)
{
  if (mAnimations.Empty())
    return true;

  String outputFile = BuildString(mAnimations[0].mName, ".anim.bin");
  String destFile = FilePath::Combine(options.OutputPath, outputFile);
  String sourceFile = FilePath::Combine(options.SourcePath, mOwner->Filename);
  return CheckFileAndMeta(options, sourceFile, destFile);
}

void AnimationBuilder::BuildListing(ResourceListing& listing)
{
  forRange (GeometryResourceEntry& entry, mAnimations.All())
  {
    String output = BuildString(entry.mName, ".anim.bin");
    listing.PushBack(ResourceEntry(0, "AnimationBin", entry.mName, output, entry.mResourceId, this->mOwner, this));
  }
}

// Texture Content
LightningDefineType(TextureContent, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);
  PlasmaBindDependency(GeometryContent);
}

void TextureContent::Serialize(Serializer& stream)
{
  SerializeNameDefault(mTextures, Array<GeometryResourceEntry>());
}

void TextureContent::Generate(ContentInitializer& initializer)
{
}

// Material Content
LightningDefineType(MaterialContent, builder, type)
{
    PlasmaBindComponent();
    PlasmaBindSetup(SetupMode::CallSetDefaults);
    PlasmaBindDependency(GeometryContent);
}

void MaterialContent::Serialize(Serializer& stream)
{
    SerializeNameDefault(mMaterials, Array<GeometryResourceEntry>());
}

void MaterialContent::Generate(ContentInitializer& initializer)
{
}

// Geometry Content

String GeometryContent::GetName()
{
  return FilePath::GetFileNameWithoutExtension(Filename);
}

void GeometryContent::BuildContentItem(BuildOptions& options)
{
  bool needToBuild = ContentComposition::AnyNeedsBuilding(options);
  if (needToBuild)
  {
    String fullFilePath = FilePath::Combine(options.SourcePath, Filename);
    GeometryImporter importer(fullFilePath, options.OutputPath, String());
    GeometryProcessorCodes::Enum result = importer.ProcessModelFiles();

    bool needsLoading = false;
    switch (result)
    {
    // no content was present in the file
    case Plasma::GeometryProcessorCodes::NoContent:
    {
      options.Failure = true;
      options.Message = String::Format("Failed to process Geometry. File '%s' "
                                       "Error: No importable content present",
                                       Filename.c_str());

      // there was no content, remove the associated files from our content
      // directory
      String metaFile = BuildString(fullFilePath, ".meta");
      return;
    }
    // it worked
    case Plasma::GeometryProcessorCodes::Success:
      return;
    // something went wrong, abort processing imported file.
    // the geometry processor outputs its own error messages to the console.
    case Plasma::GeometryProcessorCodes::Failed:
    {
      options.Failure = true;
      options.Message = String::Format("Failed to process Geometry. File '%s', "
                                       "See above Geometry Processor Error.",
                                       Filename.c_str());

      // Get our projects content folder
      ProjectSettings* projectSettings = PL::gEngine->GetProjectSettings();
      if (projectSettings != nullptr)
      {
        String contentDirectory = FilePath::Normalize(projectSettings->GetContentFolder());

        // If importing the item failed delete the content item but only if that
        // item is in the projects local content folder since it was copied and
        // will fail to be processed each time the project is opened until the
        // user deletes the files manually. Otherwise a core resource failed and
        // should not be deleted.
        String fileDirectory = FilePath::Normalize(FilePath::GetDirectoryPath(fullFilePath));
        if (fileDirectory == contentDirectory)
        {
          ProjectSettings* projectSettings = PL::gEngine->GetProjectSettings();
          String contentDirectory = FilePath::Normalize(projectSettings->GetContentFolder());

		  String fileDirectory = FilePath::Normalize(FilePath::GetDirectoryPath(fullFilePath));
          if ( fileDirectory == contentDirectory)
          {
            String metaFile = BuildString(fullFilePath, ".meta");
            DeleteFile(metaFile);
            DeleteFile(fullFilePath);
          }
        }
      }
      return;
    }
    // let editor side importing know we have to load/reload the scene graph or
    // textures
    case Plasma::GeometryProcessorCodes::LoadGraph:
    case Plasma::GeometryProcessorCodes::LoadTextures:
    case Plasma::GeometryProcessorCodes::LoadGraphAndTextures:
      needsLoading = true;
      break;
    default:
      break;
    }

    if (needsLoading)
    {
      // we need to do more work
      // Re serialize
      ClearComponents();
      LoadFromDataFile(*this, GetMetaFilePath());
      this->OnInitialize();

      // Queue for editor processing
      mNeedsEditorProcessing = true;
    }
  }
}

LightningDefineType(GeneratedArchetype, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);
  PlasmaBindDependency(GeometryContent);
}

void GeneratedArchetype::Generate(ContentInitializer& initializer)
{
  mResourceId = GenerateUniqueId64();
}

void GeneratedArchetype::Serialize(Serializer& stream)
{
  SerializeNameDefault(mResourceId, ResourceId(0));
}

GeneratedArchetype::GeneratedArchetype()
{
}

bool GeneratedArchetype::NeedToBuildArchetype(ContentItem* item)
{
  String metaFile = mOwner->GetMetaFilePath();
  String archetypeFile = item->GetFullPath();
  bool fileOutOfDate = NeedToBuild(mOwner->GetFullPath(), archetypeFile);
  bool metaFileOutOfDate = NeedToBuild(metaFile, archetypeFile);
  return fileOutOfDate || metaFileOutOfDate;
}

void CreateGeometryContent(ContentSystem* system)
{
  AddContentComponent<GeneratedArchetype>(system);

  AddContentComponent<GeometryImport>(system);
  AddContentComponent<MeshBuilder>(system);
  AddContentComponent<PhysicsMeshBuilder>(system);
  AddContentComponent<AnimationBuilder>(system);
  AddContentComponent<TextureContent>(system);
  AddContentComponent<MaterialContent>(system);

  AddContent<GeometryContent>(system);
  // COMMON INTERCHANGE FORMATS
  // Autodesk
  system->CreatorsByExtension["fbx"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Wavefront Object
  system->CreatorsByExtension["obj"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Collada
  system->CreatorsByExtension["dae"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Global Module in Basic Program
  system->CreatorsByExtension["glb"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // 3ds Max 3DS
  system->CreatorsByExtension["3ds"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Blender 3D
  system->CreatorsByExtension["blend"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // 3ds Max ASE
  system->CreatorsByExtension["ase"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Industry Foundation Classes (IFC/Step) NEEDS ADDITIONAL TESTING TO FIGURE
  // OUT WHAT WENT WRONG
  system->CreatorsByExtension["ifc"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // XGL
  system->CreatorsByExtension["xgl"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  system->CreatorsByExtension["zgl"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Stanford Polygon Library
  system->CreatorsByExtension["ply"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // AutoCAD DXF
  system->CreatorsByExtension["dxf"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // LightWave
  system->CreatorsByExtension["lwo"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // LightWave Scene
  system->CreatorsByExtension["lws"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Modo
  system->CreatorsByExtension["lxo"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Stereolithography
  system->CreatorsByExtension["stl"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // DirectX X
  system->CreatorsByExtension["x"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // AC3D
  system->CreatorsByExtension["ac"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Milkshape 3D
  system->CreatorsByExtension["ms3d"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // TrueSpace
  system->CreatorsByExtension["cob"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // untested truespace format
  system->CreatorsByExtension["scn"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);

  // MOTION CAPTURE FORMATS
  // Biovision BVH <- ASSIMP tokenizer does not properly handle nodes names with
  // spaces in this format. system->CreatorsByExtension["bvh"] =
  // ContentTypeEntry(MetaTypeOf(GeometryContent), MakeGeometryContent);
  // CharacterStudio Motion
  system->CreatorsByExtension["csm"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);

  // GRAPHICS ENGINE FORMATS
  // Ogre XML
  system->CreatorsByExtension["xml"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Irrlicht Mesh
  // system->CreatorsByExtension["irrmesh"] =
  // ContentTypeEntry(MetaTypeOf(GeometryContent), MakeGeometryContent);
  // Irrlicht Scene <- irradiance values to apply to the above
  // system->CreatorsByExtension["irr"] =
  // ContentTypeEntry(MetaTypeOf(GeometryContent), MakeGeometryContent);

  // GAME FILE FORMATS
  // Quake I && (not game file) -> 3D GameStudio (3DGS)
  system->CreatorsByExtension["mdl"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Quake II
  system->CreatorsByExtension["md2"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Quake III Mesh
  system->CreatorsByExtension["md3"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Quake III Map/BSP, none of these files I found had models or textures
  // present in them
  // system->CreatorsByExtension["pk3"] =
  // ContentTypeEntry(MetaTypeOf(GeometryContent), MakeGeometryContent);
  // Doom 3
  system->CreatorsByExtension["md5mesh"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Valve Model
  system->CreatorsByExtension["smd"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Open Game Engine Exchange
  system->CreatorsByExtension["ogex"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);

  // OTHER FILE FORMATS
  // BlitzBasic 3D
  system->CreatorsByExtension["b3d"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Quick3D
  system->CreatorsByExtension["q3o"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  system->CreatorsByExtension["q3s"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Neutral File Format && Sense8 WorldToolKit
  system->CreatorsByExtension["nff"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Object File Format
  system->CreatorsByExtension["off"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // PovRAY Raw
  system->CreatorsByExtension["raw"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Terragen Terrain
  system->CreatorsByExtension["ter"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // 3D GameStudio (3DGS) Terrain
  system->CreatorsByExtension["hmp"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Nevercenter Silo Object
  system->CreatorsByExtension["sib"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);

  system->CreatorsByExtension["amf"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  system->CreatorsByExtension["x3d"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  system->CreatorsByExtension["mmd"] =
      ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);

  // system->CreatorsByExtension["wrl"] =
  // ContentTypeEntry(MetaTypeOf(GeometryContent), MakeGeometryContent,
  // UpdateGeometryContent);

  // UNTESTED FORMAT IMPORTS BELOW, no readily available models to test
  // UNTESTED GAME FORMATS
  // glTF <-DOESN'T WORK, NEEDS TO OPEN ADDITIONAL FILES
   system->CreatorsByExtension["gltf"] =
   ContentTypeEntry(LightningTypeId(GeometryContent), MakeGeometryContent, UpdateGeometryContent);
  // Return to Castle Wolfenstein
  // system->CreatorsByExtension["mdc"] =
  // ContentTypeEntry(MetaTypeOf(GeometryContent), MakeGeometryContent);
  // system->CreatorsByExtension["vta"] =
  // ContentTypeEntry(MetaTypeOf(GeometryContent), MakeGeometryContent);
  //   // Unreal, may attempt to open multiple files that reference each other,
  //   not currently supported system->CreatorsByExtension["3d"] =
  //   ContentTypeEntry(MetaTypeOf(GeometryContent), MakeGeometryContent);

  // UNTESTED OTHER FILE FORMATS
  // Izware Nendo
  // system->CreatorsByExtension["ndo"] =
  // ContentTypeEntry(MetaTypeOf(GeometryContent), MakeGeometryContent);
}

void AddGeometryFileFilters(ResourceManager* manager)
{
  Array<FileDialogFilter>& filters = manager->mOpenFileFilters;

  uint allMeshesIndex = filters.Size();
  filters.PushBack(FileDialogFilter("All Meshes", ""));
  filters.PushBack(FileDialogFilter("*.fbx"));
  filters.PushBack(FileDialogFilter("*.obj"));
  filters.PushBack(FileDialogFilter("*.dae"));
  filters.PushBack(FileDialogFilter("*.glb"));
  filters.PushBack(FileDialogFilter("*.3ds"));
  filters.PushBack(FileDialogFilter("*.blend"));
  filters.PushBack(FileDialogFilter("*.ase"));
  filters.PushBack(FileDialogFilter("*.ifc"));
  filters.PushBack(FileDialogFilter("*.xgl"));
  filters.PushBack(FileDialogFilter("*.zgl"));
  filters.PushBack(FileDialogFilter("*.ply"));
  filters.PushBack(FileDialogFilter("*.dxf"));
  filters.PushBack(FileDialogFilter("*.lwo"));
  filters.PushBack(FileDialogFilter("*.lws"));
  filters.PushBack(FileDialogFilter("*.lxo"));
  filters.PushBack(FileDialogFilter("*.stl"));
  filters.PushBack(FileDialogFilter("*.x"));
  filters.PushBack(FileDialogFilter("*.ac"));
  filters.PushBack(FileDialogFilter("*.ms3d"));
  filters.PushBack(FileDialogFilter("*.cob"));
  filters.PushBack(FileDialogFilter("*.scn"));
  filters.PushBack(FileDialogFilter("*.csm"));
  filters.PushBack(FileDialogFilter("*.xml"));
  filters.PushBack(FileDialogFilter("*.mdl"));
  filters.PushBack(FileDialogFilter("*.md2"));
  filters.PushBack(FileDialogFilter("*.md3"));
  filters.PushBack(FileDialogFilter("*.md5mesh"));
  filters.PushBack(FileDialogFilter("*.smd"));
  filters.PushBack(FileDialogFilter("*.ogex"));
  filters.PushBack(FileDialogFilter("*.b3d"));
  filters.PushBack(FileDialogFilter("*.q3o"));
  filters.PushBack(FileDialogFilter("*.q3s"));
  filters.PushBack(FileDialogFilter("*.nff"));
  filters.PushBack(FileDialogFilter("*.off"));
  filters.PushBack(FileDialogFilter("*.raw"));
  filters.PushBack(FileDialogFilter("*.ter"));
  filters.PushBack(FileDialogFilter("*.hmp"));
  filters.PushBack(FileDialogFilter("*.sib"));
  filters.PushBack(FileDialogFilter("*.amf"));
  filters.PushBack(FileDialogFilter("*.x3d"));
  filters.PushBack(FileDialogFilter("*.mmd"));
  filters.PushBack(FileDialogFilter("*.gltf"));

  // The first filter should contain the extensions of all other filters
  FileDialogFilter& allFilter = filters[allMeshesIndex];

  StringBuilder filterBuilder;

  // Skip the first filter (it's the one we're building)
  for (uint i = allMeshesIndex + 1; i < filters.Size(); ++i)
  {
    FileDialogFilter& currFilter = filters[i];
    filterBuilder.Append(currFilter.mFilter);

    if (i < filters.Size() - 1)
      filterBuilder.Append(";");
  }

  allFilter.mFilter = filterBuilder.ToString();
}

} // namespace Plasma
