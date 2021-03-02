// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

/// The BroadPhase interface for the DynamicAabbTree. Unlike the tree itself,
/// this keeps track of internal pairs and figures out what actually
/// needs to be queried for self intersections.
class AvlDynamicAabbTreeBroadPhase : public BaseDynamicAabbTreeBroadPhase<AvlDynamicAabbTree<void*>>
{
public:
  typedef AvlDynamicAabbTree<void*> TreeType;

  AvlDynamicAabbTreeBroadPhase();
  ~AvlDynamicAabbTreeBroadPhase();

  LightningDeclareType(AvlDynamicAabbTreeBroadPhase, TypeCopyMode::ReferenceType);
};

} // namespace Plasma
