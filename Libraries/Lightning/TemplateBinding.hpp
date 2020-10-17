// MIT Licensed (see LICENSE.md).

#pragma once
#ifndef LIGHTNING_TEMPLATE_BINDING_HPP
#  define LIGHTNING_TEMPALTE_BINDING_HPP

namespace Lightning
{
namespace PropertyBinding
{
enum Enum
{
  Get,
  Set,
  GetSet
};
}

// All things relevant to binding methods
class PlasmaShared TemplateBinding
{
public:
  // Given a comma delimited string of names (eg, "destination, source, size")
  // this will fill in the parameter array with those names. The number of
  // parameters must match the number of parsed names. All names should be
  // lower-camel case For generic use, if the names list is empty, this will
  // immediately return with no errors
  static void ParseParameterArrays(ParameterArray& parameters, StringRange commaDelimitedNames);

  // Validate that a destructor has been bound (asserts within binding
  // constructors) This just returns the same bound type that is used, which
  // allows us to use this as an expression
  static BoundType* ValidateConstructorBinding(BoundType* type);

// Include all the binding code
#  include "MethodBinding.inl"
#  include "VirtualMethodBinding.inl"
#  include "ConstructorBinding.inl"

  //*** BOUND DESTRUCTOR ***// Wraps a destructor call with the Lightning signature
  template <typename Class>
  static void BoundDestructor(Call& call, ExceptionReport& report)
  {
    //// Get our self object
    // Class* self = (Class*)call.GetHandle(Call::This).Dereference();

    // We need to investigate why the above call doesn't work
    // Unfortunately this comment is written whilst looking back, and I don't
    // understand why I had commented the above out. My guess would be that for
    // struct types (value types) the 'this type' is actually the ref instead of
    // the type itself... something like that Most likely the handle is storing
    // the BoundType*, eg Quaternion, instead of the IndirectionType* Why
    // doesn't this appear elswhere, say the field get functions?
    Handle& selfHandle = call.GetHandle(Call::This);
    Class* self = (Class*)selfHandle.Dereference();

    // Explicitly call the destructor of the class
    // If this is being destructed in an exception scenario the handle could be
    // null
    if (self)
      self->~Class();
  }

  //*** BUILDER DESTRUCTOR ***// Generates a Lightning function to call a class
  // destructor
  template <typename Class>
  static Function* FromDestructor(LibraryBuilder& builder, BoundType* classBoundType)
  {
    return builder.AddBoundDestructor(classBoundType, BoundDestructor<Class>);
  }

  //*** BOUND INSTANCE FIELD GET ***//
  template <typename FieldType, typename Class, FieldType Class::*field>
  static void BoundInstanceGet(Call& call, ExceptionReport& report)
  {
    // Get our self object
    Class* self = (Class*)call.GetHandle(Call::This).Dereference();

    // Get the value of the member
    FieldType& value = self->*field;

    // Get the member's value by returning it
    call.Set(Call::Return, value);
  }

  //*** BOUND INSTANCE FIELD SET ***//
  template <typename FieldType, typename Class, FieldType Class::*field>
  static void BoundInstanceSet(Call& call, ExceptionReport& report)
  {
    // Get our self object
    Class* self = (Class*)call.GetHandle(Call::This).Dereference();

    // Read in the value that we're trying to set
    byte* stackPointer = call.GetArgumentPointer<LightningBindingType(FieldType)>(0);

    // If read is invalid, throw a more specific exception
    if (report.HasThrownExceptions())
      return ExecutableState::GetCallingState()->ThrowException("Error: Cannot assign null.");

    LightningBindingType(FieldType) value = call.CastArgumentPointer<LightningBindingType(FieldType)>(stackPointer);

    // Set the value of the member
    self->*field = value;
  }

  //*** BUILDER INSTANCE CONST FIELD ***//
  template <typename FieldPointer, FieldPointer field, typename Class, typename FieldType>
  static Property* FromField(LibraryBuilder& builder,
                             BoundType* owner,
                             StringParam name,
                             const FieldType Class::*dummy,
                             PropertyBinding::Enum mode)
  {
    ErrorIf(dummy != field, "The dummy should always match our template member");
    BoundFn get = BoundInstanceGet<const FieldType, Class, field>;
    ErrorIf(mode != PropertyBinding::Get,
            "The field is const and therefore a setter cannot be generated "
            "(use PropertyBinding::Get)");
    return builder.AddBoundGetterSetter(owner, name, LightningTypeId(FieldType), nullptr, get, MemberOptions::None);
  }

