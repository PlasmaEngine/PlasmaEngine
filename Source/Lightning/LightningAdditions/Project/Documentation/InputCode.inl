#include "../Lightning/Lightning.hpp"

using namespace Lightning;

int main()
{
  LightningStartup(Debugging::UseLightningErrorHandler);

  CompilationErrors errors;
  errors.AddCallback(DefaultErrorCallback, nullptr);
  Project proj(errors);

  char buffer[4096] = {0};
  gets(buffer);

  proj.AddCodeFromString(buffer);

  Module dep(errors);
  proj.Compile("Test", dep, EvaluationMode::Project);
  
  LightningShutdown();

  system("pause");
  
  return 0;
}
