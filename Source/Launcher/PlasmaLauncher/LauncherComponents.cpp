// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// Small helper to make sure all runes are strictly number values
bool IsNumber(StringRange range)
{
  for (; !range.Empty(); range.PopFront())
  {
    if (!IsNumber(range.Front()))
    {
      return false;
    }
  }
  return true;
}

LightningDefineType(PlasmaBuildContent, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
}

void PlasmaBuildContent::Serialize(Serializer& stream)
{
  mBuildId.Serialize(stream, true);
  SerializeNameDefault(mDisplayName, String());
  SerializeNameDefault(mPackageName, String());
  SerializeNameDefault(mTags, String());
  SerializeNameDefault(mDownloadUrl, String());
}

void PlasmaBuildContent::Initialize(CogInitializer& initializer)
{
  Parse();
}

void PlasmaBuildContent::Parse()
{
  mTagSet.Clear();

  // Populate the tag set for easy querying. This set should stay
  // up-to-date as long as AddTag is called (no way to remove tags now)
  StringSplitRange range = mTags.Split(",");
  for (; !range.Empty(); range.PopFront())
    mTagSet.Insert(range.Front());
}

BuildId PlasmaBuildContent::GetBuildId()
{
  return mBuildId;
}

void PlasmaBuildContent::SetBuildId(const BuildId& buildId)
{
  mBuildId = buildId;
}

void PlasmaBuildContent::AddTag(StringParam tag)
{
  if (ContainsTag(tag))
    return;

  mTagSet.Insert(tag);

  // Otherwise this didn't exist so add it
  if (mTags.Empty())
    mTags = tag;
  else
    mTags = BuildString(mTags, ",", tag);
}

bool PlasmaBuildContent::ContainsTag(StringParam tag)
{
  return mTagSet.Contains(tag);
}

String PlasmaBuildContent::GetTagString()
{
  return mTags;
}

LightningDefineType(PlasmaBuildReleaseNotes, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
}

void PlasmaBuildReleaseNotes::PlasmaBuildReleaseNotes::Serialize(Serializer& stream)
{
  SerializeNameDefault(mNotes, String());
}

LightningDefineType(PlasmaBuildDeprecated, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
}

void PlasmaBuildDeprecated::Serialize(Serializer& stream)
{
  SerializeNameDefault(mServerMessage, String());
  SerializeNameDefault(mUserMessage, String());
}

String PlasmaBuildDeprecated::GetDeprecatedString()
{
  if (mServerMessage.Empty())
    return mUserMessage;

  return BuildString(mServerMessage, ". ", mUserMessage);
}

LightningDefineType(PlasmaTemplate, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);
}

void PlasmaTemplate::Serialize(Serializer& stream)
{
  String emptyStr;
  // Locally useful information
  SerializeNameDefault(mSKU, emptyStr);
  SerializeNameDefault(mVersionId, emptyStr);
  SerializeNameDefault(mDisplayName, emptyStr);
  SerializeNameDefault(mAuthor, emptyStr);
  SerializeNameDefault(mDescription, emptyStr);
  SerializeNameDefault(mDate, emptyStr);
  SerializeNameDefault(mSortPriority, 10.0f);
  SerializeNameDefault(mTags, emptyStr);

  // Server side information (doesn't technically need to be saved locally, but
  // whatever)
  SerializeNameDefault(mDownloadUrl, emptyStr);
  SerializeNameDefault(mIconUrl, emptyStr);
}

void PlasmaTemplate::Initialize(CogInitializer& initializer)
{
  Parse();
}

void PlasmaTemplate::Parse()
{
  mTagSet.Clear();

  // Populate the tag set for easy querying. This set should stay
  // up-to-date as long as AddTag is called (no way to remove tags now)
  StringSplitRange range = mTags.Split(",");
  for (; !range.Empty(); range.PopFront())
    mTagSet.Insert(range.Front());

  // Parse mVersionId into ranges of build ids, this could fail in which case
  // this isn't a valid template (the version id string is wrong)
  ParseVersionId();
}

String PlasmaTemplate::GetIdString()
{
  return BuildString(mSKU, mVersionId);
}

String PlasmaTemplate::GetFullTemplateVersionName()
{
  // build a string that is SKU[Id1-Id2,Id3,Id4...etc...] for all build ids
  StringBuilder builder;
  builder.Append(mSKU);
  builder.Append("_");
  for (size_t i = 0; i < mBuildIds.Size(); ++i)
  {
    BuildIdRange& idRange = mBuildIds[i];
    // Always print out the min build id, but only print out the max if we have
    // one
    builder.Append(idRange.mMin.GetVersionString());
    if (idRange.mHasMax)
    {
      builder.Append("-");
      builder.Append(idRange.mMax.GetFullId());
    }
  }
  return builder.ToString();
}

bool PlasmaTemplate::ParseVersionId()
{
  // Clear the old list of build ids and parse the string
  mBuildIds.Clear();
  bool result = ParseVersionId(mVersionId, mBuildIds);

  if (!mSpecificVersion.IsEmpty())
  {
    BuildIdRange& idRange = mBuildIds.PushBack();
    idRange.mMin = mSpecificVersion;
    idRange.mMax = mSpecificVersion;
    idRange.mHasMax = true;
  }
  return result;
}

