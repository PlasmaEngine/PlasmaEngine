// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

DeclareEnum3(StoreResult, Added, Replaced, Failed);

/// Object cache is use to store objects at runtime.
class ObjectStore : public ExplicitSingleton<ObjectStore>
{
public:
  LightningDeclareType(ObjectStore, TypeCopyMode::ReferenceType);

  /// Set the object store name. This is so each project has a separate store file.
  void SetStoreName(StringParam storeName);

  /// Set the object store directory. This is so the local user can configure where persistent data is stored on their machine
  void MigrateStoreLocation(StringParam oldLocation);

  /// Is there an entry record for the object in the store?
  bool IsEntryStored(StringParam name);

  /// Get number of entries in the ObjectStore.
  uint GetEntryCount();

  /// Get the ObjectStore entry at the specified index.
  String GetEntryAt(uint index);

  /// Store an object.
  StoreResult::Enum Store(StringParam name, Cog* object);

  /// Restore an object to the space.
  Cog* Restore(StringParam name, Space* space);

  /// Restore an object if it is not stored use the archetype to create it.
  Cog* RestoreOrArchetype(StringParam name, Archetype* archetype, Space* space);

  /// Attempts to remove an object from the store.
  void Erase(StringParam name);

  /// Clear the store
  void ClearStore();

private:
	// Helper function for file names.
	String BuildStorePath();
	String BuildTrashPath();
	String GetFullFilePath(StringParam storePath, StringParam entryName);

	/// Create the store directory
	void SetupDirectory(StringParam storePath);

	/// Populate the internal array of file names in the ObjectStore, if the
	/// proper ObjectStore directory exists.
	void PopulateEntries(StringParam storePath);

	String mStoreName;

	Array<String> mEntries;
};

} // namespace Plasma
