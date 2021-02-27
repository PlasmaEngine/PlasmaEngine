// MIT Licensed (see LICENSE.md).
#pragma once

// Standard includes
#include "Core/Geometry/GeometryStandard.hpp"

namespace Plasma
{
class PropertyPath;
class MetaSelection;

// Meta library
class PlasmaNoImportExport MetaLibrary : public Lightning::StaticLibrary
{
public:
  LightningDeclareStaticLibraryInternals(MetaLibrary, "PlasmaEngine");

  static void Initialize();
  static void Shutdown();
};

const Guid cInvalidUniqueId = (Guid)(u64)-1;
} // namespace Plasma

// Project includes
#include "NativeTypeConversion.hpp"
#include "Singleton.hpp"
#include "Object.hpp"
#include "HandleManagers.hpp"
#include "ThreadSafeReferenceCounted.hpp"
#include "Tags.hpp"
#include "Event.hpp"
#include "CommonHandleManagers.hpp"
#include "ObjectRestoreState.hpp"
#include "PropertyHandle.hpp"
#include "MetaBindingExtensions.hpp"
#include "Notification.hpp"
#include "MetaDatabase.hpp"
#include "MetaHandles.hpp"
#include "MetaArray.hpp"
#include "MetaComposition.hpp"
#include "LocalModifications.hpp"
#include "MetaSelection.hpp"
#include "PlasmaMetaArray.hpp"
#include "MetaMath.hpp"
#include "MetaEditorExtensions.hpp"
#include "SourceControl.hpp"
#include "MetaLibraryExtensions.hpp"
#include "AttributeExtension.hpp"
