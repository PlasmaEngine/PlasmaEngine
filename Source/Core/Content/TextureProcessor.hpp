// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class TextureProcessor
{
public:
  TextureProcessor(TextureContent* textureContent, String outputPath, String inputFile);

  void ExtractAndImportTextures(const aiScene* scene, HashMap<uint, String>& textureDataMap);
  void CreateTexture(aiTexture* texture, StringParam fileName, StringParam extension);

  TextureContent* mTextureContent;
  String mOutputPath;
  String mFilename;
};

} // namespace Plasma