bool PlasmaTemplate::TestBuildId(const BuildId& buildId)
{
  // Test if any BuildId range matches to this build
  bool foundMatch = false;
  for (size_t i = 0; i < mBuildIds.Size(); ++i)
    foundMatch |= TestBuildId(buildId, mBuildIds[i]);
  return foundMatch;
}

bool PlasmaTemplate::IsMoreExactRangeThan(const BuildId& buildId, PlasmaTemplate* otherTemplate)
{
  // Find the best build range for 'this' template. If one doesn't exist then
  // this is not a more exact range.
  BuildIdRange* myBuildRange = FindBestBuildRange(buildId);
  if (myBuildRange == nullptr)
    return false;

  // Find the best build range for the 'other' template. If it doesn't exist
  // then 'this' is the better range.
  BuildIdRange* otherBuildRange = otherTemplate->FindBestBuildRange(buildId);
  if (otherBuildRange == nullptr)
    return true;

  // Otherwise, check which build has the sooner min.
  return !myBuildRange->mMin.IsOlderThan(otherBuildRange->mMin);
}

bool PlasmaTemplate::ParseVersionId(StringParam versionId, BuildIdList& buildIds)
{
  // Split on ',' and parse each remaining string as a range/individual id
  // value. If a valid fails to parse correctly then mark the entire parsing as
  // failing.
  bool validId = false;
  StringSplitRange splitRange = versionId.Split(",");
  for (; !splitRange.Empty(); splitRange.PopFront())
  {
    BuildIdRange idRange;
    bool isValid = ParseVersionId(splitRange.Front(), idRange);
    if (!isValid)
      return false;

    buildIds.PushBack(idRange);
  }
  return true;
}

bool PlasmaTemplate::ParseVersionId(StringParam versionId, BuildIdRange& buildIdRange)
{
  // Split on the range separator token '-'.
  StringSplitRange splitRange = versionId.Split("-");
  // To make life easy this information is stored in an array so we can check
  // counts
  Array<StringRange> range;
  for (; !splitRange.Empty(); splitRange.PopFront())
    range.PushBack(splitRange.Front());

  // We must have either 1 or 2 items after the split, otherwise this isn't
  // valid
  if (range.Empty() || range.Size() > 2)
    return false;

  // Parse the min value, if we failed then the range failed
  if (!ParseVersionId(range[0], buildIdRange.mMin))
    return false;

  // If we only have one value then mark that
  if (range.Size() == 1)
  {
    buildIdRange.mHasMax = false;
  }
  // Otherwise, try to parse the max
  else
  {
    if (!ParseVersionId(range[1], buildIdRange.mMax))
      return false;
    buildIdRange.mHasMax = true;
  }

  return true;
}

bool PlasmaTemplate::ParseVersionId(StringParam versionId, BuildId& buildId)
{
  // It's far easier to split the string than to use a regex due to the optional
  // terms
  StringSplitRange splitRange = versionId.Split(".");
  // Parse into an array to make checks easier
  Array<StringRange> tokens;
  for (; !splitRange.Empty(); splitRange.PopFront())
    tokens.PushBack(splitRange.Front());

  // We can have up to 5 tokens (Branch.Major.Minor.Patch.RevisionId)
  if (tokens.Size() > 5 || tokens.Empty())
    return false;

  int index = 0;
  // If the first token contains a non-numeric value then this is assumed to be
  // the branch
  if (!IsNumber(tokens[0]))
  {
    buildId.mBranch = tokens[0];
    ++index;
  }

  // Continue parsing 4 values from wherever we left off (i.e. index 1 if we
  // parsed a branch), up to the max number of tokens. This should technically
  // check if any value isn't a valid number but I'm lazy for now.
  int* ids[4] = {&buildId.mMajorVersion, &buildId.mMinorVersion, &buildId.mPatchVersion, &buildId.mRevisionId};
  for (size_t i = 0; i < 4; ++i, ++index)
  {
    if (index >= (int)tokens.Size())
      break;

    ToValue(tokens[index], *ids[i]);
  }

  return true;
}

PlasmaTemplate::BuildIdRange* PlasmaTemplate::FindBestBuildRange(const BuildId& buildId)
{
  for (size_t i = 0; i < mBuildIds.Size(); ++i)
  {
    if (TestBuildId(buildId, mBuildIds[i]))
      return &mBuildIds[i];
  }
  return nullptr;
}

bool PlasmaTemplate::TestBuildId(const BuildId& buildId, BuildIdRange& buildIdRange)
{
  BuildId& minId = buildIdRange.mMin;
  BuildId& maxId = buildIdRange.mMax;

  // Determine what kind of an update it is to go from the build's id to the min
  // version's id
  BuildUpdateState::Enum minResult = buildId.CheckForUpdate(minId);

  // If going from the build id to the min version is not explicitly an older or
  // same upgrade then this isn't valid. (This deals with going to new and
  // breaking changes).
  if (!(minResult == BuildUpdateState::Older || minResult == BuildUpdateState::Same))
    return false;

  // Otherwise, if we have a max range to compare to then check it
  if (buildIdRange.mHasMax)
  {
    BuildUpdateState::Enum maxResult = buildId.CheckForUpdate(maxId);

    // If going from the build to the max id isn't an explicitly same or newer
    // upgrade (no breaking) then we are outside the max value.
    if (!(maxResult == BuildUpdateState::Newer || maxResult == BuildUpdateState::Same))
      return false;
  }

  return true;
}

} // namespace Plasma
