// MIT Licensed (see LICENSE.md).

#pragma once

namespace Plasma
{

class DebugGraphical : public Graphical
{
public:
  LightningDeclareType(DebugGraphical, TypeCopyMode::ReferenceType);

  void Initialize(CogInitializer& initializer) override;

  Aabb GetLocalAabb() override;
  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
  void AddToSpace() override;

  // Don't process component shader inputs
  void ComponentAdded(BoundType* typeId, Component* component) override
  {
  }
  void ComponentRemoved(BoundType* typeId, Component* component) override
  {
  }

  PrimitiveType::Enum mPrimitiveType;

  Array<Debug::DebugDrawObjectAny> mDebugObjects;
};

class DebugGraphicalPrimitive : public DebugGraphical
{
public:
  LightningDeclareType(DebugGraphicalPrimitive, TypeCopyMode::ReferenceType);

  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;
};

class DebugGraphicalThickLine : public DebugGraphicalPrimitive
{
public:
  LightningDeclareType(DebugGraphicalThickLine, TypeCopyMode::ReferenceType);

  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
};

class DebugGraphicalText : public DebugGraphical
{
public:
  LightningDeclareType(DebugGraphicalText, TypeCopyMode::ReferenceType);

  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;
};

} // namespace Plasma
