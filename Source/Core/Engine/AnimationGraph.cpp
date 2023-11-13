// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

AnimationGraph::DebugPreviewFunction AnimationGraph::mOnPreviewPressed = NULL;
AnimationGraph::DebugPreviewFunction AnimationGraph::mOnGraphCreated = NULL;

LightningDefineType(AnimationGraph, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindDocumented();
  PlasmaBindSetup(SetupMode::CallSetDefaults);

  LightningBindGetterSetter(ActiveNode);
  LightningBindMethod(IsPlayingInGraph);
  LightningBindMethod(Update);

  LightningBindGetterSetterProperty(Active);
  LightningBindFieldProperty(mTimeScale);

  PlasmaBindEvent(Events::AnimationBlendEnded, AnimationGraphEvent);
  PlasmaBindEvent(Events::AnimationEnded, AnimationGraphEvent);
  PlasmaBindEvent(Events::AnimationLooped, AnimationGraphEvent);
  PlasmaBindEvent(Events::AnimationPostUpdate, Event);

  // Node creation functions
  LightningBindMethod(CreateBasicNode);
  LightningBindMethod(CreateDirectBlendNode);
  LightningBindMethod(CreateCrossBlendNode);
  LightningBindMethod(CreateSelectiveNode);
  LightningBindMethod(CreateChainNode);
  LightningBindMethod(CreateBlendSpace1D);
  LightningBindMethod(CreateBlendSpace2D);

  PlasmaBindTag(Tags::Core);

  LightningBindMethodProperty(PreviewGraph);
  LightningBindMethodProperty(PrintGraph);
}

AnimationGraph::AnimationGraph()
{
  mFrameId = 0;
}

AnimationGraph::~AnimationGraph()
{
  DeleteObjectsInContainer(mBlendTracks);
}

void AnimationGraph::Serialize(Serializer& stream)
{
  SerializeName(mActive);
  // Loaded for old projects
  SerializeEnumName(AnimationPlayMode, mPlayMode);
  SerializeName(mTimeScale);
  // Loaded for old projects
  SerializeResourceName(mAnimation, AnimationManager);
  SerializeNameDefault(mDebugPreviewId, (u64)0);
}

void AnimationGraph::OnAllObjectsCreated(CogInitializer& initializer)
{
  if (Animation* animation = mAnimation)
  {
    if (animation != AnimationManager::GetDefault())
    {
      // Only add the component if it doesn't already exist
      if (GetOwner()->has(SimpleAnimation) == NULL)
      {
        SimpleAnimation* autoPlay = new SimpleAnimation();
        GetOwner()->AddComponent(autoPlay);
        autoPlay->SetPlayMode(mPlayMode);
        autoPlay->SetAnimation(animation);
        mAnimation = NULL;

        // The space has changed
        GetSpace()->MarkModified();
      }
    }
  }
}

void AnimationGraph::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(initializer.mSpace, Events::LogicUpdate, OnUpdate);

  if (mOnGraphCreated && !GetSpace()->IsEditorMode())
    mOnGraphCreated(this);

  ConnectThisTo(MetaDatabase::GetInstance(), Events::MetaModified, OnMetaModified);
}

void AnimationGraph::SetDefaults()
{
  mActive = true;
  mTimeScale = 1.0f;
  mPlayMode = AnimationPlayMode::PlayOnce;
  mAnimation = AnimationManager::GetDefault();
}

void AnimationGraph::Update(float dt)
{
  if (mActiveNode)
  {
    // Static so we can re-use memory and avoid extra allocations
    static Array<AnimationGraphEvent*> eventsToSend;
    eventsToSend.Clear();

    // Update the root node
    mActiveNode = mActiveNode->Update(this, dt, mFrameId++, eventsToSend);

    // Apply the frame if we're given anything back
    if (mActiveNode)
      ApplyFrame(mActiveNode->mFrameData);

    // Dispatch all events from the animation graph
    forRange (AnimationGraphEvent* eventToSend, eventsToSend.All())
    {
      GetOwner()->DispatchEvent(eventToSend->EventId, eventToSend);
      delete eventToSend;
    }

    // Send the post animation event
    Event eventToSend;
    GetOwner()->DispatchEvent(Events::AnimationPostUpdate, &eventToSend);

    return;
  }
}

void AnimationGraph::OnUpdate(UpdateEvent* e)
{
  // Do nothing if we aren't active
  if (!mActive)
    return;

  Update(e->Dt);
}

