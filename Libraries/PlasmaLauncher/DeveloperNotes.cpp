// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void DeveloperNotes::Serialize(Serializer& stream)
{
  SerializeNameDefault(mFileName, String());
  SerializeNameDefault(mNotes, String());
  SerializeNameDefault(mDate, String());
}

} // namespace Plasma
