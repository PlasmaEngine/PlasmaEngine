// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{
class GitCloneJob : public Job
{
public:
  String mUrl;
  String mDirectory;

  void ExecuteAsyncBegin() override;

private:
  static void OnCloneComplete(void* userData);
};

}; // namespace Plasma
