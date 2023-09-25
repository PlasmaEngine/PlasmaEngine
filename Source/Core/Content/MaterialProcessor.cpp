// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Graphics/GraphicsRuntime/GraphicsStandard.hpp"

namespace Plasma
{
    enum class PropertySemantic : int8
    {
        Unknown = 0,

        DiffuseColor,
        MetallicValue,
        RoughnessValue,
        EmissiveColor,
        SpecularColor,
        TwosidedValue,
    };

    enum class TextureSemantic : int8
    {
        Unknown = 0,

        DiffuseMap,
        DiffuseAlphaMap,
        OcclusionMap,
        RoughnessMap,
        MetallicMap,
        OrmMap,
        DisplacementMap,
        NormalMap,
        EmissiveMap,
        SpecularMap
    };

    //template <>
    //struct HashPolicy<Vec4>
    //{
    //  inline size_t operator()(Vec4Param value) const
    //  {
    //    return HashUint(*(unsigned int*)&value.x) + HashUint(*(unsigned int*)&value.y) +
    //           HashUint(*(unsigned int*)&value.z) + HashUint(*(unsigned int*)&value.w);
    //  }
    //  inline bool Equal(Vec4Param left, Vec4Param right) const
    //  {
    //    return left == right;
    //  }
    //};

    template <>
    struct HashPolicy<PropertySemantic>
    {
        inline size_t operator()(const PropertySemantic& value) const
        {
            return HashUint((unsigned int)value);
        }
        inline bool Equal(const PropertySemantic& left, const PropertySemantic& right) const
        {
            return left == right;
        }
    };

    template <>
    struct HashPolicy<TextureSemantic>
    {
        inline size_t operator()(const TextureSemantic& value) const
        {
            return HashUint((unsigned int)value);
        }
        inline bool Equal(const TextureSemantic& left, const TextureSemantic& right) const
        {
            return left == right;
        }
    };

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
    static void TryReadAssimpProperty(HashMap<PropertySemantic, Variant>& properties,
        PropertySemantic targetSemantic,
        const aiMaterial& assimpMaterial,
        const char* aiKey,
        uint32 aiType,
        uint32 aiIndex,
        bool invert = false)
    {
        assimpType value;

        if (assimpMaterial.Get(aiKey, aiType, aiIndex, value) == AI_SUCCESS)
        {
            properties[targetSemantic] = ConvertAssimpType(value, invert);
        }
    }

    static void TryReadAssimpTexture(HashMap<TextureSemantic, String>& textures,
        aiTextureType aiType,
        TextureSemantic targetSemantic,
        const aiMaterial& assimpMaterial,
        uint32 aiIndex)
    {
        aiString path;
        if (assimpMaterial.GetTexture(aiType, aiIndex, &path) == AI_SUCCESS)
        {
            textures[targetSemantic] = path.C_Str();
        }
    }

    MaterialProcessor::MaterialProcessor(MaterialContent* materialContent, String outputPath, String inputFile) :
        mMaterialContent(materialContent),
        mOutputPath(outputPath),
        mFilename(FilePath::GetFileNameWithoutExtension(inputFile))
    {
    }

    static String MakeValidMaterialName(const char* source, uint materialIndex, HashSet<String>& knownMaterialNames)
    {
        StringBuilder tmp;

        if (knownMaterialNames.Contains(tmp.ToString()))
        {
            if (tmp.GetSize() != 0)
            {
            }
        }
    }

    void MaterialProcessor::ExtractAndImportMaterials(const aiScene* scene)
    {
        aiMaterial** materials = scene->mMaterials;
        size_t numMaterials = scene->mNumMaterials;

        HashSet<String> knownMaterialNames;
        knownMaterialNames.Insert("");

        for (size_t i = 0; i < numMaterials; ++i)
        {
            aiMaterial* material = materials[i];

            CreateMaterial(material, i, "Material.data");
        }
    }

