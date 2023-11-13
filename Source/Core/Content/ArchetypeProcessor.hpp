// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class ArchetypeProcessor
{
public:
  ArchetypeProcessor(GeneratedArchetype* generatedArchetype,
                     HierarchyDataMap& hierarchyData,
                     MaterialDataMap& materialDataMap);

  void BuildSceneGraph(String rootNode);
  SceneGraphNode* BuildSceneNodes(HierarchyData nodeData);
  void ExportSceneGraph(String filename, String outputPath);

  SceneGraphSource mSceneSource;
  GeneratedArchetype* mGeneratedArchetype;
  HierarchyDataMap& mHierarchyDataMap;
  MaterialDataMap& mMaterialDataMap;
};

} // namespace Plasma
