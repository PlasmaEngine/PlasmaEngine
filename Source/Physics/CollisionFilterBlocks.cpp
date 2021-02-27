// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(CollisionFilterBlock, builder, type)
{
  PlasmaBindDocumented();

  LightningBindGetter(BlockType);
  LightningBindGetterSetterProperty(SendEventsToA);
  LightningBindGetterSetterProperty(SendEventsToB);
  LightningBindGetterSetterProperty(SendEventsToSpace);
  LightningBindFieldProperty(mEventOverride);
}

CollisionFilterBlock::CollisionFilterBlock()
{
  mStates.Clear();
}

void CollisionFilterBlock::Serialize(Serializer& stream)
{
  uint defaultFlags = CollisionBlockStates::SendEventsToA | CollisionBlockStates::SendEventsToB;
  SerializeBits(stream, mStates, CollisionBlockStates::Names, 0, defaultFlags);
  SerializeNameDefault(mEventOverride, String());
}

bool CollisionFilterBlock::GetSendEventsToA()
{
  return mStates.IsSet(CollisionBlockStates::SendEventsToA);
}

void CollisionFilterBlock::SetSendEventsToA(bool state)
{
  mStates.SetState(CollisionBlockStates::SendEventsToA, state);
}

bool CollisionFilterBlock::GetSendEventsToB()
{
  return mStates.IsSet(CollisionBlockStates::SendEventsToB);
}

void CollisionFilterBlock::SetSendEventsToB(bool state)
{
  mStates.SetState(CollisionBlockStates::SendEventsToB, state);
}

bool CollisionFilterBlock::GetSendEventsToSpace()
{
  return mStates.IsSet(CollisionBlockStates::SendEventsToSpace);
}

void CollisionFilterBlock::SetSendEventsToSpace(bool state)
{
  mStates.SetState(CollisionBlockStates::SendEventsToSpace, state);
}

CollisionFilterBlockType::Enum CollisionFilterBlock::GetBlockType() const
{
  if (mBlockType & FilterFlags::StartEvent)
    return CollisionFilterBlockType::CollisionStartedBlock;
  if (mBlockType & FilterFlags::PersistedEvent)
    return CollisionFilterBlockType::CollisionPersistedBlock;
  if (mBlockType & FilterFlags::EndEvent)
    return CollisionFilterBlockType::CollisionEndedBlock;
  if (mBlockType & FilterFlags::PreSolveEvent)
    return CollisionFilterBlockType::PreSolveBlock;
  return CollisionFilterBlockType::CollisionStartedBlock;
}

LightningDefineType(CollisionStartBlock, builder, type)
{
  PlasmaBindComponent();
  type->HasOrAdd<::Plasma::CogComponentMeta>(type);
  type->Add(new MetaSerialization());
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindDocumented();
}

CollisionStartBlock::CollisionStartBlock()
{
  mBlockType = FilterFlags::StartEvent;
}

LightningDefineType(CollisionPersistedBlock, builder, type)
{
  PlasmaBindComponent();
  type->HasOrAdd<::Plasma::CogComponentMeta>(type);
  type->Add(new MetaSerialization());
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindDocumented();
}

CollisionPersistedBlock::CollisionPersistedBlock()
{
  mBlockType = FilterFlags::PersistedEvent;
}

LightningDefineType(CollisionEndBlock, builder, type)
{
  PlasmaBindComponent();
  type->HasOrAdd<::Plasma::CogComponentMeta>(type);
  type->Add(new MetaSerialization());
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindDocumented();
}

CollisionEndBlock::CollisionEndBlock()
{
  mBlockType = FilterFlags::EndEvent;
}

LightningDefineType(PreSolveBlock, builder, type)
{
  PlasmaBindComponent();
  type->HasOrAdd<::Plasma::CogComponentMeta>(type);
  type->Add(new MetaSerialization());
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindDocumented();
}

PreSolveBlock::PreSolveBlock()
{
  mBlockType = FilterFlags::PreSolveEvent;
}

LightningDefineTemplateType(CollisionFilterMetaComposition, builder, type)
{
}

} // namespace Plasma
