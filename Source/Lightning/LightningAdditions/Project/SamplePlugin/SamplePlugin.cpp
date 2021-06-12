#include "SamplePlugin.hpp"

//***************************************************************************
LightningDefineStaticLibraryAndPlugin(SampleLibrary, SamplePlugin)
{
  LightningInitializeType(Sample);
}

//***************************************************************************
void SamplePlugin::PreBuild(Lightning::BuildEvent* event)
{
}

//***************************************************************************
void SamplePlugin::Uninitialize()
{
}

//***************************************************************************
LightningDefineType(Sample, builder, type)
{
  LightningBindMethod(Run);
}

//***************************************************************************
void Sample::Run(StringBuilderExtended* builder, ArrayClass<Byte>* bytes)
{
  String s = builder->ToString();
  printf("It's ALIVE %s YAY!\n", s.c_str());
}