void AnimationGraph::ApplyFrame(AnimationFrame& frame)
{
  forRange (BlendTrack* blendTrack, mBlendTracks.Values())
  {
    ErrorIf(blendTrack->Index >= frame.Tracks.Size(), "Frame error");
    if (blendTrack->Index < frame.Tracks.Size())
    {
      AnimationFrameData& frameData = frame.Tracks[blendTrack->Index];
      if (frameData.Active)
      {
        Any& newValue = frameData.Value;
        if (!blendTrack->Object.IsNull() && newValue.IsHoldingValue())
          blendTrack->Property->SetValue(blendTrack->Object, newValue);
      }
    }
    else
    {
    }
  }
}

void AnimationGraph::OnMetaModified(MetaLibraryEvent* e)
{
  // The blend tracks store pointers to MetaProperties, and must be deleted
  DeleteObjectsInContainer(mBlendTracks);

  // Re-link all active animations
  if (AnimationNode* root = mActiveNode)
    root->ReLinkAnimations();
}

void AnimationGraph::SetActive(bool value)
{
  mActive = value;
}

void AnimationGraph::ForceUpdate()
{
  if (mActiveNode)
  {
    AnimationFrame frame;

    // Static so we can re-use memory and avoid extra allocations
    static Array<AnimationGraphEvent*> eventsToSend;
    eventsToSend.Clear();

    mActiveNode = mActiveNode->Update(this, 0.0f, mFrameId++, eventsToSend);
    if (mActiveNode)
      ApplyFrame(mActiveNode->mFrameData);
  }
}

void AnimationGraph::SetTimeScale(float scale)
{
  ReturnIf(scale < 0.0f, , "TimeScale must be positive.");

  mTimeScale = scale;
}

float AnimationGraph::GetTimeScale()
{
  return mTimeScale;
}

void AnimationGraph::SetActiveNode(AnimationNode* node)
{
  mActiveNode = node;
}

AnimationNode* AnimationGraph::GetActiveNode()
{
  return mActiveNode;
}

// ExamplePath:  /Stomach/Chest/LArm
// RootPath:     /
Cog* ResolveObjectPath(Cog* object, Array<String>& cogNames)
{
  Cog* foundObject = object;
  for (size_t i = 0; i < cogNames.Size(); ++i)
  {
    const String& name = cogNames[i];

    foundObject = object->FindChildByName(name);

    // If we failed to find the object then this could be an error
    // We'll at least attempt to find the next child by name incase name changes
    // occurred
    if (foundObject == nullptr)
      continue;

    object = foundObject;
  }

  return foundObject;
}

void AnimationGraph::SetUpPlayData(Animation* animation, PlayData& playData)
{
  playData.Clear();
  playData.Resize(animation->mNumberOfTracks);

  // Find all objects this track references
  forRange (ObjectTrack& track, animation->ObjectTracks.All())
  {
    ObjectTrackPlayData& objectData = playData[track.ObjectTrackId];
    Cog* currentObject = objectData.ObjectHandle;
    Cog* object = ResolveObjectPath(mOwner, track.CogNames);
    if (object)
    {
      objectData.ObjectHandle = object;

      // Clear All Per instance data
      // Find all objects for each sub track of each track
      objectData.mSubTrackPlayData.Clear();

      forRange (PropertyTrack& subTrack, track.PropertyTracks.All())
      {
        PropertyTrackPlayData& subTrackData = objectData.mSubTrackPlayData.PushBack();
        subTrackData.mComponent = NULL;
        subTrackData.mKeyframeIndex = 0;
        subTrack.LinkInstance(objectData.mSubTrackPlayData.Back(), mBlendTracks, track.GetFullPath(), object);
      }
    }
    else
    {
      DebugPrint("Failed to find object in animation track. %s\n", track.GetFullPath().c_str());
    }
  }
}

void AnimationGraph::PreviewGraph()
{
  if (mOnPreviewPressed)
    mOnPreviewPressed(this);
}

void AnimationGraph::SetPreviewMode()
{
  ConnectThisTo(GetSpace(), Events::PreviewUpdate, OnUpdate);
}

bool AnimationGraph::IsPlayingInGraph(Animation* animation)
{
  if (mActiveNode)
    return mActiveNode->IsPlayingInNode(animation->Name);
  return false;
}

void AnimationGraph::PrintGraph()
{
  if (mActiveNode)
    mActiveNode->PrintNode(0);
}

BasicAnimation* AnimationGraph::CreateBasicNode(Animation* animation, AnimationPlayMode::Enum mode)
{
  if (animation == NULL)
    DoNotifyException("Null Animation",
                      "Trying to create an animation node but the animation "
                      "resource is null.");
  return new BasicAnimation(this, animation, 0.0f, mode);
}

DirectBlend* AnimationGraph::CreateDirectBlendNode()
{
  return new DirectBlend();
}

CrossBlend* AnimationGraph::CreateCrossBlendNode()
{
  return new CrossBlend();
}