  //*** BUILDER INSTANCE FIELD ***//
  // Generates a Lightning property by creating get/set functions to wrap the member
  // variable and binding them
  template <typename FieldPointer, FieldPointer field, typename Class, typename FieldType>
  static Property* FromField(
      LibraryBuilder& builder, BoundType* owner, StringParam name, FieldType Class::*dummy, PropertyBinding::Enum mode)
  {
    ErrorIf(dummy != field, "The dummy should always match our template member");
    BoundFn set = BoundInstanceSet<FieldType, Class, field>;
    BoundFn get = BoundInstanceGet<FieldType, Class, field>;

    if (mode == PropertyBinding::Get)
    {
      set = nullptr;
    }
    if (mode == PropertyBinding::Set)
    {
      get = nullptr;
    }

    return builder.AddBoundGetterSetter(owner, name, LightningTypeId(FieldType), set, get, MemberOptions::None);
  }

  //*** BOUND STATIC FIELD GET ***//
  template <typename FieldType, FieldType* field>
  static void BoundStaticGet(Call& call, ExceptionReport& report)
  {
    // Get the value of the member
    FieldType& value = *field;

    // Get the member's value by returning it
    call.Set(Call::Return, value);
  }

  //*** BOUND STATIC FIELD SET ***//
  template <typename FieldType, FieldType* field>
  static void BoundStaticSet(Call& call, ExceptionReport& report)
  {
    // Read in the value that we're trying to set
    byte* stackPointer = call.GetArgumentPointer<LightningBindingType(FieldType)>(0);

    // If read is invalid, throw a more specific exception
    if (report.HasThrownExceptions())
      return ExecutableState::GetCallingState()->ThrowException("Error: Cannot assign null.");

    LightningBindingType(FieldType) value = call.CastArgumentPointer<LightningBindingType(FieldType)>(stackPointer);

    // Set the value of the member
    *field = value;
  }

  //*** BUILDER STATIC CONST FIELD ***//
  template <typename FieldPointer, FieldPointer field, typename FieldType>
  static Property* FromField(
      LibraryBuilder& builder, BoundType* owner, StringParam name, const FieldType* dummy, PropertyBinding::Enum mode)
  {
    ErrorIf(dummy != field, "The dummy should always match our template member");
    BoundFn get = BoundStaticGet<const FieldType, field>;
    ErrorIf(mode != PropertyBinding::Get,
            "The field is const and therefore a setter cannot be generated "
            "(use PropertyBinding::Get)");
    return builder.AddBoundGetterSetter(owner, name, LightningTypeId(FieldType), nullptr, get, MemberOptions::Static);
  }

  //*** BUILDER STATIC FIELD ***//
  // Generates a Lightning property by creating get/set functions to wrap the global
  // variable and binding them
  template <typename FieldPointer, FieldPointer field, typename FieldType>
  static Property*
  FromField(LibraryBuilder& builder, BoundType* owner, StringParam name, FieldType* dummy, PropertyBinding::Enum mode)
  {
    ErrorIf(dummy != field, "The dummy should always match our template member");
    BoundFn set = BoundStaticSet<FieldType, field>;
    BoundFn get = BoundStaticGet<FieldType, field>;

    if (mode == PropertyBinding::Get)
      set = nullptr;
    if (mode == PropertyBinding::Set)
      get = nullptr;

    return builder.AddBoundGetterSetter(owner, name, LightningTypeId(FieldType), set, get, MemberOptions::Static);
  }

  //*** BUILDER INSTANCE PROPERTY GET/SET ***//
  template <typename GetterType,
            GetterType getter,
            typename SetterType,
            SetterType setter,
            typename Class,
            typename GetType,
            typename SetType>
  static GetterSetter* FromGetterSetter(LibraryBuilder& builder,
                                        BoundType* owner,
                                        StringRange name,
                                        GetType (Class::*dummyGetter)(),
                                        void (Class::*dummySetter)(SetType))
  {
    ReturnIf(LightningTypeId(GetType) != LightningTypeId(SetType),
             nullptr,
             "Cannot bind a Get/Set property type that has a different fundamental "
             "type for the getter's return value and setters input value");

    ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
    ErrorIf(dummySetter != setter, "The dummy getter should always match our template member");

    BoundFn boundGet = BoundInstanceReturn<GetterType, getter, Class, GetType>;
    BoundFn boundSet = BoundInstance<SetterType, setter, Class, SetType>;

    return builder.AddBoundGetterSetter(owner, name, LightningTypeId(GetType), boundSet, boundGet, MemberOptions::None);
  }

