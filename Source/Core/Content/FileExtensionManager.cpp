// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

String TypeExtensionEntry::GetDefaultExtensionNoDot() const
{
  // Assume that the default extension is the first one in the list.
  return mExtensions[0];
}

String TypeExtensionEntry::GetDefaultExtensionWithDot() const
{
  return BuildString(".", GetDefaultExtensionNoDot());
}

bool TypeExtensionEntry::IsValidExtensionWithDot(StringParam extension) const
{
  // Strip the dot from the given extension
  StringRange dotLocation = extension.FindFirstOf(Rune('.'));
  ErrorIf(dotLocation.Empty(), "Given extension must contain a '.'");
  StringRange extensionNoDot = extension.SubString(dotLocation.End(), extension.End());
  // Now check the "no dot" version as that's what we store.
  return IsValidExtensionNoDot(extensionNoDot);
}

bool TypeExtensionEntry::IsValidExtensionNoDot(StringParam extension) const
{
  for (size_t i = 0; i < mExtensions.Size(); ++i)
  {
    if (mExtensions[i] == extension)
      return true;
  }
  return false;
}

FileExtensionManager::FileExtensionManager()
{
  // Create default type entries. The first one in the list is assumed to be the
  // default extension
  TypeExtensionEntry& lightningEntry = mTypeExtensionEntries["LightningScript"];
  lightningEntry.mExtensions.PushBack("lightningscript");
  lightningEntry.mExtensions.PushBack("z");

  TypeExtensionEntry& fragEntry = mTypeExtensionEntries["LightningFragment"];
  fragEntry.mExtensions.PushBack("lightningfrag");
  fragEntry.mExtensions.PushBack("lightningFrag");
}

TypeExtensionEntry* FileExtensionManager::GetLightningScriptTypeEntry()
{
  FileExtensionManager* instance = FileExtensionManager::GetInstance();
  return instance->mTypeExtensionEntries.FindPointer("LightningScript");
}

TypeExtensionEntry* FileExtensionManager::GetLightningFragmentTypeEntry()
{
  FileExtensionManager* instance = FileExtensionManager::GetInstance();
  return instance->mTypeExtensionEntries.FindPointer("LightningFragment");
}

} // namespace Plasma
