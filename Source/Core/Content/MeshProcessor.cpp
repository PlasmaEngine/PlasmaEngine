// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

Vec4 ConvertAssimpType(const aiColor4D& value, bool invert /*= false*/)
{
  if (invert)
    return Vec4(1.0f - value.r, 1.0f - value.g, 1.0f - value.b, 1.0f - value.a);
  else
    return Vec4(value.r, value.g, value.b, value.a);
}

Vec4 ConvertAssimpType(const aiColor3D& value, bool invert /*= false*/)
{
  if (invert)
    return Vec4(1.0f - value.r, 1.0f - value.g, 1.0f - value.b, 0.0f);
  else
    return Vec4(value.r, value.g, value.b, 0.0f);
}

Mat4 ConvertAssimpType(const aiMatrix4x4& aiMatrix4, bool dummy /*= false*/)
{
  Assert(!dummy, "not implemented");

#if ColumnBasis
  return Mat4(aiMatrix4.a1,
              aiMatrix4.a2,
              aiMatrix4.a3,
              aiMatrix4.a4,
              aiMatrix4.b1,
              aiMatrix4.b2,
              aiMatrix4.b3,
              aiMatrix4.b4,
              aiMatrix4.c1,
              aiMatrix4.c2,
              aiMatrix4.c3,
              aiMatrix4.c4,
              aiMatrix4.d1,
              aiMatrix4.d2,
              aiMatrix4.d3,
              aiMatrix4.d4);
#else
  return Mat4(aiMatrix4.a1,
              aiMatrix4.b1,
              aiMatrix4.c1,
              aiMatrix4.d1,
              aiMatrix4.a2,
              aiMatrix4.b2,
              aiMatrix4.c2,
              aiMatrix4.d2,
              aiMatrix4.a3,
              aiMatrix4.b3,
              aiMatrix4.c3,
              aiMatrix4.d3,
              aiMatrix4.a4,
              aiMatrix4.b4,
              aiMatrix4.c4,
              aiMatrix4.d4);
#endif
}

Vec3 ConvertAssimpType(const aiVector3D& value, bool dummy /*= false*/)
{
  Assert(!dummy, "not implemented");

  return Vec3(value.x, value.y, value.z);
}

Quat ConvertAssimpType(const aiQuaternion& value, bool dummy /*= false*/)
{
  Assert(!dummy, "not implemented");

  return Quat(value.x, value.y, value.z, value.w);
}

float ConvertAssimpType(float value, bool dummy /*= false*/)
{
  Assert(!dummy, "not implemented");

  return value;
}

int ConvertAssimpType(int value, bool dummy /*= false*/)
{
  Assert(!dummy, "not implemented");

  return value;
}

template <typename assimpType>
static void TryReadAssimpProperty(HashMap<String, Any>& properties,
                                  MaterialAttribute::Enum targetSemantic,
                                  const aiMaterial& assimpMaterial,
                                  const char* aiKey,
                                  uint32 aiType,
                                  uint32 aiIndex,
                                  bool invert = false)
{
  assimpType value;

  if (assimpMaterial.Get(aiKey, aiType, aiIndex, value) == AI_SUCCESS)
  {
    properties[MaterialAttribute::Names[targetSemantic]] = ConvertAssimpType(value, invert);
  }
}

static void TryReadAssimpTexture(HashMap<String, Any>& textures,
                                 aiTextureType aiType,
                                 MaterialAttribute::Enum targetSemantic,
                                 const aiMaterial& assimpMaterial,
                                 uint32 aiIndex,
                                 HashMap<uint, String>& textureDataMap)
{
  aiString path;
  if (assimpMaterial.GetTexture(aiType, aiIndex, &path) == AI_SUCCESS)
  {
    String texturePath = String(path.C_Str());
    if (texturePath.StartsWith("*"))
    {
      StringRange index = texturePath.SubString(texturePath.Begin() + 1, texturePath.End());
      uint indexValue;
      ToValue(index, indexValue);
      if (textureDataMap.ContainsKey(indexValue))
      {
        textureDataMap.TryGetValue(indexValue, texturePath);
      }
    }
    textures[MaterialAttribute::Names[targetSemantic]] = texturePath;
  }
}