  //*** BUILDER INSTANCE PROPERTY CONST GET/SET ***//
  template <typename GetterType,
            GetterType getter,
            typename SetterType,
            SetterType setter,
            typename Class,
            typename GetType,
            typename SetType>
  static GetterSetter* FromGetterSetter(LibraryBuilder& builder,
                                        BoundType* owner,
                                        StringRange name,
                                        GetType (Class::*dummyGetter)() const,
                                        void (Class::*dummySetter)(SetType))
  {
    ReturnIf(LightningTypeId(GetType) != LightningTypeId(SetType),
             nullptr,
             "Cannot bind a Get/Set property type that has a different fundamental "
             "type for the getter's return value and setters input value");

    ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
    ErrorIf(dummySetter != setter, "The dummy getter should always match our template member");

    BoundFn boundGet = BoundInstanceReturn<GetterType, getter, Class, GetType>;
    BoundFn boundSet = BoundInstance<SetterType, setter, Class, SetType>;

    return builder.AddBoundGetterSetter(owner, name, LightningTypeId(GetType), boundSet, boundGet, MemberOptions::None);
  }

  //*** BUILDER INSTANCE PROPERTY GET ***//
  template <typename GetterType,
            GetterType getter,
            typename SetterType,
            SetterType setter,
            typename Class,
            typename GetType>
  static GetterSetter* FromGetterSetter(
      LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (Class::*dummyGetter)(), NullPointerType)
  {
    ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
    BoundFn boundGet = BoundInstanceReturn<GetterType, getter, Class, GetType>;
    return builder.AddBoundGetterSetter(owner, name, LightningTypeId(GetType), nullptr, boundGet, MemberOptions::None);
  }

  //*** BUILDER INSTANCE PROPERTY CONST GET ***//
  template <typename GetterType,
            GetterType getter,
            typename SetterType,
            SetterType setter,
            typename Class,
            typename GetType>
  static GetterSetter* FromGetterSetter(LibraryBuilder& builder,
                                        BoundType* owner,
                                        StringRange name,
                                        GetType (Class::*dummyGetter)() const,
                                        NullPointerType)
  {
    ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
    BoundFn boundGet = BoundInstanceReturn<GetterType, getter, Class, GetType>;
    return builder.AddBoundGetterSetter(owner, name, LightningTypeId(GetType), nullptr, boundGet, MemberOptions::None);
  }

  //*** BUILDER INSTANCE PROPERTY SET ***//
  template <typename GetterType,
            GetterType getter,
            typename SetterType,
            SetterType setter,
            typename Class,
            typename SetType>
  static GetterSetter* FromGetterSetter(
      LibraryBuilder& builder, BoundType* owner, StringRange name, NullPointerType, void (Class::*dummySetter)(SetType))
  {
    ErrorIf(dummySetter != setter, "The dummy setter should always match our template member");
    BoundFn boundSet = BoundInstance<SetterType, setter, Class, SetType>;
    return builder.AddBoundGetterSetter(owner, name, LightningTypeId(SetType), boundSet, nullptr, MemberOptions::None);
  }

  //*** BUILDER STATIC PROPERTY GET/SET ***//
  template <typename GetterType,
            GetterType getter,
            typename SetterType,
            SetterType setter,
            typename GetType,
            typename SetType>
  static GetterSetter* FromGetterSetter(LibraryBuilder& builder,
                                        BoundType* owner,
                                        StringRange name,
                                        GetType (*dummyGetter)(),
                                        void (*dummySetter)(SetType))
  {
    ReturnIf(LightningTypeId(GetType) != LightningTypeId(SetType),
             nullptr,
             "Cannot bind a Get/Set property type that has a different fundamental "
             "type for the getter's return value and setters input value");

    ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
    ErrorIf(dummySetter != setter, "The dummy getter should always match our template member");

    BoundFn boundGet = BoundStaticReturn<GetterType, getter, GetType>;
    BoundFn boundSet = BoundStatic<SetterType, setter, SetType>;

    return builder.AddBoundGetterSetter(owner, name, LightningTypeId(GetType), boundSet, boundGet, MemberOptions::Static);
  }

