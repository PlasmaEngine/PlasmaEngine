// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

IndexElementType::Enum DetermineIndexType(uint numIndices);
Mat4 AiMat4ToPlasmaMat4(aiMatrix4x4& aiMatrix4);

// Assimp stores UVs in Vec3s so this looks odd, but it is as intended and
// needed
void ConvertAndFillArrayVec2(aiVector3D* aiArray, uint numElements, Array<Vec2>& plasmaArray);
void ConvertAndFillArrayVec3(aiVector3D* aiArray, uint numElements, Array<Vec3>& plasmaArray);
void ConvertAndFillArrayVec4(aiColor4D* aiArray, uint numElements, Array<Vec4>& plasmaArray);
String CleanAssetName(String nodeName);
bool IsPivot(String nodeName);

// Helper functions for converting Assimp animation keys to plasma track types
PositionKey AssimpToPlasmaPositionKey(aiVectorKey positionKey);
RotationKey AssimpToPlasmaRotationKey(aiQuatKey rotationKey);
ScalingKey AssimpToPlasmaScalingKey(aiVectorKey scalingKey);

template <typename T>
void WriteIndexData(Array<uint>& indexBuffer, ChunkFileWriter& writer)
{
  for (size_t i = 0; i < indexBuffer.Size(); ++i)
    writer.Write((T)indexBuffer[i]);
}

} // namespace Plasma
