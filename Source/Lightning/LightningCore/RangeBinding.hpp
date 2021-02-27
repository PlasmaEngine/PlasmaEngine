// MIT Licensed (see LICENSE.md).

#pragma once
#ifndef LIGHTNING_RANGE_BINDING_HPP
#  define LIGHTNING_RANGE_BINDING_HPP

namespace Lightning
{
template <typename T>
LightningDefineType(Range<T>, builder, type)
{
  type->CopyMode = LS::TypeCopyMode::ReferenceType;
  LightningFullBindDestructor(builder, type, LightningSelf);
  LightningFullBindConstructor(builder, type, LightningSelf, LightningNoNames);
  LightningFullBindGetterSetter(builder, type, &LightningSelf::GetAll, LightningNoOverload, LightningNoSetter, LightningNoOverload, "All");
  LightningFullBindGetterSetter(
      builder, type, &LightningSelf::GetCurrent, LightningNoOverload, LightningNoSetter, LightningNoOverload, "Current");
  LightningFullBindGetterSetter(
      builder, type, &LightningSelf::IsEmpty, LightningNoOverload, LightningNoSetter, LightningNoOverload, "IsEmpty");
  LightningFullBindGetterSetter(
      builder, type, &LightningSelf::IsNotEmpty, LightningNoOverload, LightningNoSetter, LightningNoOverload, "IsNotEmpty");
  LightningFullBindMethod(builder, type, &LightningSelf::MoveNext, LightningNoOverload, "MoveNext", LightningNoNames);
  LightningFullBindMethod(builder, type, &LightningSelf::Reset, LightningNoOverload, "Reset", LightningNoNames);
}

// For when we are binding a standard range on a container
template <typename RangeAdapterBaseType>
class RangeAdapter : public RangeAdapterBaseType
{
  typedef typename RangeAdapterBaseType::FrontResult FrontResult;

public:
  RangeAdapterBaseType* All()
  {
    return this;
  }

  typename RangeAdapterBaseType::FrontResult Current()
  {
    if (this->IsEmpty())
    {
      // Throw an exception since the range was empty and we called Current
      if (ExecutableState::CallingState)
        ExecutableState::CallingState->ThrowException("The range reached the end and an attempt was made to get the "
                                                      "current value");

      return GetInvalid<FrontResult>();
    }

    return this->Front();
  }

  bool IsNotEmpty()
  {
    return !this->Empty();
  }

  bool IsEmpty()
  {
    return this->Empty();
  }

  void MoveNext()
  {
    if (this->IsEmpty())
    {
      // Throw an exception since the range was empty and we called MoveNext
      if (ExecutableState::CallingState)
        ExecutableState::CallingState->ThrowException("The range reached the end, but then an attempt was made to make "
                                                      "it iterate forward more");
      return;
    }

    this->PopFront();
  }
};

#  define LightningDeclareRange(RangeType) LightningDeclareExternalType(RangeType)
#  define LightningDefineRange(RangeType)                                                                                  \
    LightningDefineExternalBaseType(RangeType, LS::TypeCopyMode::ReferenceType, builder, type)                             \
    {                                                                                                                  \
      LightningBindDefaultCopyDestructor();                                                                                \
      LightningFullBindGetterSetter(                                                                                       \
          builder, type, &LS::RangeAdapter<LightningSelf>::All, LightningNoOverload, LightningNoSetter, LightningNoOverload, "All");   \
      LightningFullBindGetterSetter(builder,                                                                               \
                                type,                                                                                  \
                                &LS::RangeAdapter<LightningSelf>::Current,                                                 \
                                LightningNoOverload,                                                                       \
                                LightningNoSetter,                                                                         \
                                LightningNoOverload,                                                                       \
                                "Current");                                                                            \
      LightningFullBindGetterSetter(builder,                                                                               \
                                type,                                                                                  \
                                &LS::RangeAdapter<LightningSelf>::IsNotEmpty,                                              \
                                LightningNoOverload,                                                                       \
                                LightningNoSetter,                                                                         \
                                LightningNoOverload,                                                                       \
                                "IsNotEmpty");                                                                         \
      LightningFullBindGetterSetter(builder,                                                                               \
                                type,                                                                                  \
                                &LS::RangeAdapter<LightningSelf>::IsEmpty,                                                 \
                                LightningNoOverload,                                                                       \
                                LightningNoSetter,                                                                         \
                                LightningNoOverload,                                                                       \
                                "IsEmpty");                                                                            \
      LightningFullBindMethod(                                                                                             \
          builder, type, &LS::RangeAdapter<LightningSelf>::MoveNext, LightningNoOverload, "MoveNext", LightningNoNames);           \
    }
#  define LightningInitializeRange(RangeType) LightningInitializeExternalType(RangeType)
#  define LightningInitializeRangeAs(RangeType, Name) LightningInitializeExternalTypeAs(RangeType, Name)
} // namespace Lightning

#endif
