#define main CompilingLightningScriptMain
#include "CompilingLightningScript.inl"
#undef main

#define main RunningLightningScriptMain
#include "RunningLightningScript.inl"
#undef main

#include "Wallaby.inl"
#include "WallabyBinding.inl"

int main(void)
{
  {
    int result = CompilingLightningScriptMain();
    ErrorIf(result != 0);
  }
  {
    int result = RunningLightningScriptMain();
    ErrorIf(result != 0);
  }

  {
    LightningSetup setup;
    LibraryRef wallaby = Wallaby::GetLibrary();
  }

  system("pause");
  return 0;
}