  //*** BUILDER STATIC PROPERTY GET ***//
  template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename GetType>
  static GetterSetter* FromGetterSetter(
      LibraryBuilder& builder, BoundType* owner, StringRange name, GetType (*dummyGetter)(), NullPointerType)
  {
    ErrorIf(dummyGetter != getter, "The dummy getter should always match our template member");
    BoundFn boundGet = BoundStaticReturn<GetterType, getter, GetType>;
    return builder.AddBoundGetterSetter(owner, name, LightningTypeId(GetType), nullptr, boundGet, MemberOptions::Static);
  }

  //*** BUILDER STATIC PROPERTY SET ***//
  template <typename GetterType, GetterType getter, typename SetterType, SetterType setter, typename SetType>
  static GetterSetter* FromGetterSetter(
      LibraryBuilder& builder, BoundType* owner, StringRange name, NullPointerType, void (*dummySetter)(SetType))
  {
    ErrorIf(dummySetter != setter, "The dummy setter should always match our template member");
    BoundFn boundSet = BoundStatic<SetterType, setter, SetType>;
    return builder.AddBoundGetterSetter(owner, name, LightningTypeId(SetType), boundSet, nullptr, MemberOptions::Static);
  }
};

// If we want more readable code when not specifying a getter or setter in
// LightningFullBindGetterSetter
#  define LightningNoSetter nullptr
#  define LightningNoGetter nullptr

// When we want to specify that a method binding has no parameter names or
// documentation, we use this macro (more readable and clear)
#  define LightningNoNames nullptr
#  define LightningNoDocumentation nullptr

// When using the LightningFullBindMethod macro if we're binding a method that has
// no overloads then we use this constant for the parameter 'OverloadResolution'
#  define LightningNoOverload

// Workhorse macro for binding methods
#  define LightningFullBindMethod(                                                                                         \
      LightningBuilder, LightningType, MethodPointer, OverloadResolution, Name, SpaceDelimitedParameterNames)                  \
    LS::TemplateBinding::FromMethod<decltype(OverloadResolution MethodPointer), MethodPointer>(                        \
        LightningBuilder, LightningType, Name, SpaceDelimitedParameterNames, OverloadResolution(MethodPointer))

// Workhorse macro for binding virtual methods
#  define LightningFullBindVirtualMethod(LightningBuilder, LightningType, MethodPointer, NameOrNull)                               \
    LS::TemplateBinding::FromVirtual<decltype(MethodPointer), MethodPointer>(                                          \
        LightningBuilder, LightningType, Name, SpaceDelimitedParameterNames, (MethodPointer))

// Bind a constructor that takes any number of arguments
// Due to the inability to get a 'member function pointer' to a constructor, the
// arguments must always be specified
#  define LightningFullBindConstructor(LightningBuilder, LightningType, Class, SpaceDelimitedParameterNames, ...)                  \
    LS::TemplateBinding::FromConstructor<Class, ##__VA_ARGS__>(LightningBuilder, LightningType, SpaceDelimitedParameterNames)

// Bind a constructor that takes any number of arguments (this binds a special
// constructor that lets Lightning know about the type's v-table) Due to the
// inability to get a 'member function pointer' to a constructor, the arguments
// must always be specified
#  define LightningFullBindConstructorVirtual(LightningBuilder, LightningType, Class, SpaceDelimitedParameterNames, ...)           \
    LS::TemplateBinding::FromConstructorVirtual<Class, ##__VA_ARGS__>(                                                 \
        LightningBuilder, LightningType, SpaceDelimitedParameterNames)

// Bind the destructor of a class
// The destructor should ALWAYS be bound if the constructor is bound
#  define LightningFullBindDestructor(LightningBuilder, LightningType, Class)                                                      \
    LS::TemplateBinding::FromDestructor<Class>(LightningBuilder, LightningType)

// Bind data members as properties
#  define LightningFullBindField(LightningBuilder, LightningType, FieldPointer, Name, PropertyBinding)                             \
    LS::TemplateBinding::FromField<decltype(FieldPointer), FieldPointer>(                                              \
        LightningBuilder, LightningType, Name, FieldPointer, PropertyBinding)

