// MIT Licensed (see LICENSE.md).
#pragma once

#include "Core/Support/SupportStandard.hpp"
#include "PlatformStandard.hpp"
#include "Core/Meta/MetaStandard.hpp"

#include "Lightning/LightningCore/Precompiled.hpp"
using namespace Lightning;

namespace Plasma
{

// Serialization library
class PlasmaNoImportExport SerializationLibrary : public Lightning::StaticLibrary
{
public:
  LightningDeclareStaticLibraryInternals(SerializationLibrary, "PlasmaEngine");

  static void Initialize();
  static void Shutdown();
};

} // namespace Plasma

#include "Serialization.hpp"
#include "PlasmaContainers.hpp"
#include "SerializationBuilder.hpp"
#include "Tokenizer.hpp"
#include "SerializationUtility.hpp"
#include "Text.hpp"
#include "Binary.hpp"
#include "DataTreeNode.hpp"
#include "DataTree.hpp"
#include "Simple.hpp"
#include "DefaultSerializer.hpp"
#include "Tokenizer.hpp"
#include "SerializationTraits.hpp"
#include "EnumSerialization.hpp"
#include "MetaSerialization.hpp"
#include "MathSerialization.hpp"