MeshProcessor::MeshProcessor(MeshBuilder* meshBuilder,
                             MeshDataMap& meshDataMap,
                             MaterialDataMap& materialDataMap,
                             HashMap<uint, String>& textureDataMap) :
    mBuilder(meshBuilder),
    mMeshDataMap(meshDataMap),
    mMaterialDataMap(materialDataMap),
    mTextureDataMap(textureDataMap)
{
}

MeshProcessor::~MeshProcessor()
{
}

void MeshProcessor::ExtractMaterialData(const aiScene* scene)
{
  GeometryImport* geoImport = mBuilder->mOwner->has(GeometryImport);

  aiMaterial** materials = scene->mMaterials;
  size_t numMaterials = scene->mNumMaterials;

  // process and collect each material information
  for (size_t materialIndex = 0; materialIndex < numMaterials; ++materialIndex)
  {
    aiMaterial* material = materials[materialIndex];
    if (material->GetName().C_Str() == "")
      continue;
    MaterialData& materialData = mMaterialDataMap[materialIndex];
    materialData.mMaterialName = String(material->GetName().C_Str());

    for (uint i = 0; i < material->mNumProperties; i++)
    {
      aiMaterialProperty* property = material->mProperties[i];
    }

    // for (int propertyIndex = 0; propertyIndex < material->mNumProperties; propertyIndex++)
    //{
    //     // TODO: optimize, do not try to read all properties
    //     // check if property read was succussful and continue
    //     // Note: special case when we want to override a property
    //     // e.g.: aiTextureType_BASE_COLOR overrides aiTextureType_DIFFUSE
    {
      TryReadAssimpProperty<aiColor4D>(
          materialData.mMaterialProperties, MaterialAttribute::DiffuseColor, *material, AI_MATKEY_COLOR_DIFFUSE);

      TryReadAssimpProperty<float>(materialData.mMaterialProperties,
                                   MaterialAttribute::MetallicValue,
                                   *material,
                                   /*AI_MATKEY_METALLIC_FACTOR*/ AI_MATKEY_SHININESS_STRENGTH);

      TryReadAssimpProperty<real>(materialData.mMaterialProperties,
                                   MaterialAttribute::RoughnessValue,
                                   *material,
                                   AI_MATKEY_ROUGHNESS_FACTOR /*AI_MATKEY_SHININESS*/);

      TryReadAssimpProperty<aiColor3D>(
          materialData.mMaterialProperties, MaterialAttribute::EmissiveColor, *material, AI_MATKEY_COLOR_EMISSIVE);

      TryReadAssimpProperty<float>(materialData.mMaterialProperties,
                                       MaterialAttribute::SpecularValue,
                                       *material,
                                       AI_MATKEY_SPECULAR_FACTOR /*AI_MATKEY_COLOR_SPECULAR*/);

      TryReadAssimpProperty<int>(
          materialData.mMaterialProperties, MaterialAttribute::TwosidedValue, *material, AI_MATKEY_TWOSIDED);

      TryReadAssimpTexture(materialData.mMaterialProperties,
                           aiTextureType_DIFFUSE,
                           MaterialAttribute::DiffuseMap,
                           *material,
                           materialIndex,
                           mTextureDataMap);

      TryReadAssimpTexture(materialData.mMaterialProperties,
                           aiTextureType_BASE_COLOR,
                           MaterialAttribute::DiffuseMap,
                           *material,
                           materialIndex,
                           mTextureDataMap); // override aiTextureType_DIFFUSE

      TryReadAssimpTexture(materialData.mMaterialProperties,
                           aiTextureType_SHININESS,
                           MaterialAttribute::RoughnessMap,
                           *material,
                           materialIndex,
                           mTextureDataMap);

      TryReadAssimpTexture(materialData.mMaterialProperties,
                           aiTextureType_DIFFUSE_ROUGHNESS,
                           MaterialAttribute::RoughnessMap,
                           *material,
                           materialIndex,
                           mTextureDataMap); // override aiTextureType_SHININESS

      TryReadAssimpTexture(materialData.mMaterialProperties,
                           aiTextureType_SPECULAR,
                           MaterialAttribute::MetallicMap,
                           *material,
                           materialIndex,
                           mTextureDataMap);

      TryReadAssimpTexture(materialData.mMaterialProperties,
                           aiTextureType_METALNESS,
                           MaterialAttribute::MetallicMap,
                           *material,
                           materialIndex,
                           mTextureDataMap); // override aiTextureType_SPECULAR

      TryReadAssimpTexture(materialData.mMaterialProperties,
                           aiTextureType_AMBIENT,
                           MaterialAttribute::OcclusionMap,
                           *material,
                           materialIndex,
                           mTextureDataMap);

      TryReadAssimpTexture(materialData.mMaterialProperties,
                           aiTextureType_AMBIENT_OCCLUSION,
                           MaterialAttribute::OcclusionMap,
                           *material,
                           materialIndex,
                           mTextureDataMap); // override aiTextureType_AMBIENT

      TryReadAssimpTexture(materialData.mMaterialProperties,
                           aiTextureType_DISPLACEMENT,
                           MaterialAttribute::DisplacementMap,
                           *material,
                           materialIndex,
                           mTextureDataMap);

      TryReadAssimpTexture(materialData.mMaterialProperties,
                           aiTextureType_NORMALS,
                           MaterialAttribute::NormalMap,
                           *material,
                           materialIndex,
                           mTextureDataMap);

      TryReadAssimpTexture(materialData.mMaterialProperties,
                           aiTextureType_EMISSIVE,
                           MaterialAttribute::EmissiveMap,
                           *material,
                           materialIndex,
                           mTextureDataMap);

      TryReadAssimpTexture(materialData.mMaterialProperties,
                           aiTextureType_EMISSION_COLOR,
                           MaterialAttribute::EmissiveMap,
                           *material,
                           materialIndex,
                           mTextureDataMap); // override aiTextureType_EMISSIVE

      TryReadAssimpTexture(materialData.mMaterialProperties,
                           aiTextureType_OPACITY,
                           MaterialAttribute::DiffuseAlphaMap,
                           *material,
                           materialIndex,
                           mTextureDataMap);
    }
  }
}