// Bind data members with an offset
#  define LightningFullBindMember(LightningBuilder, LightningType, MemberName, Name, PropertyBinding)                              \
    [&]() {                                                                                                            \
      ErrorIf(LightningTypeId(decltype(LightningSelf::MemberName))->GetCopyableSize() != sizeof(LightningSelf::MemberName),        \
              "When binding a member the type must match the exact size it is "                                        \
              "expected to be in Lightning "                                                                               \
              "(e.g. all reference types must be a Handle). Most likely you want "                                     \
              "LightningBindField.");                                                                                      \
      return LightningBuilder.AddBoundField(LightningType,                                                                     \
                                        Name,                                                                          \
                                        LightningTypeId(decltype(LightningSelf::MemberName)),                                  \
                                        PlasmaOffsetOf(LightningSelf, MemberName),                                           \
                                        PropertyBinding);                                                              \
    }()

// Bind a property (getter and setter in C++) to Lightning
// A property will appear like a member, but it will invoke the getter when
// being read, and the setter when being written to
#  define LightningFullBindGetterSetter(                                                                                   \
      LightningBuilder, LightningType, GetterMethodPointer, GetterOverload, SetterMethodPointer, SetterOverload, Name)         \
    LS::TemplateBinding::FromGetterSetter<decltype(GetterOverload GetterMethodPointer),                                \
                                          GetterMethodPointer,                                                         \
                                          decltype(SetterOverload SetterMethodPointer),                                \
                                          SetterMethodPointer>(                                                        \
        LightningBuilder, LightningType, Name, GetterMethodPointer, SetterMethodPointer)

// Bind a type as being an enum (verifies that the size matches)
#  define LightningFullBindEnum(LightningBuilder, LightningType, SpecialTypeEnum)                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
      (LightningType)->SpecialType = SpecialTypeEnum;                                                                      \
      if ((SpecialTypeEnum) == LS::SpecialType::Enumeration)                                                           \
        (LightningType)->ToStringFunction = LS::VirtualMachine::EnumerationToString;                                       \
      else                                                                                                             \
        (LightningType)->ToStringFunction = LS::VirtualMachine::FlagsToString;                                             \
      ErrorIf((LightningType)->Size != sizeof(LS::Integer),                                                                \
              "The sizeof(Enum) bound to Lightning must match the sizeof(Integer)");                                       \
      (LightningType)->BaseType = LightningTypeId(LS::Enum);                                                                   \
    } while (false)

// Bind a single value of
#  define LightningFullBindEnumValue(LightningBuilder, LightningType, EnumValue, Name)                                             \
    (LightningBuilder).AddEnumValue((LightningType), (Name), (EnumValue));

// Extra convenience macros that just wrap the above macros (simpler, intended
// for binding values only to our own type) Note that the Property versions only
// add the attribute "Property" and can be used for displaying within a property
// grid
#  define LightningBindConstructor(...) LightningFullBindConstructor(builder, type, LightningSelf, LightningNoNames, ##__VA_ARGS__)
#  define LightningBindDefaultConstructor() LightningBindConstructor()
#  define LightningBindCopyConstructor() LightningBindConstructor(const LightningSelf&)
#  define LightningBindDestructor() LightningFullBindDestructor(builder, type, LightningSelf)
#  define LightningBindDefaultDestructor()                                                                                 \
    LightningBindDefaultConstructor();                                                                                     \
    LightningBindDestructor();
#  define LightningBindDefaultCopyDestructor()                                                                             \
    LightningBindDefaultConstructor();                                                                                     \
    LightningBindCopyConstructor();                                                                                        \
    LightningBindDestructor();

// These versions allow you to rename anything bound
// Note that 'Custom' means we don't apply the Get or Set to the beginning of
// the name
#  define LightningBindOverloadedMethodAs(MethodName, OverloadResolution, Name)                                            \
    LightningFullBindMethod(builder, type, &LightningSelf::MethodName, OverloadResolution, Name, LightningNoNames)
#  define LightningBindMethodAs(MethodName, Name) LightningBindOverloadedMethodAs(MethodName, LightningNoOverload, Name)
#  define LightningBindOverloadedMethodPropertyAs(MethodName, OverloadResolution, Name)                                    \
    LightningFullBindMethod(builder, type, &LightningSelf::MethodName, OverloadResolution, Name, LightningNoNames)                 \
        ->AddAttributeChainable(Lightning::PropertyAttribute)
#  define LightningBindMethodPropertyAs(MethodName, Name)                                                                  \
    LightningBindOverloadedMethodAs(MethodName, LightningNoOverload, Name)->AddAttributeChainable(Lightning::PropertyAttribute)
