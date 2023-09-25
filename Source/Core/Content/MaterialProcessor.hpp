// MIT Licenced (see LICENCE.md)
#pragma once

namespace Plasma
{
    class MaterialProcessor
    {
    public:
        MaterialProcessor(MaterialContent* materialContent, String outputPath, String inputFile);

        void ExtractAndImportMaterials(const aiScene* scene);
        void CreateMaterial(aiMaterial* material, uint materialIndex, StringParam extension);

        MaterialContent* mMaterialContent;
        String mOutputPath;
        String mFilename;
    };
} // namespace Plasma