void MeshProcessor::ExtractAndProcessMeshData(const aiScene* scene)
{
  GeometryImport* geoImport = mBuilder->mOwner->has(GeometryImport);
  Mat4 transform = geoImport->mTransform;
  Mat3 normalTransform = geoImport->mNormalTransform;
  Mat3 changeOfBasis = geoImport->mChangeOfBasis;

  // if the winding order is being flipped our normal transform needs to account
  // for this
  if (mBuilder->mFlipWindingOrder)
    normalTransform *= -1;
  // this is an in editor only exposed option to fixed messed up normals
  if (mBuilder->mFlipNormals)
    normalTransform *= -1;

  aiMesh** meshes = scene->mMeshes;
  // size_t numMeshes = scene->mNumMeshes;
  size_t numMeshes = mMeshDataMap.Size();

  // process and collect each meshes information
  for (size_t meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
  {
    aiMesh* mesh = meshes[meshIndex];
    MeshData& meshData = mMeshDataMap[meshIndex];

    VertexDescriptionBuilder vertexDescriptionBuilder;
    meshData.mVertexDescription = vertexDescriptionBuilder.SetupDescriptionFromMesh(mesh);

    // write out the vertex count
    uint numVertices = mesh->mNumVertices;

    // extract all the vertex data
    VertexArray& vertexBuffer = meshData.mVertexBuffer;
    vertexBuffer.Resize(numVertices);
    // guarantees bone weights and indices to be 0 if unset on a vertex
    memset(vertexBuffer.Data(), 0, vertexBuffer.Size() * sizeof(VertexData));

    if (mesh->HasPositions())
    {
      meshData.mHasPosition = true;
      for (size_t i = 0; i < numVertices; ++i)
      {
        aiVector3D position = mesh->mVertices[i];
        vertexBuffer[i].mPosition = Vec3(position.x, position.y, position.z);
        vertexBuffer[i].mPosition = Math::TransformPoint(transform, vertexBuffer[i].mPosition);
        // we are generating the aabb as we go
        meshData.mAabb.Expand(vertexBuffer[i].mPosition);
      }
    }

    if (mesh->HasNormals())
    {
      meshData.mHasNormal = true;
      for (size_t i = 0; i < numVertices; ++i)
      {
        aiVector3D normal = mesh->mNormals[i];
        vertexBuffer[i].mNormal = Vec3(normal.x, normal.y, normal.z);
        vertexBuffer[i].mNormal = Math::Transform(normalTransform, vertexBuffer[i].mNormal);
      }
    }

    if (mesh->HasTangentsAndBitangents())
    {
      meshData.mHasTangentBitangent = true;
      for (size_t i = 0; i < numVertices; ++i)
      {
        aiVector3D tangent = mesh->mTangents[i];
        aiVector3D biTangent = mesh->mBitangents[i];
        vertexBuffer[i].mTangent = Vec3(tangent.x, tangent.y, tangent.z);
        vertexBuffer[i].mBitangent = Vec3(biTangent.x, biTangent.y, biTangent.z);
        vertexBuffer[i].mTangent = Math::Transform(normalTransform, vertexBuffer[i].mTangent);
        vertexBuffer[i].mBitangent = Math::Transform(normalTransform, vertexBuffer[i].mBitangent);
      }
    }

    if (mesh->HasTextureCoords(0))
    {
      meshData.mHasUV0 = true;
      for (size_t i = 0; i < numVertices; ++i)
      {
        aiVector3D uv = mesh->mTextureCoords[0][i];
        vertexBuffer[i].mUV0 = Vec2(uv.x, uv.y);
      }
    }

    if (mesh->HasTextureCoords(1))
    {
      meshData.mHasUV1 = true;
      for (size_t i = 0; i < numVertices; ++i)
      {
        aiVector3D uv = mesh->mTextureCoords[1][i];
        vertexBuffer[i].mUV1 = Vec2(uv.x, uv.y);
      }
    }

    if (mesh->HasVertexColors(0))
    {
      meshData.mHasColor0 = true;
      for (size_t i = 0; i < numVertices; ++i)
      {
        aiColor4D color = mesh->mColors[0][i];
        vertexBuffer[i].mColor0 = Vec4(color.r, color.g, color.b, color.a);
      }
    }

    if (mesh->HasVertexColors(1))
    {
      meshData.mHasColor1 = true;
      for (size_t i = 0; i < numVertices; ++i)
      {
        aiColor4D color = mesh->mColors[1][i];
        vertexBuffer[i].mColor1 = Vec4(color.r, color.g, color.b, color.a);
      }
    }

    if (mesh->HasBones())
    {
      meshData.mHasBones = true;

      aiBone** bones = mesh->mBones;
      size_t numBones = mesh->mNumBones;

      // collect all the weights keyed by the vertex they affect
      BoneDataMap boneWeightData;
      for (size_t boneIndex = 0; boneIndex < numBones; ++boneIndex)
      {
        aiBone* bone = bones[boneIndex];

        Mat4 bind = AiMat4ToPlasmaMat4(bone->mOffsetMatrix);

        Vec3 scale;
        Mat3 rotate;
        Vec3 translate;
        bind.Decompose(&scale, &rotate, &translate);

        // Apply import transformations
        translate = Math::TransformPoint(transform, translate);
        rotate = changeOfBasis * rotate * changeOfBasis.Transposed();

        bind.BuildTransform(translate, rotate, scale);

        MeshBone meshBone;
        meshBone.mName = CleanAssetName(bone->mName.C_Str());
        meshBone.mBindTransform = bind;
        meshData.mBones.PushBack(meshBone);

        size_t numWeights = bone->mNumWeights;
        for (size_t weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
          aiVertexWeight vertWeight = bone->mWeights[weightIndex];
          BoneData boneData;
          boneData.mBoneWeight = vertWeight.mWeight;
          boneData.mBoneIndex = (::byte)boneIndex;
          boneWeightData[vertWeight.mVertexId].PushBack(boneData);
        }
      }

      // go over our data and make sure no vertex has more than 4 bones
      // affecting it
      for (size_t i = 0; i < numVertices; ++i)
      {
        Array<BoneData>* weightDataPointer = boneWeightData.FindPointer(i);
        if (weightDataPointer)
        {
          Array<BoneData>& weightData = *weightDataPointer;
          bool weightsRemoved = false;
          // remove the smallest influence weight until we are down to the max
          while (weightData.Size() > cMaxBonesWeights)
          {
            weightsRemoved = true;
            unsigned indexOfSmallest = 0;
            float smallestWeight = weightData[0].mBoneWeight;
            for (size_t j = 0; j < weightData.Size(); ++j)
            {
              float weight = weightData[j].mBoneWeight;
              if (weight < smallestWeight)
              {
                smallestWeight = weight;
                indexOfSmallest = j;
              }
            }
            weightData.EraseAt(indexOfSmallest);
          }
          // if we removed any weights we need to normalize their influence
          if (weightsRemoved)
          {
            // Normalize the weights
            float sum = 0.f;
            for (int w = 0; w < cMaxBonesWeights; ++w)
              sum += weightData[w].mBoneWeight;
            for (int w = 0; w < cMaxBonesWeights; ++w)
              weightData[w].mBoneWeight /= sum;
          }
        }
      }

      // now that we have collected all the weights we need to go to assign each
      // to their corresponding vertices
      for (size_t i = 0; i < numVertices; ++i)
      {
        Array<BoneData>* weightDataPointer = boneWeightData.FindPointer(i);
        if (weightDataPointer)
        {
          // the memzero'ed vertex buffer at the start makes sure that bone data
          // is at least weight 0 index 0 so we only assign the data we
          // collected from the bones
          Array<BoneData>& weightData = *weightDataPointer;
          for (size_t j = 0; j < weightData.Size(); ++j)
          {
            vertexBuffer[i].mBoneWeights[j] = weightData[j].mBoneWeight;
            vertexBuffer[i].mBoneIndices[j] = weightData[j].mBoneIndex;
          }
        }
      }

      Vec3 scale;
      Mat3 rotate;
      Vec3 translate;
      meshData.mMeshTransform.Decompose(&scale, &rotate, &translate);

      // Apply import transformations
      translate = Math::TransformPoint(transform, translate);
      rotate = changeOfBasis * rotate * changeOfBasis.Transposed();

      meshData.mMeshTransform.BuildTransform(translate, rotate, scale);
    }
    else
    {
      // Don't need a bind offset for the mesh if it's not skinned
      meshData.mMeshTransform = Mat4::cIdentity;
    }

    // collect index buffer data
    if (mesh->HasFaces())
    {
      aiFace* faces = mesh->mFaces;
      uint numFaces = mesh->mNumFaces;

      uint* indices = faces->mIndices;
      uint numIndices = faces->mNumIndices;
      // this only works for now assuming all faces are triangles as we force
      // triangulate models determine the index size needed based on index count

      uint totalIndicies = numFaces * numIndices;
      meshData.mIndexBuffer.Reserve(totalIndicies);
      for (size_t i = 0; i < numFaces; ++i)
      {
        for (size_t j = 0; j < faces[i].mNumIndices; ++j)
          meshData.mIndexBuffer.PushBack(faces[i].mIndices[j]);
      }
    }
  }
}

void MeshProcessor::ExportMeshData(String outputPath)
{
  WriteSingleMeshes(outputPath);
}

void MeshProcessor::WriteSingleMeshes(String outputPath)
{
  size_t numMeshes = mBuilder->Meshes.Size();
  for (size_t meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
  {
    GeometryResourceEntry& entry = mBuilder->Meshes[meshIndex];
    MeshData& meshData = mMeshDataMap[meshIndex];

    // setup the chunk writer
    ChunkFileWriter writer;

    String meshFile = FilePath::CombineWithExtension(outputPath, entry.mName, ".mesh");
    writer.Open(meshFile);

    // write out the the header first
    MeshHeader header;
    header.mFileId = MeshFileId;
    header.mAabb = meshData.mAabb;
    // for now we will only ever write out triangles
    header.mPrimitiveType = PrimitiveType::Triangles;
    header.mBindOffsetInv = meshData.mMeshTransform.Inverted();
    writer.Write(header);

    // write out vertex buffer chunk
    u32 vertexStart = writer.StartChunk(VertexChunk);
    writer.Write(meshData.mVertexDescription);

    uint numVertices = meshData.mVertexBuffer.Size();
    writer.Write(numVertices);

    // write all the vertex data
    for (size_t vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
    {
      VertexData& vertexData = meshData.mVertexBuffer[vertexIndex];
      if (meshData.mHasPosition)
        writer.Write(vertexData.mPosition);

      if (meshData.mHasNormal)
        writer.Write(vertexData.mNormal);

      if (meshData.mHasTangentBitangent)
      {
        writer.Write(vertexData.mTangent);
        writer.Write(vertexData.mBitangent);
      }

      if (meshData.mHasUV0)
        writer.Write(vertexData.mUV0);

      if (meshData.mHasUV1)
        writer.Write(vertexData.mUV1);

      if (meshData.mHasColor0)
        writer.Write(vertexData.mColor0);

      if (meshData.mHasColor1)
        writer.Write(vertexData.mColor1);

      if (meshData.mHasBones)
      {
        writer.Write(vertexData.mBoneWeights);
        writer.Write(vertexData.mBoneIndices);
      }
    }
    writer.EndChunk(vertexStart);

    if (!meshData.mIndexBuffer.Empty())
    {
      u32 indexStart = writer.StartChunk(IndexChunk);
      uint numIndices = meshData.mIndexBuffer.Size();

      IndexElementType::Enum indexType = DetermineIndexType(numIndices);
      ::byte indexTypeByte = (::byte)indexType;
      writer.Write(indexTypeByte);
      writer.Write(numIndices);

      switch (indexType)
      {
      case IndexElementType::Byte:
        WriteIndexData<::byte>(meshData.mIndexBuffer, writer);
        break;
      case IndexElementType::Ushort:
        WriteIndexData<ushort>(meshData.mIndexBuffer, writer);
        break;
      case IndexElementType::Uint:
        WriteIndexData<uint>(meshData.mIndexBuffer, writer);
        break;
      }
      writer.EndChunk(indexStart);
    }

    if (!meshData.mBones.Empty())
    {
      u32 indexStart = writer.StartChunk(SkeletonChunk);

      uint count = meshData.mBones.Size();
      writer.Write(count);

      forRange (MeshBone& bone, meshData.mBones.All())
      {
        writer.Write(bone.mName);
        writer.Write(bone.mBindTransform);
      }

      writer.EndChunk(indexStart);
    }
  }
}

void MeshProcessor::WriteCombinedMesh(String outputPath)
{
}

} // namespace Plasma