#  define LightningBindMemberAs(MemberName, Name)                                                                          \
    LightningFullBindMember(builder, type, MemberName, Name, Lightning::MemberOptions::None)
#  define LightningBindMemberPropertyAs(MemberName, Name)                                                                  \
    LightningBindMemberAs(MemberName, Name)->AddAttributeChainable(Lightning::PropertyAttribute)
#  define LightningBindFieldAs(FieldName, Name)                                                                            \
    LightningFullBindField(builder, type, &LightningSelf::FieldName, Name, Lightning::PropertyBinding::GetSet)
#  define LightningBindFieldGetterAs(FieldName, Name)                                                                      \
    LightningFullBindField(builder, type, &LightningSelf::FieldName, Name, Lightning::PropertyBinding::Get)
#  define LightningBindFieldSetterAs(FieldName, Name)                                                                      \
    LightningFullBindField(builder, type, &LightningSelf::FieldName, Name, Lightning::PropertyBinding::Set)
#  define LightningBindFieldPropertyAs(FieldName, Name)                                                                    \
    LightningBindFieldAs(FieldName, Name)->AddAttributeChainable(Lightning::PropertyAttribute)
#  define LightningBindFieldGetterPropertyAs(FieldName, Name)                                                              \
    LightningBindFieldGetterAs(FieldName, Name)->AddAttributeChainable(Lightning::PropertyAttribute)
#  define LightningBindFieldSetterPropertyAs(FieldName, Name)                                                              \
    LightningBindFieldSetterAs(FieldName, Name)->AddAttributeChainable(Lightning::PropertyAttribute)