    void MaterialProcessor::CreateMaterial(aiMaterial* material, uint materialIndex, StringParam extension)
    {
        Status status;
        String filename = BuildString(FilePath::GetFileNameWithoutExtension(mFilename),
            "_",
            String(material->GetName().C_Str()));


        HashMap<PropertySemantic, Variant> properties;
        HashMap<TextureSemantic, String> textures;

        TryReadAssimpProperty<aiColor3D>(properties, PropertySemantic::DiffuseColor, *material, AI_MATKEY_COLOR_DIFFUSE);
        TryReadAssimpProperty<float>(properties, PropertySemantic::MetallicValue, *material, AI_MATKEY_SHININESS_STRENGTH);
        TryReadAssimpProperty<float>(properties, PropertySemantic::RoughnessValue, *material, AI_MATKEY_SHININESS);
        TryReadAssimpProperty<aiColor3D>(properties, PropertySemantic::EmissiveColor, *material, AI_MATKEY_COLOR_EMISSIVE);
        TryReadAssimpProperty<aiColor3D>(properties, PropertySemantic::SpecularColor, *material, AI_MATKEY_COLOR_SPECULAR);
        TryReadAssimpProperty<int>(properties, PropertySemantic::TwosidedValue, *material, AI_MATKEY_TWOSIDED);

        TryReadAssimpTexture(textures, aiTextureType_DIFFUSE, TextureSemantic::DiffuseMap, *material, materialIndex);
        TryReadAssimpTexture(textures,
            aiTextureType_BASE_COLOR,
            TextureSemantic::DiffuseMap,
            *material,
            materialIndex); // override aiTextureType_DIFFUSE

        TryReadAssimpTexture(textures, aiTextureType_SHININESS, TextureSemantic::RoughnessMap, *material, materialIndex);
        TryReadAssimpTexture(textures,
            aiTextureType_DIFFUSE_ROUGHNESS,
            TextureSemantic::RoughnessMap,
            *material,
            materialIndex); // override aiTextureType_SHININESS

        TryReadAssimpTexture(textures, aiTextureType_SPECULAR, TextureSemantic::MetallicMap, *material, materialIndex);
        TryReadAssimpTexture(textures,
            aiTextureType_METALNESS,
            TextureSemantic::MetallicMap,
            *material,
            materialIndex); // override aiTextureType_SPECULAR

        TryReadAssimpTexture(textures, aiTextureType_AMBIENT, TextureSemantic::OcclusionMap, *material, materialIndex);
        TryReadAssimpTexture(textures,
            aiTextureType_AMBIENT_OCCLUSION,
            TextureSemantic::OcclusionMap,
            *material,
            materialIndex); // override aiTextureType_AMBIENT

        TryReadAssimpTexture(
            textures, aiTextureType_DISPLACEMENT, TextureSemantic::DisplacementMap, *material, materialIndex);
        TryReadAssimpTexture(textures, aiTextureType_NORMALS, TextureSemantic::NormalMap, *material, materialIndex);

        TryReadAssimpTexture(textures, aiTextureType_EMISSIVE, TextureSemantic::EmissiveMap, *material, materialIndex);
        TryReadAssimpTexture(textures,
            aiTextureType_EMISSION_COLOR,
            TextureSemantic::EmissiveMap,
            *material,
            materialIndex); // override aiTextureType_EMISSIVE

        TryReadAssimpTexture(textures, aiTextureType_OPACITY, TextureSemantic::DiffuseAlphaMap, *material, materialIndex);




        // The member pcData holds the entire file data in memory, where mWidth is the
        // full length in bytes of pcData.
        String filePath = FilePath::CombineWithExtension(mOutputPath, filename, BuildString(".", extension));

        Material* zeroMat = MaterialManager::GetInstance()->CreateNewResource(filename);

        MaterialBlock diffuseColor;
        zeroMat->Add(diffuseColor, 0);

        zeroMat->Save(filePath);

        // WriteToFile(filePath.c_str(), (byte*)material->pcData, texture->mWidth);
    }

} // namespace Plasma