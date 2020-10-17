// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineStaticLibrary(SerializationLibrary)
{
  builder.CreatableInScriptDefault = false;

  LightningInitializeType(MetaSerialization);
  LightningInitializeType(SerializationFilter);
  LightningInitializeType(PrimitiveMetaSerialization<Integer>);
  LightningInitializeType(PrimitiveMetaSerialization<Integer2>);
  LightningInitializeType(PrimitiveMetaSerialization<Integer3>);
  LightningInitializeType(PrimitiveMetaSerialization<Integer4>);
  LightningInitializeType(PrimitiveMetaSerialization<String>);
  LightningInitializeType(PrimitiveMetaSerialization<Boolean>);
  LightningInitializeType(PrimitiveMetaSerialization<Real>);
  LightningInitializeType(PrimitiveMetaSerialization<Real2>);
  LightningInitializeType(PrimitiveMetaSerialization<Real3>);
  LightningInitializeType(PrimitiveMetaSerialization<Real4>);
  LightningInitializeType(PrimitiveMetaSerialization<Mat2>);
  LightningInitializeType(PrimitiveMetaSerialization<Mat3>);
  LightningInitializeType(PrimitiveMetaSerialization<Mat4>);
  LightningInitializeType(PrimitiveMetaSerialization<Quat>);
  LightningInitializeType(MetaStringSerialization);
  LightningInitializeType(EnumMetaSerialization);

  PlasmaBindSerializationPrimitiveExternal(Integer);
  PlasmaBindSerializationPrimitiveExternal(Integer2);
  PlasmaBindSerializationPrimitiveExternal(Integer3);
  PlasmaBindSerializationPrimitiveExternal(Integer4);
  PlasmaBindSerializationPrimitiveExternal(String);
  PlasmaBindSerializationPrimitiveExternal(Boolean);
  PlasmaBindSerializationPrimitiveExternal(Real);
  PlasmaBindSerializationPrimitiveExternal(Real2);
  PlasmaBindSerializationPrimitiveExternal(Real3);
  PlasmaBindSerializationPrimitiveExternal(Real4);
  PlasmaBindSerializationPrimitiveExternal(Mat2);
  PlasmaBindSerializationPrimitiveExternal(Mat3);
  PlasmaBindSerializationPrimitiveExternal(Mat4);
  PlasmaBindSerializationPrimitiveExternal(Quat);
  PlasmaBindSerializationPrimitiveExternal(Enum);

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

void SerializationLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  LightningTypeId(Integer)->Add(new PrimitiveMetaSerialization<Integer>());
  LightningTypeId(Integer2)->Add(new PrimitiveMetaSerialization<Integer2>());
  LightningTypeId(Integer3)->Add(new PrimitiveMetaSerialization<Integer3>());
  LightningTypeId(Integer4)->Add(new PrimitiveMetaSerialization<Integer4>());
  LightningTypeId(String)->Add(new MetaStringSerialization);
  LightningTypeId(Boolean)->Add(new PrimitiveMetaSerialization<Boolean>());
  LightningTypeId(Real)->Add(new PrimitiveMetaSerialization<Real>());
  LightningTypeId(Real2)->Add(new PrimitiveMetaSerialization<Real2>());
  LightningTypeId(Real3)->Add(new PrimitiveMetaSerialization<Real3>());
  LightningTypeId(Real4)->Add(new PrimitiveMetaSerialization<Real4>());
  LightningTypeId(Mat2)->Add(new PrimitiveMetaSerialization<Mat2>());
  LightningTypeId(Mat3)->Add(new PrimitiveMetaSerialization<Mat3>());
  LightningTypeId(Mat4)->Add(new PrimitiveMetaSerialization<Mat4>());
  LightningTypeId(Quat)->Add(new PrimitiveMetaSerialization<Quat>());
}

void SerializationLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Plasma