#  define LightningBindGetterAs(PropertyName, Name)                                                                        \
    LightningFullBindGetterSetter(                                                                                         \
        builder, type, &LightningSelf::Get##PropertyName, LightningNoOverload, LightningNoSetter, LightningNoOverload, Name)
#  define LightningBindSetterAs(PropertyName, Name)                                                                        \
    LightningFullBindGetterSetter(                                                                                         \
        builder, type, LightningNoGetter, LightningNoOverload, &LightningSelf::Set##PropertyName, LightningNoOverload, Name)
#  define LightningBindGetterSetterAs(PropertyName, Name)                                                                  \
    LightningFullBindGetterSetter(builder,                                                                                 \
                              type,                                                                                    \
                              &LightningSelf::Get##PropertyName,                                                           \
                              LightningNoOverload,                                                                         \
                              &LightningSelf::Set##PropertyName,                                                           \
                              LightningNoOverload,                                                                         \
                              Name)
#  define LightningBindGetterPropertyAs(PropertyName, Name)                                                                \
    LightningBindGetterAs(PropertyName, Name)->AddAttributeChainable(Lightning::PropertyAttribute)
#  define LightningBindSetterPropertyAs(PropertyName, Name)                                                                \
    LightningBindSetterAs(PropertyName, Name)->AddAttributeChainable(Lightning::PropertyAttribute)
#  define LightningBindGetterSetterPropertyAs(PropertyName, Name)                                                          \
    LightningBindGetterSetterAs(PropertyName, Name)->AddAttributeChainable(Lightning::PropertyAttribute)
#  define LightningBindCustomGetterAs(PropertyName, Name)                                                                  \
    LightningFullBindGetterSetter(                                                                                         \
        builder, type, &LightningSelf::PropertyName, LightningNoOverload, LightningNoSetter, LightningNoOverload, Name)
#  define LightningBindCustomSetterAs(PropertyName, Name)                                                                  \
    LightningFullBindGetterSetter(                                                                                         \
        builder, type, LightningNoGetter, LightningNoOverload, &LightningSelf::PropertyName, LightningNoOverload, Name)
#  define LightningBindCustomGetterSetterAs(PropertyName, Name)                                                            \
    LightningFullBindGetterSetter(                                                                                         \
        builder, type, &LightningSelf::PropertyName, LightningNoOverload, &LightningSelf::PropertyName, LightningNoOverload, Name)
#  define LightningBindCustomGetterPropertyAs(PropertyName, Name)                                                          \
    LightningBindCustomGetterAs(PropertyName, Name)->AddAttributeChainable(Lightning::PropertyAttribute)
#  define LightningBindCustomSetterPropertyAs(PropertyName, Name)                                                          \
    LightningBindCustomSetterAs(PropertyName, Name)->AddAttributeChainable(Lightning::PropertyAttribute)
#  define LightningBindCustomGetterSetterPropertyAs(PropertyName, Name)                                                    \
    LightningBindCustomGetterSetterAs(PropertyName, Name)->AddAttributeChainable(Lightning::PropertyAttribute)
#  define LightningBindEnumValueAs(EnumValue, Name) LightningFullBindEnumValue(builder, type, EnumValue, Name)

// All these versions assume the name is the same as the property/field/method
// identifier
#  define LightningBindOverloadedMethod(MethodName, OverloadResolution)                                                    \
    LightningBindOverloadedMethodAs(MethodName, OverloadResolution, #MethodName)
#  define LightningBindMethod(MethodName) LightningBindMethodAs(MethodName, #  MethodName)
#  define LightningBindOverloadedMethodProperty(MethodName, OverloadResolution)                                            \
    LightningBindOverloadedPropertyMethodAs(MethodName, OverloadResolution, #MethodName)
#  define LightningBindMethodProperty(MethodName) LightningBindMethodPropertyAs(MethodName, #  MethodName)
#  define LightningBindMember(MemberName) LightningBindMemberAs(MemberName, #  MemberName)
#  define LightningBindMemberProperty(MemberName) LightningBindMemberPropertyAs(MemberName, #  MemberName)
#  define LightningBindField(FieldName) LightningBindFieldAs(FieldName, #  FieldName)
#  define LightningBindFieldGetter(FieldName) LightningBindFieldGetterAs(FieldName, #  FieldName)
#  define LightningBindFieldSetter(FieldName) LightningBindFieldSetterAs(FieldName, #  FieldName)
#  define LightningBindFieldProperty(FieldName) LightningBindFieldPropertyAs(FieldName, #  FieldName)
#  define LightningBindFieldGetterProperty(FieldName) LightningBindFieldGetterPropertyAs(FieldName, #  FieldName)
#  define LightningBindFieldSetterProperty(FieldName) LightningBindFieldSetterPropertyAs(FieldName, #  FieldName)
#  define LightningBindGetter(PropertyName) LightningBindGetterAs(PropertyName, #  PropertyName)
#  define LightningBindSetter(PropertyName) LightningBindSetterAs(PropertyName, #  PropertyName)
#  define LightningBindGetterSetter(PropertyName) LightningBindGetterSetterAs(PropertyName, #  PropertyName)
#  define LightningBindGetterProperty(PropertyName) LightningBindGetterPropertyAs(PropertyName, #  PropertyName)
#  define LightningBindSetterProperty(PropertyName) LightningBindSetterPropertyAs(PropertyName, #  PropertyName)
#  define LightningBindGetterSetterProperty(PropertyName) LightningBindGetterSetterPropertyAs(PropertyName, #  PropertyName)
#  define LightningBindCustomGetter(PropertyName) LightningBindCustomGetterAs(PropertyName, #  PropertyName)
#  define LightningBindCustomSetter(PropertyName) LightningBindCustomSetterAs(PropertyName, #  PropertyName)
#  define LightningBindCustomGetterSetter(PropertyName) LightningBindCustomGetterSetterAs(PropertyName, #  PropertyName)
#  define LightningBindCustomGetterProperty(PropertyName) LightningBindCustomGetterPropertyAs(PropertyName, #  PropertyName)
#  define LightningBindCustomSetterProperty(PropertyName) LightningBindCustomSetterPropertyAs(PropertyName, #  PropertyName)
#  define LightningBindCustomGetterSetterProperty(PropertyName)                                                            \
    LightningBindCustomGetterSetterPropertyAs(PropertyName, #PropertyName)
#  define LightningBindEnumValue(EnumValue) LightningBindEnumValueAs(EnumValue, #  EnumValue)

// Overload resolution helper macros
#  define LightningStaticOverload(ReturnType, ...) (ReturnType(*)(__VA_ARGS__))
#  define LightningInstanceOverload(ReturnType, ...) (ReturnType(LightningSelf::*)(__VA_ARGS__))
#  define LightningConstInstanceOverload(ReturnType, ...) (ReturnType(LightningSelf::*)(__VA_ARGS__) const)

// This macro sets up all the special attributes to make a C++ value into a
// Lightning enumeration
#  define LightningBindEnum(SpecialTypeEnum) LightningFullBindEnum(builder, type, SpecialTypeEnum)
} // namespace Lightning
#endif