SelectiveNode* AnimationGraph::CreateSelectiveNode()
{
  return new SelectiveNode();
}

ChainNode* AnimationGraph::CreateChainNode()
{
  return new ChainNode();
}

BlendSpace1D* AnimationGraph::CreateBlendSpace1D()
{
    return new BlendSpace1D();
}

BlendSpace2D* AnimationGraph::CreateBlendSpace2D()
{
    return new BlendSpace2D();
}

LightningDefineType(SimpleAnimation, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindDocumented();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
  PlasmaBindDependency(AnimationGraph);
  LightningBindFieldProperty(mPlayMode);
  LightningBindGetterSetterProperty(Animation)->Add(new MetaEditorResource());

  LightningBindMethod(PlaySingle);
  LightningBindMethod(DirectBlend);
  LightningBindMethod(CrossBlend);
  LightningBindMethod(PlayIsolatedAnimation);
  LightningBindMethod(ChainAnimation);
}

void SimpleAnimation::Serialize(Serializer& stream)
{
  SerializeResourceName(mAnimation, AnimationManager);
  SerializeEnumNameDefault(AnimationPlayMode, mPlayMode, AnimationPlayMode::Loop);
}

void SimpleAnimation::Initialize(CogInitializer& initializer)
{
  mAnimGraph = GetOwner()->has(AnimationGraph);
  ReturnIf(mAnimGraph == NULL, , "Missing dependency.");

  // Play the animation
  if (mAnimation)
    PlaySingle(mAnimation, mPlayMode);
}

Animation* SimpleAnimation::GetAnimation()
{
  return mAnimation;
}

void SimpleAnimation::SetAnimation(Animation* animation)
{
  PlaySingle(animation, mPlayMode);
  mAnimation = animation;
}

AnimationPlayMode::Enum SimpleAnimation::GetPlayMode()
{
  return mPlayMode;
}

void SimpleAnimation::SetPlayMode(AnimationPlayMode::Enum mode)
{
  mPlayMode = mode;
}

AnimationNode* SimpleAnimation::PlaySingle(Animation* animation, AnimationPlayMode::Enum playMode)
{
  ReturnIf(animation == NULL, NULL, "Invalid animation given.");
  AnimationNode* node = BuildBasic(mAnimGraph, animation, 0, playMode);
  mAnimGraph->SetActiveNode(node);
  return node;
}

AnimationNode* SimpleAnimation::DirectBlend(Animation* animation,
                                            float transitionTime,
                                            AnimationPlayMode::Enum playMode)
{
  AnimationNode* activeNode = mAnimGraph->GetActiveNode();

  if (activeNode && animation)
  {
    AnimationNode* end = BuildBasic(mAnimGraph, animation, 0.0f, playMode);
    activeNode = BuildDirectBlend(mAnimGraph, activeNode, end, transitionTime);
    mAnimGraph->SetActiveNode(activeNode);
    return activeNode;
  }

  return NULL;
}

AnimationNode* SimpleAnimation::CrossBlend(Animation* animation, float transitionTime, AnimationPlayMode::Enum playMode)
{
  AnimationNode* activeNode = mAnimGraph->GetActiveNode();

  if (activeNode && animation)
  {
    AnimationNode* end = BuildBasic(mAnimGraph, animation, 0.0f, playMode);
    activeNode = BuildCrossBlend(mAnimGraph, activeNode, end, transitionTime);
    mAnimGraph->SetActiveNode(activeNode);
    return activeNode;
  }

  return NULL;
}

AnimationNode* SimpleAnimation::PlayIsolatedAnimation(Animation* animation,
                                                      Cog* rootBone,
                                                      AnimationPlayMode::Enum playMode)
{
  AnimationNode* activeNode = mAnimGraph->GetActiveNode();

  if (activeNode && animation)
  {
    AnimationNode* isolated = BuildBasic(mAnimGraph, animation, 0.0f, playMode);
    AnimationNode* blendingTo = BuildDirectBlend(mAnimGraph, activeNode->Clone(), isolated, 0.15f);
    activeNode = BuildSelectiveNode(mAnimGraph, activeNode, blendingTo, rootBone);
    mAnimGraph->SetActiveNode(activeNode);
    return activeNode;
  }

  return NULL;
}

AnimationNode* SimpleAnimation::ChainAnimation(Animation* animation, AnimationPlayMode::Enum playMode)
{
  AnimationNode* activeNode = mAnimGraph->GetActiveNode();

  if (activeNode && animation)
  {
    AnimationNode* b = BuildBasic(mAnimGraph, animation, 0.0f, playMode);
    activeNode = BuildChainNode(mAnimGraph, activeNode, b);
    mAnimGraph->SetActiveNode(activeNode);
    return activeNode;
  }

  return NULL;
}

} // namespace Plasma
