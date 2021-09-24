// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(ObjectStore, builder, type)
{
	type->HandleManager = LightningManagerId(PointerManager);

	PlasmaBindDocumented();

	LightningBindGetter(EntryCount);

	LightningBindMethodAs(IsEntryStored, "IsStored")->AddAttribute(DeprecatedAttribute);
	LightningBindMethod(IsEntryStored);
	LightningBindMethod(GetEntryAt);
	LightningBindMethod(Store);
	LightningBindMethod(Restore);
	LightningBindMethod(RestoreOrArchetype);
	LightningBindMethod(Erase);
	LightningBindMethod(ClearStore);
}

void ObjectStore::SetStoreName(StringParam storeName)
{
	// switch over to a new path and populate based on the files there
	mStoreName = storeName;
	String storePath = BuildStorePath();
	SetupDirectory(storePath);
}

void ObjectStore::MigrateStoreLocation(StringParam oldLocation)
{
	String newLocation = BuildStorePath();

	// do nothing if it is the same location
	if (newLocation == oldLocation)
	{
		return;
	}

	//create the directory to contain stored objects.
	if (DirectoryExists(newLocation) == false)
	{
		CreateDirectoryAndParents(newLocation);
	}

	// move the files
	if (DirectoryExists(oldLocation))
	{
		MoveFolderContents(newLocation, oldLocation);
		DeleteDirectory(oldLocation);
		PersistFiles();
	}
	
	// populate entries from the new directory
	SetupDirectory(newLocation);
}

void ObjectStore::SetupDirectory(StringParam storePath)
{
	if (DirectoryExists(storePath) == false)
	{
		//create the directory to contain stored objects.
		CreateDirectoryAndParents(storePath);
	}

	if (DirectoryExists(storePath))
	{
		PopulateEntries(storePath);
	}
	else
	{
		Warn("Failed to create directory for ObjectStore: %s", storePath.c_str());
	}
}

void ObjectStore::PopulateEntries(StringParam storePath)
{
  mEntries.Clear();

  FileRange filesInDirectory(storePath);
  for (; !filesInDirectory.Empty(); filesInDirectory.PopFront())
  {
	String filename = filesInDirectory.Front();
	String name = FilePath::GetFileNameWithoutExtension(filename);
	mEntries.PushBack(name);
  }
}

String ObjectStore::BuildStorePath()
{
	return FilePath::Combine(GetUserApplicationDirectory(), "Store");
}

String ObjectStore::BuildTrashPath()
{
	return FilePath::Combine(GetUserApplicationDirectory(), "StoreTrash");
}

String ObjectStore::GetFullFilePath(StringParam storePath, StringParam entryName)
{
	return FilePath::CombineWithExtension(storePath, entryName, ".data");
}

bool ObjectStore::IsEntryStored(StringParam name)
{
  if (!mEntries.Empty())
  {
	int count = mEntries.Size();
	for (int i = 0; i < count; ++i)
	{
	  if (mEntries[i] == name)
		return true;
	}

	return false;
  }
  else
  {
	// Check to is if file exists.
	String storeFile = GetFullFilePath(BuildStorePath(), name);
	return FileExists(storeFile);
  }
}

uint ObjectStore::GetEntryCount()
{
  return (uint)mEntries.Size();
}

String ObjectStore::GetEntryAt(uint index)
{
  if (mEntries.Empty())
	return String();

  if (index >= mEntries.Size())
	return String();

  return mEntries[index];
}

// Store an object.
StoreResult::Enum ObjectStore::Store(StringParam name, Cog* object)
{
  ReturnIf(object == nullptr, StoreResult::Failed, "Can not store null object.");

  String storeFile = GetFullFilePath(BuildStorePath(), name);

  // Default is added
  StoreResult::Enum result = StoreResult::Added;

  // If the file already exists set the result to replaced.
  if (FileExists(storeFile))
	result = StoreResult::Replaced;

  Status status;
  // Create a text serializer
  ObjectSaver saver;
  // Add a saving context so that ids are relative
  CogSavingContext savingContext;

  // Attempt to open the file
  saver.Open(status, storeFile.c_str());

  if (status)
  {
	if (result == StoreResult::Added)
	  mEntries.PushBack(name);

	saver.SetSerializationContext(&savingContext);
	saver.SaveFullObject(object);
	saver.Close();
	PersistFiles();
  }
  else
  {
	PlasmaPrintFilter(Filter::DefaultFilter, "Failed to store object %s", name.c_str());
	return StoreResult::Failed;
  }

  return result;
}

// Restore an object.
Cog* ObjectStore::Restore(StringParam name, Space* space)
{
  if (space == nullptr)
  {
	DoNotifyException("Invalid ObjectStore Restore",
					  "The space passed in was null. Cannot restore an object "
					  "to an invalid space.");
	return nullptr;
  }

  String storeFile = GetFullFilePath(BuildStorePath(), name);

  // Test to see if a file exists and restore it from
  // file if it exists
  if (FileExists(storeFile))
  {
	Cog* cog = space->CreateNamed(storeFile);
	return cog;
  }

  return nullptr;
}

Cog* ObjectStore::RestoreOrArchetype(StringParam name, Archetype* archetype, Space* space)
{
  Cog* cog = Restore(name, space);

  if (cog == nullptr && space != nullptr)
  {
	// Object was not present in store use archetype.
	cog = space->Create(archetype);
	if (cog == nullptr)
	{
	  PlasmaPrintFilter(Filter::DefaultFilter, "Failed to restore object %s", name.c_str());
	}
  }

  return cog;
}

void ObjectStore::Erase(StringParam name)
{
  // Get the name of the file to remove
  String storeFile = GetFullFilePath(BuildStorePath(), name);

  // First check if it exists
  if (FileExists(storeFile))
  {
	// Get the path to where we're moving the file to (in the trash directory)
	String trashDirectory = BuildTrashPath();
	String fileDestination = GetFullFilePath(trashDirectory, name);

	// Create the trash directory if it doesn't exist
	CreateDirectoryAndParents(trashDirectory);

	// Move the file
	MoveFile(fileDestination, storeFile);

	mEntries.EraseValue(name);
	PersistFiles();
  }
}

void ObjectStore::ClearStore()
{
	mEntries.Clear();

	String storeDirectory = BuildStorePath();
	String trashDirectory = BuildTrashPath();

	if (DirectoryExists(storeDirectory))
	{
		// Create the trash directory if it doesn't exist
		CreateDirectoryAndParents(trashDirectory);

		MoveFolderContents(trashDirectory, BuildStorePath());
		PersistFiles();
	}

	SetupDirectory(storeDirectory);
}

} // namespace Plasma
