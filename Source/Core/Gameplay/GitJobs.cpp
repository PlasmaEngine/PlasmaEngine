// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
void GitCloneJob::OnCloneComplete(void* userData)
{
  GitCloneJob* self = (GitCloneJob*)userData;
  self->ExecuteAsyncEnd();
}

void GitCloneJob::ExecuteAsyncBegin()
{
  ZoneScoped;
  Git::GetInstance()->Clone(mUrl, mDirectory, &OnCloneComplete, this);
}

} // namespace Plasma
