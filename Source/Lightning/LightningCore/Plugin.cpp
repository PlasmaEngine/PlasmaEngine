// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

// Set this to true this to get asserts for plugin types and functions not
// linking
static const bool LightningDebugPluginLinking = false;

namespace Lightning
{
namespace Events
{
LightningDefineEvent(PreBuild);
}

LightningDefineType(BuildEvent, builder, type)
{
}

LightningDefineType(PluginEvent, builder, type)
{
}

BuildEvent::BuildEvent() : Dependencies(nullptr)
{
}

LibraryRef BuildEvent::FindLibrary(StringParam name)
{
  LightningForEach (LibraryRef library, *this->Dependencies)
  {
    if (library->Name == name)
      return library;
  }

  return nullptr;
}

PlasmaShared EventHandler PluginEvent::GlobalEvents;

PluginEvent::PluginEvent() :
    mCString(nullptr),
    mInteger(0),
    mFloat(0),
    mDouble(0),
    mBool(false),
    mPointer(nullptr),
    mHandled(false)
{
}

Plugin::Plugin() : UserData(nullptr), Initialized(false), SharedLibrary(nullptr)
{
}

Plugin::~Plugin()
{
}

void Plugin::PreBuild(BuildEvent* event)
{
}

void Plugin::Initialize()
{
}

void Plugin::Uninitialize()
{
}

LibraryRef Plugin::LoadFromFile(Status& status, Module& dependencies, StringParam filePath, void* userData)
{
  // In order to not lock the library and support dynamic reloading, we make a
  // copy of any plugin files Ideally we want to load the same libraries and not
  // duplicate code loading, therefore we use the hash of the library to
  // uniquely identify it
  File file;
  file.Open(filePath, Plasma::FileMode::Read, Plasma::FileAccessPattern::Sequential, Plasma::FileShare::Read, &status);
  if (status.Failed())
  {
    status.SetFailed("We failed to open the plugin file for Read only access "
                     "(does it exist or is there permission?)");
    return nullptr;
  }

  // If the file is empty, then skip it
  if (file.CurrentFileSize() == 0)
  {
    status.SetFailed("The plugin file was empty");
    status.Context = StatusContextEmpty;
    return nullptr;
  }

  // Get the hash of the shared library and then close the file
  String sha1Hash = Sha1Builder::GetHashStringFromFile(file);
  file.Close();

  // Copy the library to a new temporary location
  StringRange pluginName = Plasma::FilePath::GetFileNameWithoutExtension(filePath);
  String fileName = BuildString(pluginName, ".", sha1Hash, ".lightningPlugin");
  String pluginLocation = Plasma::FilePath::Combine(Plasma::GetTemporaryDirectory(), fileName);

  // Only copy if the file doesn't already exist
  if (Plasma::FileExists(pluginLocation) == false)
  {
    // If we fail to copy the file, then just load the plugin directly...
    if (Plasma::CopyFile(pluginLocation, filePath) == false)
      pluginLocation = filePath;
  }

  // Attempt to load the plugin file
  UniquePointer<ExternalLibrary> lib = new ExternalLibrary();
  lib->Load(status, pluginLocation.c_str());
  if (status.Failed())
    return nullptr;

  // If we failed to load the library, then early out
  if (lib->IsValid() == false)
  {
    status.SetFailed("The plugin dynamic/shared library was not a valid "
                     "library and could not be loaded");
    status.Context = StatusContextNotValid;
    return nullptr;
  }

  // Look for the create plugin functionality, early out if we don't find it
  CreateLightningPluginFn createPlugin = (CreateLightningPluginFn)lib->GetFunctionByName("CreateLightningPlugin");
  if (createPlugin == nullptr)
  {
    status.SetFailed("The 'CreateLightningPlugin' function was not exported within "
                     "the dll (did you use the PlasmaExport macro?)");
    status.Context = StatusContextNoCreateLightningPlugin;
    return nullptr;
  }

  // Finally, attempt to create a plugin (the user should return us a plugin at
  // this point)
  Plugin* plugin = createPlugin();
  if (plugin == nullptr)
  {
    status.SetFailed("We found the 'CreateLightningPlugin' function and called it, "
                     "but it returned null so no plugin was created");
    status.Context = StatusContextNullCreateLightningPlugin;
    return nullptr;
  }

  // We successfully loaded the plugin
  plugin->UserData = userData;
  plugin->SharedLibrary = lib.Release();

  BuildEvent buildEvent;
  buildEvent.Dependencies = &dependencies;
  plugin->PreBuild(&buildEvent);

  LibraryRef pluginLibrary = plugin->GetLibrary();
  pluginLibrary->Plugin = plugin;
  return pluginLibrary;
}

void Plugin::LoadFromDirectory(
    Status& status, Module& dependencies, Array<LibraryRef>& pluginsOut, StringParam directory, void* userData)
{
  // Walk through all the files in the directory looking for anything ending
  // with .lightningPlugin
  static const String PluginExtension("lightningPlugin");

  Plasma::FileRange range(directory);
  while (range.Empty() == false)
  {
    // If this file has the .lightningPlugin extension
    Plasma::FileEntry fileEntry = range.FrontEntry();
    String filePath = fileEntry.GetFullPath();
    if (Plasma::FilePath::GetExtension(fileEntry.mFileName) == PluginExtension)
    {
      // Attempt to load the plugin (this may fail!)
      LibraryRef pluginLibrary = LoadFromFile(status, dependencies, filePath, userData);

      // If we successfully created a plugin, then output it
      if (pluginLibrary != nullptr)
        pluginsOut.PushBack(pluginLibrary);
    }
    range.PopFront();
  }
}

void Plugin::InitializeSafe()
{
  if (this->Initialized)
    return;

  this->Initialize();
  this->Initialized = true;
}

void Plugin::UninitializeSafe()
{
  if (!this->Initialized)
    return;

  this->Uninitialize();
  this->Initialized = false;
}

bool Plugin::IsInitialized()
{
  return this->Initialized;
}

void CreateAllocatingHandle(Handle& handle, void* pointer)
{
  ExecutableState* state = ExecutableState::GetCallingState();
  handle.StoredType = state->AllocatingType;
  handle.Manager = state->PointerObjects;
  state->PointerObjects->ObjectToHandle((const byte*)pointer, handle.StoredType, handle);
}

PluginStubLibrary& PluginStubLibrary::GetInstance()
{
  static PluginStubLibrary instance;
  return instance;
}

PluginStubLibrary::PluginStubLibrary() : StaticLibrary("PluginStubLibrary")
{
}

PluginStubLibrary::~PluginStubLibrary()
{
}

bool PluginStubLibrary::CanBuildTypes()
{
  return true;
}

void NameMangler::MangleLibrary(LibraryRef library)
{
  // In order to do lookup functions from the side of the plugin
  // We need to mangle the function names so they can be looked up, akin to a
  // linker However, rather than using strings, we use 64bit ids to keep the
  // size of the library small
  for (size_t j = 0; j < library->OwnedFunctions.Size(); ++j)
  {
    Function* currentFunction = library->OwnedFunctions[j];
    Function*& function = this->HashToFunction[currentFunction->Hash];
    ErrorIf(function != nullptr,
            "Two functions hashed to the same value (for mangling and linking, "
            "hashes need to be unique)");
    function = currentFunction;
  }
}

Function* NameMangler::FindFunction(GuidType functionHash, const char* functionName, StringParam typeName)
{
  Function* function = this->HashToFunction.FindValue(functionHash, nullptr);
  ErrorIf(LightningDebugPluginLinking && function == nullptr,
          "Unable to find function/overload %s on type %s (with id %llu)",
          functionName,
          typeName.c_str(),
          functionHash);
  return function;
}

GuidType NameMangler::GetDelegateTypeId(DelegateType* type)
{
  // To keep code generation sizes small, we generate one binding function
  // for all function calls of the same signature
  // To reference that function, we generate an id
  // (typically the function will be named _id, eg _61345)
  GuidType& id = this->DelegateTypeToId[type];

  if (id == 0)
    id = (int)this->DelegateTypeToId.Size();
  return id;
}

BoundType* FindLibraryType(LibraryRef library, const char* name)
{
  BoundType* type = library->BoundTypes.FindValue(name, nullptr);
  ErrorIf(LightningDebugPluginLinking && type == nullptr, "Unable to find the %s type", name);
  return type;
}

NativeName::NativeName()
{
}

NativeName::NativeName(StringParam className, StringParam parameterName, StringParam returnName) :
    Class(className),
    Parameter(parameterName),
    Return(returnName)
{
}

NativeStubCode::NativeStubCode() : Libraries(nullptr)
{
  // Because some of the types that we have bound already exist within the Plasma
  // or Lightning namespace then when we generate code for them, we want to redirect
  // them to use the special type names specified here
  this->TypeToCppName.Insert(LightningTypeId(Any), NativeName("Any", "const Lightning::Any&", "Lightning::Any"));
  this->TypeToCppName.Insert(LightningTypeId(Handle), NativeName("Handle", "const Lightning::Handle&", "Lightning::Handle"));
  this->TypeToCppName.Insert(LightningTypeId(Delegate),
                             NativeName("Delegate", "const Lightning::Delegate&", "Lightning::Delegate"));

  this->TypeToCppName.Insert(LightningTypeId(Member),
                             NativeName("Member", "Lightning::Member*", "Lightning::HandleOf<Lightning::Member>"));
  this->TypeToCppName.Insert(LightningTypeId(Property),
                             NativeName("Property", "Lightning::Property*", "Lightning::HandleOf<Lightning::Property>"));
  this->TypeToCppName.Insert(
      LightningTypeId(GetterSetter),
      NativeName("GetterSetter", "Lightning::GetterSetter*", "Lightning::HandleOf<Lightning::GetterSetter>"));
  this->TypeToCppName.Insert(LightningTypeId(Field), NativeName("Field", "Lightning::Field*", "Lightning::HandleOf<Lightning::Field>"));
  this->TypeToCppName.Insert(LightningTypeId(Function),
                             NativeName("Function", "Lightning::Function*", "Lightning::HandleOf<Lightning::Function>"));

  this->TypeToCppName.Insert(LightningTypeId(Void), NativeName("Void", "void", "void"));

  this->TypeToCppName.Insert(LightningTypeId(Boolean), NativeName("Boolean", "bool", "bool"));
  this->TypeToCppName.Insert(LightningTypeId(Boolean2),
                             NativeName("Boolean2", "const Lightning::Boolean2&", "Lightning::Boolean2"));
  this->TypeToCppName.Insert(LightningTypeId(Boolean3),
                             NativeName("Boolean3", "const Lightning::Boolean3&", "Lightning::Boolean3"));
  this->TypeToCppName.Insert(LightningTypeId(Boolean4),
                             NativeName("Boolean4", "const Lightning::Boolean4&", "Lightning::Boolean4"));

  this->TypeToCppName.Insert(LightningTypeId(Byte), NativeName("Byte", "unsigned char", "unsigned char"));

  this->TypeToCppName.Insert(LightningTypeId(Integer), NativeName("Integer", "int", "int"));
  this->TypeToCppName.Insert(LightningTypeId(Integer2),
                             NativeName("Integer2", "const Lightning::Integer2&", "Lightning::Integer2"));
  this->TypeToCppName.Insert(LightningTypeId(Integer3),
                             NativeName("Integer3", "const Lightning::Integer3&", "Lightning::Integer3"));
  this->TypeToCppName.Insert(LightningTypeId(Integer4),
                             NativeName("Integer4", "const Lightning::Integer4&", "Lightning::Integer4"));

  this->TypeToCppName.Insert(LightningTypeId(Real), NativeName("Real", "float", "float"));
  this->TypeToCppName.Insert(LightningTypeId(Real2), NativeName("Real2", "const Lightning::Real2&", "Lightning::Real2"));
  this->TypeToCppName.Insert(LightningTypeId(Real3), NativeName("Real3", "const Lightning::Real3&", "Lightning::Real3"));
  this->TypeToCppName.Insert(LightningTypeId(Real4), NativeName("Real4", "const Lightning::Real4&", "Lightning::Real4"));

  this->TypeToCppName.Insert(LightningTypeId(Quaternion),
                             NativeName("Quaternion", "const Lightning::Quaternion&", "Lightning::Quaternion"));
  this->TypeToCppName.Insert(LightningTypeId(Real3x3), NativeName("Real3x3", "const Lightning::Real3x3&", "Lightning::Real3x3"));
  this->TypeToCppName.Insert(LightningTypeId(Real4x4), NativeName("Real4x4", "const Lightning::Real4x4&", "Lightning::Real4x4"));

  this->TypeToCppName.Insert(LightningTypeId(String), NativeName("String", "const Lightning::String&", "Lightning::String"));

  this->TypeToCppName.Insert(LightningTypeId(DoubleReal), NativeName("DoubleReal", "double", "double"));
  this->TypeToCppName.Insert(LightningTypeId(DoubleInteger), NativeName("DoubleInteger", "long long", "long long"));

  this->TypeToCppName.Insert(LightningTypeId(Members::Enum),
                             NativeName("Members::Enum", "Lightning::Members::Enum", "Lightning::Members::Enum"));
  this->TypeToCppName.Insert(LightningTypeId(FileMode::Enum),
                             NativeName("FileMode::Enum", "Lightning::FileMode::Enum", "Lightning::FileMode::Enum"));
  this->TypeToCppName.Insert(
      LightningTypeId(StreamCapabilities::Enum),
      NativeName("StreamCapabilities::Enum", "Lightning::StreamCapabilities::Enum", "Lightning::StreamCapabilities::Enum"));
  this->TypeToCppName.Insert(
      LightningTypeId(StreamOrigin::Enum),
      NativeName("StreamOrigin::Enum", "Lightning::StreamOrigin::Enum", "Lightning::StreamOrigin::Enum"));

  this->TypeToCppName.Insert(LightningTypeId(EventsClass),
                             NativeName("EventsClass", "const Lightning::EventsClass&", "Lightning::EventsClass"));
  this->TypeToCppName.Insert(LightningTypeId(FilePathClass),
                             NativeName("FilePathClass", "const Lightning::FilePathClass&", "Lightning::FilePathClass"));
  this->TypeToCppName.Insert(LightningTypeId(IStreamClass),
                             NativeName("IStreamClass", "const Lightning::IStreamClass&", "Lightning::IStreamClass"));
  this->TypeToCppName.Insert(LightningTypeId(FileStreamClass),
                             NativeName("FileStreamClass", "const Lightning::FileStreamClass&", "Lightning::FileStreamClass"));
  this->TypeToCppName.Insert(
      LightningTypeId(StringBuilderExtended),
      NativeName("StringBuilderExtended", "const Lightning::StringBuilderExtended&", "Lightning::StringBuilderExtended"));
  this->TypeToCppName.Insert(
      LightningTypeId(StringRangeExtended),
      NativeName("StringRangeExtended", "const Lightning::StringRangeExtended&", "Lightning::StringRangeExtended"));
  this->TypeToCppName.Insert(LightningTypeId(StringSplitRangeExtended),
                             NativeName("StringSplitRangeExtended",
                                        "const Lightning::StringSplitRangeExtended&",
                                        "Lightning::StringSplitRangeExtended"));

  this->TypeToCppName.Insert(LightningTypeId(ColorClass),
                             NativeName("ColorClass", "const Lightning::ColorClass&", "Lightning::ColorClass"));
  this->TypeToCppName.Insert(LightningTypeId(ColorsClass),
                             NativeName("ColorsClass", "const Lightning::ColorsClass&", "Lightning::ColorsClass"));

  this->TypeToCppName.Insert(
      LightningTypeId(ParameterArray::range),
      NativeName("ParameterArray::range", "const Lightning::ParameterArray::range&", "Lightning::ParameterArray::range"));
  this->TypeToCppName.Insert(LightningTypeId(MemberRange<Member>),
                             NativeName("MemberRange<Lightning::Member>",
                                        "const Lightning::MemberRange<Lightning::Member>&",
                                        "Lightning::MemberRange<Lightning::Member>"));
  this->TypeToCppName.Insert(LightningTypeId(MemberRange<Property>),
                             NativeName("MemberRange<Lightning::Property>",
                                        "const Lightning::MemberRange<Lightning::Property>&",
                                        "Lightning::MemberRange<Lightning::Property>"));
  this->TypeToCppName.Insert(LightningTypeId(MemberRange<GetterSetter>),
                             NativeName("MemberRange<Lightning::GetterSetter>",
                                        "const Lightning::MemberRange<Lightning::GetterSetter>&",
                                        "Lightning::MemberRange<Lightning::GetterSetter>"));
  this->TypeToCppName.Insert(LightningTypeId(MemberRange<Field>),
                             NativeName("MemberRange<Lightning::Field>",
                                        "const Lightning::MemberRange<Lightning::Field>&",
                                        "Lightning::MemberRange<Lightning::Field>"));
  this->TypeToCppName.Insert(LightningTypeId(MemberRange<Function>),
                             NativeName("MemberRange<Lightning::Function>",
                                        "const Lightning::MemberRange<Lightning::Function>&",
                                        "Lightning::MemberRange<Lightning::Function>"));
}

NativeName NativeStubCode::GetCppTypeName(Type* type)
{
  // All delegates that we take in just become a Lightning Delegate (not type
  // checked unfortunately)
  if (Type::IsDelegateType(type))
    type = LightningTypeId(Delegate);

  // If we've already generated a native name, avoid a bunch of extra string
  // allocations This also allows us to handle the built in types to Plasma/Lightning,
  // such as 'Any' (see above)
  NativeName* nativeName = this->TypeToCppName.FindPointer(type);
  if (nativeName != nullptr)
    return *nativeName;

  // Remove any invalid characters from the type name (that's what \0 means
  // here)
  String name = LibraryBuilder::FixIdentifier(type->ToString(), TokenCheck::None, '\0');

  // If the generated type is not from our currently library, then qualify it
  // with a
  if (this->LibrarySet.Contains(type->SourceLibrary) == false)
    name = BuildString(type->SourceLibrary->GetPluginNamespace(), "::", name);

  NativeName nativeNameResult;
  nativeNameResult.Class = name;

  if (Type::IsHandleType(type))
  {
    // If this is a handle type, then when we generate the stub
    // we want to accept it as a pointer when taken as a parameter
    nativeNameResult.Parameter = BuildString(name, "*");

    // In order to ensure that reference counting works properly, we also need
    // ALL returns to be of Handle type. Imagine allocating a Handle, then
    // returning a pointer... the handle would be the last reference count, and
    // would get destroyed, so the pointer would go invalid immediately
    nativeNameResult.Return = BuildString("Lightning::HandleOf<", name, ">");
  }
  else
  {
    // Otherwise this is a value type, so just always take it by value
    nativeNameResult.Parameter = BuildString("const ", name, "&");
    nativeNameResult.Return = name;
  }

  // We force every type we generate to at least inherit from a base Lightning class
  // This closes some inheritance holes by ensuring every class has a base
  static const String ReferenceBase = "Lightning::ReferenceType";
  static const String ValueBase = "Lightning::ValueType";
  if (Type::IsValueType(type))
    nativeNameResult.Base = ValueBase;
  else
    nativeNameResult.Base = ReferenceBase;

  // We also want to know if this type has a base type (easily access and
  // generate its name too)
  if (BoundType* boundType = Type::DynamicCast<BoundType*>(type))
  {
    BoundType* base = boundType->BaseType;
    if (base != nullptr)
      nativeNameResult.Base = this->GetCppTypeName(base).Class;
  }

  this->TypeToCppName.Insert(type, nativeNameResult);
  return nativeNameResult;
}

void NativeStubCode::WriteParameters(LightningCodeBuilder& builder, DelegateType* delegateType)
{
  builder.Write("(");

  size_t lastIndex = delegateType->Parameters.Size() - 1;
  for (size_t k = 0; k < delegateType->Parameters.Size(); ++k)
  {
    DelegateParameter& parameter = delegateType->Parameters[k];
    builder.Write(this->GetCppTypeName(parameter.ParameterType).Parameter);
    builder.WriteSpace();
    builder.Write(parameter.GetNameOrGenerate());

    if (k != lastIndex)
      builder.Write(", ");
  }

  builder.Write(")");
}

void NativeStubCode::WriteDescription(LightningCodeBuilder& builder, ReflectionObject* object)
{
  if (object->Description.Empty() == false)
  {
    builder.WriteSingleLineComment(object->Description);
    builder.WriteLineIndented();
  }
}

String GetCoreNamespace()
{
  // Define types if this is not the Core library (which has already defined its
  // types)
  static const String CoreNamespace = Core::GetInstance().GetLibrary()->GetPluginNamespace();
  return CoreNamespace;
}

String NativeStubCode::GenerateHpp()
{
  String nameDefine = this->Filename.ToUpper();

  LightningCodeBuilder builder;

  if (this->HppHeader.Empty() == false)
  {
    builder.Write(this->HppHeader);
    builder.WriteLineIndented();
    builder.WriteLineIndented();
  }

  builder.WriteLineIndented("#pragma once");
  builder.Write("#ifndef ");
  builder.Write(nameDefine);
  builder.WriteLineIndented("_HPP");
  builder.Write("#define ");
  builder.Write(nameDefine);
  builder.WriteLineIndented("_HPP");
  builder.WriteLineIndented();
  builder.WriteLineIndented("#include \"Lightning.hpp\"");
  builder.WriteLineIndented();

  if (this->HppMiddle.Empty() == false)
  {
    builder.WriteLineIndented();
    builder.Write(this->HppMiddle);
    builder.WriteLineIndented();
    builder.WriteLineIndented();
  }

  if (this->Namespace != GetCoreNamespace())
  {
    builder.Write("namespace ");
    builder.Write(this->Namespace);
    builder.BeginScope(ScopeType::Block);
    builder.WriteLineIndented();

    // Forward declarations
    LightningForEach (BoundType* type, this->TypesInDependencyOrder)
    {
      String typeName = this->GetCppTypeName(type).Class;

      builder.Write("class ");
      builder.Write(typeName);
      builder.WriteLineIndented(";");
    }

    builder.WriteLineIndented();

    // Outputting all the declarations for all the types (in order)
    LightningForEach (BoundType* type, this->TypesInDependencyOrder)
    {
      this->WriteDescription(builder, type);

      NativeName name = this->GetCppTypeName(type);
      String typeName = name.Class;
      String baseName = name.Base;

      BoundType* base = type->BaseType;
      if (base != nullptr)
        baseName = this->GetCppTypeName(base).Class;

      builder.Write("class ");
      builder.Write(typeName);
      builder.Write(" : public ");
      builder.Write(baseName);

      builder.BeginScope(ScopeType::Class);

      // We generally put 'public' on the same line as the scope
      --builder.Indentation;
      builder.WriteLineIndented();
      builder.Write("public:");
      ++builder.Indentation;
      builder.WriteLineIndented();

      // Add typedefs for LightningSelf and LightningBase
      builder.Write("typedef ");
      builder.Write(typeName);
      builder.Write(" LightningSelf;");
      builder.WriteLineIndented();
      builder.Write("typedef ");
      builder.Write(baseName);
      builder.Write(" LightningBase;");
      builder.WriteLineIndented();
      builder.Write("Lightning::BoundType* LightningGetDerivedType() const { return "
                    "LightningTypeId(LightningSelf)->GetBindingVirtualTypeFromInstance(this); }");
      builder.WriteLineIndented();
      builder.WriteLineIndented();

      String* customDefine = this->CustomClassHeaderDefines.FindPointer(type);
      if (customDefine != nullptr && customDefine->Empty() == false)
        builder.WriteLineIndented(*customDefine);

      // All function declarations (static and instance)
      for (size_t j = 0; j < type->AllFunctions.Size(); ++j)
      {
        Function* function = type->AllFunctions[j];

        String functionName = LibraryBuilder::FixIdentifier(function->Name, TokenCheck::RemoveOuterBrackets);

        this->WriteDescription(builder, function);

        if (function->IsStatic)
          builder.Write("static ");

        DelegateType* delegateType = function->FunctionType;
        Type* returnType = delegateType->Return;
        NativeName returnTypeName = this->GetCppTypeName(returnType);
        builder.Write(returnTypeName.Return);

        builder.WriteSpace();
        builder.Write(functionName);
        this->WriteParameters(builder, delegateType);
        builder.Write(';');
        builder.WriteLineIndented();
        builder.WriteLineIndented();
      }

      // An enums base class already has an 'Integer' for the value
      if (!Type::IsEnumOrFlagsType(type))
      {
        size_t size = type->GetAllocatedSize();
        if (type->BaseType != nullptr)
          size -= type->BaseType->GetAllocatedSize();

        if (size != 0)
        {
          builder.Write("unsigned char mValue[");
          builder.Write((Integer)size);
          builder.WriteLineIndented("];");
        }
      }

      bool isValueType = Type::IsValueType(type);
      if (isValueType == false)
      {
        // Protected members (such as constructors)
        --builder.Indentation;
        builder.WriteLineIndented();
        builder.Write("protected:");
        ++builder.Indentation;
        builder.WriteLineIndented();
      }

      // Inheritable constructor declarations (not allocation!)
      if (type->Sealed == false || isValueType)
      {
        // If we don't have a default constructor, implicitly add one
        if (isValueType && type->GetDefaultConstructor(false) == nullptr)
        {
          // Make a default constructor that plasmaes our type out
          builder.Write(typeName);
          builder.Write("();");
          builder.WriteLineIndented();
          builder.WriteLineIndented();
        }

        for (size_t j = 0; j < type->Constructors.Size(); ++j)
        {
          Function* constructor = type->Constructors[j];
          this->WriteDescription(builder, constructor);

          builder.Write(typeName);
          DelegateType* delegateType = constructor->FunctionType;
          this->WriteParameters(builder, delegateType);
          builder.Write(';');
          builder.WriteLineIndented();
          builder.WriteLineIndented();
        }
      }

      if (Type::IsValueType(type) == false)
      {
        // Make an empty 'base constructor'
        builder.Write(typeName);
        builder.Write("(Lightning::NoType none) : ");
        builder.Write(baseName);
        builder.Write("(none) {}");
        builder.WriteLineIndented();

        // Make this type not copyable
        builder.Write("LightningNoCopy(");
        builder.Write(typeName);
        builder.Write(");");
        builder.WriteLineIndented();
      }

      builder.EndScope();
      builder.WriteLineIndented(";");
      builder.WriteLineIndented();
    }

    builder.EndScope();
    builder.WriteLineIndented();
    builder.WriteLineIndented();
  }

  // Declare HookUpLibrary function
  builder.Write("bool HookUp");
  builder.Write(this->Filename);
  builder.WriteLineIndented("(Lightning::BuildEvent* event);");
  builder.WriteLineIndented();

  builder.WriteLineIndented("#endif");

  if (this->HppFooter.Empty() == false)
  {
    builder.WriteLineIndented();
    builder.Write(this->HppFooter);
    builder.WriteLineIndented();
  }

  String result = builder.ToString();
  return result;
}

String NativeStubCode::GenerateCpp()
{
  LightningCodeBuilder builder;

  if (this->CppHeader.Empty() == false)
  {
    builder.Write(this->CppHeader);
    builder.WriteLineIndented();
    builder.WriteLineIndented();
  }

  if (this->PrecompiledHeader.Empty())
  {
    builder.WriteLineIndented("#define PlasmaImportDll");
    builder.Write("#include \"");
    builder.Write(this->Namespace);
    builder.WriteLineIndented(".hpp\"");
  }
  else
  {
    builder.Write("#include ");
    builder.WriteLineIndented(this->PrecompiledHeader);
  }
  builder.WriteLineIndented();

  if (this->CppMiddle.Empty() == false)
  {
    builder.WriteLineIndented();
    builder.Write(this->CppMiddle);
    builder.WriteLineIndented();
    builder.WriteLineIndented();
  }

  if (this->Namespace != GetCoreNamespace())
  {
    builder.Write("namespace ");
    builder.Write(this->Namespace);
    builder.BeginScope(ScopeType::Block);
    builder.WriteLineIndented();

    // Unique function calling signatures (independent of actual method names)
    HashSet<DelegateType*, DelegateTypePolicy> delegates;
    LightningForEach (const LibraryRef& library, *this->Libraries)
    {
      LightningForEach (Function* function, library->OwnedFunctions)
      {
        DelegateType* delegateType = function->FunctionType;
        if (delegates.Contains(delegateType))
          continue;

        delegates.Insert(delegateType);

        builder.WriteLineIndented("//"
                                  "********************************************"
                                  "*******************************");

        Type* returnType = delegateType->Return;
        NativeName returnTypeName = this->GetCppTypeName(returnType);

        bool isReturnVoid = (delegateType->Return == LightningTypeId(void));
        String defaultReturn;
        if (isReturnVoid == false)
          defaultReturn = String::Format(" %s()", returnTypeName.Return.c_str());

        builder.Write(returnTypeName.Return);
        builder.WriteSpace();

        GuidType cachedDelegateId = this->Mangler.GetDelegateTypeId(delegateType);
        builder.Write('_');
        builder.Write(cachedDelegateId);

        builder.Write("(Lightning::Handle* thisHandle, Lightning::Function* function");

        for (size_t k = 0; k < delegateType->Parameters.Size(); ++k)
        {
          builder.Write(", ");
          DelegateParameter& parameter = delegateType->Parameters[k];
          builder.Write(this->GetCppTypeName(parameter.ParameterType).Parameter);
          builder.WriteSpace();
          builder.Write(parameter.GetNameOrGenerate());
        }

        builder.Write(")");
        builder.BeginScope(ScopeType::Function);
        builder.WriteLineIndented();

        builder.Write("ReturnIf(function == nullptr,");
        builder.Write(defaultReturn);
        builder.WriteLineIndented(", \"The function does not exist (it may have been removed)\");");

        builder.WriteLineIndented("Lightning::ExecutableState* __state = "
                                  "Lightning::ExecutableState::GetCallingState();");
        builder.Write("ReturnIf(__state == nullptr,");
        builder.Write(defaultReturn);
        builder.WriteLineIndented(", \"You can only invoke this function when "
                                  "your code is called from Lightning\");");
        builder.WriteLineIndented();

        builder.WriteLineIndented("Lightning::Call __call(function, __state);");

        builder.WriteLineIndented("if (thisHandle != nullptr)");
        builder.WriteIndent();
        builder.WriteLineIndented("__call.Set<Lightning::Handle>(Lightning::Call::This, *thisHandle);");

        for (size_t k = 0; k < delegateType->Parameters.Size(); ++k)
        {
          DelegateParameter& parameter = delegateType->Parameters[k];

          builder.Write("__call.Set<");
          builder.Write(this->GetCppTypeName(parameter.ParameterType).Parameter);
          builder.Write(">(");
          builder.Write((Integer)k);
          builder.Write(", ");
          builder.Write(parameter.GetNameOrGenerate());
          builder.WriteLineIndented(");");
        }
        builder.WriteLineIndented();

        if (isReturnVoid)
        {
          builder.WriteLineIndented("__call.Invoke();");
          builder.WriteLineIndented();
          builder.WriteLineIndented("return;");
        }
        else
        {
          builder.WriteLineIndented("if (__call.Invoke() == false)");
          builder.WriteIndent();
          builder.Write("return");
          builder.Write(defaultReturn);
          builder.WriteLineIndented(";");
          builder.WriteLineIndented();

          builder.Write("return __call.Get<");
          builder.Write(returnTypeName.Return);
          builder.WriteLineIndented(" >(Lightning::Call::Return);");
        }

        builder.EndScope();
        builder.WriteLineIndented();
        builder.WriteLineIndented();
      }
    }

    // Emit all types and all functions for those types
    LightningForEach (BoundType* type, this->TypesInDependencyOrder)
    {
      NativeName name = this->GetCppTypeName(type);
      String typeName = name.Class;
      String baseName = name.Base;

      builder.WriteLineIndented("//"
                                "**********************************************"
                                "*****************************");
      builder.Write("static Lightning::BoundType* ");
      String cachedTypeName = String::Format("%s_Type", typeName.c_str());
      builder.Write(cachedTypeName);
      builder.WriteLineIndented(" = nullptr;");

      // Inheritable constructors (not allocation!)
      bool isValueType = Type::IsValueType(type);
      if (type->Sealed == false || isValueType)
      {
        // If we don't have a default constructor, implicitly add one
        if (isValueType && type->GetDefaultConstructor(false) == nullptr)
        {
          // Make a default constructor that plasmaes our type out
          builder.Write(typeName);
          builder.Write("::");
          builder.Write(typeName);
          builder.Write("()");

          builder.BeginScope(ScopeType::Function);
          builder.WriteLineIndented();

          builder.WriteLineIndented("memset(this, 0, sizeof(*this));");

          builder.EndScope();
          builder.WriteLineIndented();
          builder.WriteLineIndented();
        }

        for (size_t j = 0; j < type->Constructors.Size(); ++j)
        {
          Function* constructor = type->Constructors[j];
          builder.WriteLineIndented("//"
                                    "******************************************"
                                    "*********************************");
          builder.Write("static Lightning::Function* _");
          builder.Write(constructor->Hash);
          builder.WriteLineIndented(" = nullptr;");

          builder.Write(typeName);
          builder.Write("::");
          builder.Write(typeName);

          DelegateType* delegateType = constructor->FunctionType;
          this->WriteParameters(builder, delegateType);

          if (isValueType == false)
          {
            builder.Write(" : ");
            builder.Write(baseName);
            builder.Write("(Lightning::NoType())");
          }

          builder.BeginScope(ScopeType::Function);
          builder.WriteLineIndented();

          builder.WriteLineIndented("Lightning::Handle thisHandle;");
          builder.WriteLineIndented("Lightning::CreateAllocatingHandle(thisHandle, this);");

          GuidType cachedDelegateId = this->Mangler.GetDelegateTypeId(delegateType);
          builder.Write("_");
          builder.Write(cachedDelegateId);
          builder.Write("(&thisHandle, _");
          builder.Write(constructor->Hash);

          for (size_t k = 0; k < delegateType->Parameters.Size(); ++k)
          {
            builder.Write(", ");
            DelegateParameter& parameter = delegateType->Parameters[k];
            builder.Write(parameter.GetNameOrGenerate());
          }
          builder.WriteLineIndented(");");

          builder.EndScope();
          builder.WriteLineIndented();
          builder.WriteLineIndented();
        }
      }

      // All function definitions (static and instance)
      for (size_t j = 0; j < type->AllFunctions.Size(); ++j)
      {
        Function* function = type->AllFunctions[j];

        String functionName = function->Name;
        if (function->OwningProperty != nullptr)
        {
          StringIterator begin = functionName.Begin();
          StringIterator end = functionName.End();
          ++begin;
          --end;
          functionName = functionName.SubString(begin, end);
        }
        DelegateType* delegateType = function->FunctionType;

        builder.WriteLineIndented("//"
                                  "********************************************"
                                  "*******************************");
        builder.Write("static Lightning::Function* _");
        builder.Write(function->Hash);
        builder.WriteLineIndented(" = nullptr;");

        Type* returnType = delegateType->Return;
        NativeName returnTypeName = this->GetCppTypeName(returnType);

        builder.Write(returnTypeName.Return);
        builder.WriteSpace();
        builder.Write(typeName);
        builder.Write("::");
        builder.Write(functionName);

        this->WriteParameters(builder, delegateType);

        builder.BeginScope(ScopeType::Function);
        builder.WriteLineIndented();

        GuidType cachedDelegateId = this->Mangler.GetDelegateTypeId(delegateType);

        if (function->This != nullptr)
        {
          builder.Write("Lightning::HandleOf<");
          builder.Write(typeName);
          builder.Write("> thisHandle(this);");
          builder.WriteLineIndented();
        }

        builder.Write("return _");
        builder.Write(cachedDelegateId);
        builder.Write("(");

        if (function->This != nullptr)
          builder.Write("&thisHandle, ");
        else
          builder.Write("nullptr, ");

        builder.Write('_');
        builder.Write(function->Hash);

        for (size_t k = 0; k < delegateType->Parameters.Size(); ++k)
        {
          builder.Write(", ");
          DelegateParameter& parameter = delegateType->Parameters[k];
          builder.Write(parameter.GetNameOrGenerate());
        }
        builder.WriteLineIndented(");");

        builder.EndScope();
        builder.WriteLineIndented();
        builder.WriteLineIndented();
      }
    }
    builder.EndScope();
    builder.WriteLineIndented();
    builder.WriteLineIndented();
  }

  builder.WriteLineIndented("#if defined(PlasmaCompilerMsvc)");
  builder.WriteLineIndented("#pragma optimize(\"\", off)");
  builder.WriteLineIndented("#endif");

  // Define HookUpLibrary function
  builder.WriteLineIndented();
  builder.Write("bool HookUp");
  builder.Write(this->Filename);
  builder.Write("(Lightning::BuildEvent* event)");
  builder.BeginScope(ScopeType::Function);
  builder.WriteLineIndented();

  builder.Write("Lightning::NativeBindingList::SetBuildingLibraryForThisThread(true);");
  builder.WriteLineIndented();

  builder.WriteLineIndented("Lightning::NameMangler mangler;");

  LightningForEach (const LibraryRef& library, *this->Libraries)
  {
    builder.BeginScope(ScopeType::Block);
    builder.WriteLineIndented();

    builder.Write("const char* libraryName = \"");
    builder.Write(library->Name);
    builder.WriteLineIndented("\";");

    builder.WriteLineIndented("Lightning::BoundType* type = nullptr;");

    builder.WriteLineIndented("Lightning::LibraryRef library = event->FindLibrary(libraryName);");
    builder.WriteLineIndented("ReturnIf(library == nullptr, false, \"Unable to find the library %s "
                              "in the list of dependencies\", libraryName);");

    builder.WriteLineIndented("mangler.MangleLibrary(library);");

    // Loop through all types created in this library
    LightningForEach (Type* type, library->OwnedTypes)
    {
      // Only consider bound types...
      BoundType* boundType = Type::DynamicCast<BoundType*>(type);
      if (boundType == nullptr)
        continue;

      if (this->Namespace == GetCoreNamespace())
      {
        // Ignore non-native bound types in the Core library (these types don't
        // have a corresponding C++ type)
        if (boundType->Native == false)
          continue;
      }

      bool isValueType = Type::IsValueType(type);
      String typeName = this->GetCppTypeName(type).Class;

      String qualifiedTypeName = String::Format("%s::%s", this->Namespace.c_str(), typeName.c_str());

      bool shouldPatchType = this->Namespace == GetCoreNamespace() && boundType->HasNativeBinding() ||
                             this->Namespace != GetCoreNamespace();

      if (shouldPatchType)
      {
        builder.Write("type = Lightning::PatchLibraryType");
        builder.Write("< ");
        builder.Write(qualifiedTypeName);
        builder.Write(" >");
        builder.Write("(library, \"");
        builder.Write(boundType->Name);
        builder.WriteLineIndented("\");");
      }

      // Assign defined functions if this is not the Core library
      if (this->Namespace != GetCoreNamespace())
      {
        builder.Write("if (type != nullptr)");
        builder.BeginScope(ScopeType::Block);
        builder.WriteLineIndented();

        FunctionArray functions = boundType->AllFunctions;

        if (boundType->Sealed == false || isValueType)
          functions.Append(boundType->Constructors.All());

        for (size_t j = 0; j < functions.Size(); ++j)
        {
          Function* function = functions[j];

          builder.Write(this->Namespace);
          builder.Write("::_");
          builder.Write(function->Hash);
          builder.Write(" = mangler.FindFunction(");
          builder.Write(function->Hash);
          builder.Write(", \"");
          builder.Write(function->Name);
          builder.WriteLineIndented("\", type->Name);");
        }

        builder.EndScope();
        builder.WriteLineIndented();
        builder.WriteLineIndented();
      }
    }

    builder.EndScope();
    builder.WriteLineIndented();
  }

  builder.WriteLineIndented();
  builder.Write("Lightning::NativeBindingList::SetBuildingLibraryForThisThread(false);");
  builder.WriteLineIndented();

  builder.WriteLineIndented();
  builder.WriteLineIndented("return true;");
  builder.EndScope();
  builder.WriteLineIndented();

  if (this->CppFooter.Empty() == false)
  {
    builder.WriteLineIndented();
    builder.Write(this->CppFooter);
    builder.WriteLineIndented();
  }

  String result = builder.ToString();
  return result;
}

String NativeStubCode::Generate(const LibraryArray& libraries)
{
  this->Libraries = &libraries;

  LightningForEach (const LibraryRef& library, libraries)
  {
    if (this->Namespace.Empty())
    {
      this->Namespace = library->GetPluginNamespace();
      this->Filename = this->Namespace;
      continue;
    }

    ErrorIf(library->GetPluginNamespace() != this->Namespace,
            "All the libraries in the array must have the same namespace");
  }

  if (this->Filename == GetCoreNamespace())
  {
    this->Filename = "Core";
  }

  ComputeTypesInDependencyOrder(libraries, this->LibrarySet, this->TypesInDependencyOrder);

  this->Hpp = this->GenerateHpp();
  this->Cpp = this->GenerateCpp();

  return this->Namespace;
}

String NativeStubCode::Generate(LibraryParam library)
{
  return this->Generate(LibraryArray(PlasmaInit, library));
}
} // namespace Lightning
