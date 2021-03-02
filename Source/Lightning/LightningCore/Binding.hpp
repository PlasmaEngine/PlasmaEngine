// MIT Licensed (see LICENSE.md).

#pragma once
#ifndef LIGHTNING_BINDING_HPP
#  define LIGHTNING_BINDING_HPP

namespace Lightning
{
/****************************** CONTAINER RANGE
 * ********************************/
// We bind container types as a ranges to Lightning (can use foreach)
// Note: This won't work with Pair ranges unless the pair is also explicitly
// bound as an external type (before the definition is seen here)
// The range makes a copy of the container for safety purposes
template <typename ContainerType, typename RangeType, typename ElementType>
class PlasmaSharedTemplate ContainerRange
{
public:
  // The container that we're iterating through (a copy)
  ContainerType Container;
};

/*************************** VIRTUAL DERIVED TYPE
 * ******************************/

// Derive from object
class PlasmaShared ILightningObject
{
public:
  // Declare a virtual destructor
  virtual ~ILightningObject()
  {
  }

  // Get the most derived type from an instance of the class (probably a base
  // pointer)
  virtual BoundType* LightningGetDerivedType() const = 0;
};

// This type is used in the case where we have no base type (base type gets
// defined as this)
class PlasmaShared NoType : public ILightningObject
{
public:
  virtual BoundType* LightningGetDerivedType() const;
};

/********************************** BINDING
 * ************************************/

// Helper macros for accessing these types
#  define LightningStaticType(Type) LS::TypeBinding::StaticTypeId<Type>
#  define LightningTypeId(Type) (LightningStaticType(Type)::GetType())
#  define LightningBindingType(Type) typename LightningStaticType(Type)::BindingType

// This class keeps track of all BoundTypes that are natively bound
// (any deleted type that is native must be removed from here)
class PlasmaShared NativeBindingList
{
public:
  NativeBindingList();
  static NativeBindingList& GetInstance();

  // Call this when you want to verify that all native types (to this point)
  // have been properly registered. Note that this MUST be called at a time when
  // no other threads are initializing libraries, and never in the middle of a
  // library initialization only after all libraries 'CreateLibrary' calls has
  // been made
  static void ValidateTypes();

  // Checks if we are currently building a library
  static bool IsBuildingLibrary();
  static void SetBuildingLibraryForThisThread(bool value);

  HashSet<BoundType*> AllNativeBoundTypes;
  Plasma::ThreadLock Lock;
};

// When we need a handle that we can use within our automatic binding
// that would know and limit the types it could store via the compile time type
// (T)
template <typename T>
class PlasmaSharedTemplate HandleOf : public Handle
{
public:
  typedef T HandleOfType;

  HandleOf();
  HandleOf(const Handle& other);
  HandleOf(Handle&& other);
  HandleOf(const T& value);
  HandleOf(const T* value);

  BoundType* GetCompileType() const;
  operator T*() const;
  operator T&() const;
  T* operator->() const;

  T* operator*() const;

  void SafeDestroy();

  template <typename ComponentType>
  ComponentType* Has()
  {
    if (T* instance = *this)
      return instance->template Has<ComponentType>();
    return nullptr;
  }

  T* Dereference() const;
};

// All things relevant to binding types
class PlasmaShared TypeBinding
{
public:
  PlasmaDeclareHasMemberTrait(CanGetDerivedType, LightningGetDerivedType);

  // The macro LightningVirtualTypeId will create this template using the above
  // SFINAE template (checking for LightningGetDerivedType) If the method does not
  // exist on type T, then this one will get called (just resulting in the
  // static typeid)
  template <typename T, bool HasLightningGetDerivedType>
  class DiscoverDerivedType
  {
  public:
    static BoundType* Get(const T* object)
    {
      return LightningTypeId(T);
    }
  };

  // If the type T has LightningGetDerivedType when invoking the macro
  // LightningVirtualTypeId, this one will get chosen, and will actually invoke
  // LightningGetDerivedType
  template <typename T>
  class DiscoverDerivedType<T, true>
  {
  public:
    static BoundType* Get(const T* object)
    {
      if (object != nullptr)
        return object->LightningGetDerivedType();
      return LightningTypeId(T);
    }
  };

// Get the type of the pointer (using virtual behavior if possible)
#  define LightningVirtualTypeId(Pointer)                                                                                  \
    LS::TypeBinding::DiscoverDerivedType<                                                                              \
        LightningStrip(decltype(Pointer)),                                                                                 \
        LS::TypeBinding::CanGetDerivedType<LightningStrip(decltype(Pointer))>::value>::Get(Pointer)

  template <typename T>
  class StripQualifiers
  {
  public:
    typedef T Type;
  };

  template <typename T>
  class StripQualifiers<T*>
  {
  public:
    // Use template recursion to strip all qualifiers
    typedef typename StripQualifiers<T>::Type Type;
  };

  template <typename T>
  class StripQualifiers<T&>
  {
  public:
    // Use template recursion to strip all qualifiers
    typedef typename StripQualifiers<T>::Type Type;
  };

  template <typename T>
  class StripQualifiers<const T>
  {
  public:
    // Use template recursion to strip all qualifiers
    typedef typename StripQualifiers<T>::Type Type;
  };

  template <typename T>
  class StripQualifiers<volatile T>
  {
  public:
    // Use template recursion to strip all qualifiers
    typedef typename StripQualifiers<T>::Type Type;
  };

// Strip all const, pointer, reference, and volatile qualifiers from a type to
// get its core
#  define LightningStrip(T) typename LS::TypeBinding::StripQualifiers<T>::Type

  // Handles:
  //  int
  //  const int
  //  int&
  //  const int&
  template <typename T>
  static T* InternalToPointer(const T& value)
  {
    return (T*)&value;
  }

  // Handles:
  //  int*
  //  const int*
  //  int*&
  //  const int*&
  //  int* const
  //  const int* const
  //  int* const&
  //  const int* const&
  template <typename T>
  static T* InternalToPointer(const T* value)
  {
    return (T*)value;
  }

  // Handles:
  //  const int**
  //  const int**&
  template <typename T>
  static T* InternalToPointer(const T** value)
  {
    return (T*)*value;
  }

  // Handles:
  //  int**
  //  int**&
  template <typename T>
  static T* InternalToPointer(T* const* value)
  {
    return (T*)*value;
  }

// Takes any expression and turns it into a pointer of the core type
// Examples: const int** -> int*, or const int& -> int*
// To just get the type as a pointer rather than the expression, use
// LightningStrip(T)*
#  define LightningToPointer(Expression) (LS::TypeBinding::InternalToPointer<LightningStrip(decltype(Expression))>(Expression))

  // Strips all forms of const from a type
  template <typename T>
  class StripConst
  {
  public:
    typedef T Type;
  };

  template <typename T>
  class StripConst<const T> : public StripConst<T>
  {
  };

  template <typename T>
  class StripConst<const T&> : public StripConst<typename StripConst<T>::Type&>
  {
  };

  // Value to value
  template <typename From, typename To>
  class ReferenceCast
  {
  public:
    static To Cast(From from)
    {
      return from;
    }
  };
  // Value to pointer (illegal)
  template <typename From, typename To>
  class ReferenceCast<From, To*>
  {
  public:
    static To* Cast(From from)
    {
      return "IllegalConversionFromValueToPointer";
    }
  };
  // Value to reference (illegal)
  template <typename From, typename To>
  class ReferenceCast<From, To&>
  {
  public:
    static To& Cast(From from)
    {
      return "IllegalConversionFromValueToReference";
    }
  };
  // Pointer to pointer
  template <typename From, typename To>
  class ReferenceCast<From*, To*>
  {
  public:
    static To* Cast(From* from)
    {
      return from;
    }
  };
  // Pointer to reference
  template <typename From, typename To>
  class ReferenceCast<From*, To&>
  {
  public:
    static To& Cast(From* from)
    {
      return *from;
    }
  };
  // Pointer to value (only works because we specialized pointer and reference
  // above)
  template <typename From, typename To>
  class ReferenceCast<From*, To>
  {
  public:
    static To Cast(From* from)
    {
      return *from;
    }
  };
  // Reference to reference
  template <typename From, typename To>
  class ReferenceCast<From&, To&>
  {
  public:
    static To& Cast(From& from)
    {
      return from;
    }
  };
  // Reference to pointer
  template <typename From, typename To>
  class ReferenceCast<From&, To*>
  {
  public:
    static To* Cast(From& from)
    {
      return &from;
    }
  };
  // Reference to value
  template <typename From, typename To>
  class ReferenceCast<From&, To>
  {
  public:
    static To Cast(From& from)
    {
      return from;
    }
  };
  // Reference to a pointer to pointer
  template <typename From, typename To>
  class ReferenceCast<From*&, To*>
  {
  public:
    static To* Cast(From*& from)
    {
      return from;
    }
  };
  // Reference to a pointer to reference
  template <typename From, typename To>
  class ReferenceCast<From*&, To&>
  {
  public:
    static To& Cast(From*& from)
    {
      return *from;
    }
  };

  template <typename T>
  class FromDataPointer
  {
  public:
    static T Cast(byte* data)
    {
      return *(T*)data;
    }
  };

  template <typename T>
  class FromDataPointer<T*>
  {
  public:
    static T* Cast(byte* data)
    {
      return (T*)data;
    }
  };

  template <typename T>
  class FromDataPointer<T&>
  {
  public:
    static T& Cast(byte* data)
    {
      return *(T*)data;
    }
  };

  template <typename T>
  class ToDataPointer
  {
  public:
    static byte* Cast(const T& value)
    {
      return (byte*)&value;
    }
  };

  template <typename T>
  class ToDataPointer<T*>
  {
  public:
    static byte* Cast(T* value)
    {
      return (byte*)value;
    }
  };

  template <typename T>
  class ToDataPointer<T&>
  {
  public:
    static byte* Cast(T& value)
    {
      return (byte*)&value;
    }
  };

  // Lets us choose one type or the other based the condition (eg
  // TypeChooser<int, float, false>::Type will be float)
  template <typename IfTrueType, typename IfFalseType, bool condition>
  class TypeChooser
  {
  public:
  };

  template <typename IfTrueType, typename IfFalseType>
  class TypeChooser<IfTrueType, IfFalseType, true>
  {
  public:
    typedef IfTrueType Type;
  };

  template <typename IfTrueType, typename IfFalseType>
  class TypeChooser<IfTrueType, IfFalseType, false>
  {
  public:
    typedef IfFalseType Type;
  };

  PlasmaDeclareHasMemberTrait(HasDoNotBind, LightningDoNotBind);

  // A template for grabbing a TypeInfo given a template type
  // (This template can be specialized using macros)
  // This is to allow Rtti for pre-defined types such as int, via specialization
  template <typename T>
  class PlasmaSharedTemplate StaticTypeId
  {
  public:
    // The T type (and because partial specializations of 'StaticTypeId', this
    // will always be a stripped type)
    typedef T UnqualifiedType;

    // This type gets shadowed by the other partial specializations (and will be
    // the actual type when accessed from the LightningStaticType macro)
    typedef T QualifiedType;

    // The type we use in any generated binding. This is generally the
    // QualifiedType, however if the type is a redirect, it will be unqualified
    typedef T BindingType;

    // This is the type that we represent when written to the stack
    // (as a value type) or dereferenced from a handle (as a reference type)
    typedef T RepresentedType;

    // Because our representation is the exact same (whether in C++ or on the
    // stack/in a handle) then our resulting type when read is just a reference
    // (not the case in all specializations!)
    typedef T& ReadType;

    // Since our representation is the exact same (as mentioned above) this will
    // be a direct read
    static const bool DirectRead = true;

    // Read our object representation from either stack data or handle data
    static ReadType Read(byte* from)
    {
      // Reading and writing by default should just be pulling the object out
      // directly
      return *(T*)from;
    }

    // Write our object representation to either stack data or handle data
    static void Write(const T& value, byte* to)
    {
      memcpy(to, (void*)&value, sizeof(T));
    }

    template <typename U>
    static void ErrorOnDoNotBind(P_ENABLE_IF(HasDoNotBind<U>::value))
    {
      static_assert(Plasma::False<U>::Value,
                    "This type was marked to not allow binding. "
                    "Walk up the template errors until you find the "
                    "LightningBind/LightningTypeId calls that are causing the error.");
    }

    template <typename U>
    static void ErrorOnDoNotBind(P_DISABLE_IF(HasDoNotBind<U>::value))
    {
    }

    static BoundType*& GetType();

    static void AssertBadType(const char* prependedMessage)
    {
      Error("%sA type '%s' was gotten via LightningTypeId but was never "
            "initialized (see the template type T in the call stack "
            "if RTTI doesn't provide the name of the type)",
            prependedMessage,
            typeid(T).name());
    }
  };

  // A partial specialization for reference types
  template <typename T>
  class PlasmaSharedTemplate StaticTypeId<T&> : public StaticTypeId<T>
  {
  public:
    // This type gets shadowed by the other partial specializations (and will be
    // the actual type when accessed from the LightningStaticType macro)
    typedef T& QualifiedType;

    // Based on whether this object supports direct reading or not, we choose to
    // use the qualified or unqualified type in binding
    typedef typename TypeChooser<QualifiedType,
                                 typename StaticTypeId<T>::UnqualifiedType,
                                 StaticTypeId<T>::DirectRead>::Type BindingType;
  };

  // A partial specialization for reference types
  template <typename T>
  class PlasmaSharedTemplate StaticTypeId<const T> : public StaticTypeId<T>
  {
  public:
    // This type gets shadowed by the other partial specializations (and will be
    // the actual type when accessed from the LightningStaticType macro)
    typedef const T QualifiedType;

    // Based on whether this object supports direct reading or not, we choose to
    // use the qualified or unqualified type in binding
    typedef typename TypeChooser<QualifiedType,
                                 typename StaticTypeId<T>::UnqualifiedType,
                                 StaticTypeId<T>::DirectRead>::Type BindingType;
  };

  // A partial specialization for pointer types
  template <typename T>
  class PlasmaSharedTemplate StaticTypeId<T*> : public StaticTypeId<T>
  {
  public:
    // This type gets shadowed by the other partial specializations (and will be
    // the actual type when accessed from the LightningStaticType macro)
    typedef T* QualifiedType;

    // Based on whether this object supports direct reading or not, we choose to
    // use the qualified or unqualified type in binding
    typedef typename TypeChooser<QualifiedType,
                                 typename StaticTypeId<T>::UnqualifiedType,
                                 StaticTypeId<T>::DirectRead>::Type BindingType;
  };

  // We need to let binding know to redirect the type id operations
  template <typename T>
  class PlasmaSharedTemplate StaticTypeId<HandleOf<T>> : public StaticTypeId<T>
  {
  public:
    // This type gets shadowed by the other partial specializations (and will be
    // the actual type when accessed from the LightningStaticType macro)
    typedef HandleOf<T> QualifiedType;

    // Based on whether this object supports direct reading or not, we choose to
    // use the qualified or unqualified type in binding
    typedef HandleOf<T> BindingType;
  };

  // Used to count virtual table functions or get a virtual method index
  class VirtualTableCounter
  {
  public:
    // Constructor
    VirtualTableCounter();

    // A helper to tell us whether or not a method being tested is virtual
    static bool StaticDebugIsVirtual;

    // An extra helper to tell us whether or not a method being tested is
    // virtual
    bool InstanceDebugIsVirtual;

    // Checks if the function we were testing was virtual or not
    void AssertIfNotVirtual();

#  include "VirtualTableBinding.inl"
  };

  // The signature / size that this compiler uses for function pointers in a
  // virtual-table
  typedef void (TypeBinding::*VirtualTableFn)();

  // Get the number of entries in a class' virtual-table
  template <typename T>
  static size_t GetVirtualTableCount(typename IsPrimitive<T>::FalseType)
  {
    // We use a trick where we derive from the given type and add a single
    // virtual function, which when invoked on a 'counter' class will return us
    // the index of the last function (eg the count)
    class Derived : public T
    {
    public:
      // The last function in the virtual-table, which when reinterpreted and
      // called gives us the count
      virtual size_t GetVirtualTableCount()
      {
        // We can really return anything here, it should never be called
        Error("This should never be called!");
        return (size_t)-1;
      }
    };

    // This class has a virtual-table where every function returns its own index
    VirtualTableCounter counter;

    // Reinterpret the counter class as the derived class, and invoke
    // the last function (which will actually return the count)
    size_t count = reinterpret_cast<Derived*>(&counter)->GetVirtualTableCount();

    // Ensure the function we invoked a virtual function (this should always be
    // the case)
    counter.AssertIfNotVirtual();

    // Return the count we got from the virtual table
    return count;
  }

  template <typename T>
  static size_t GetVirtualTableCount()
  {
    return 0;
  }

  // Checks if a class has a virtual-table by counting it's virtual functions
  template <typename T>
  static bool HasVirtualTable()
  {
    return GetVirtualTableCount<T>() != 0;
  }

  // Gets the index of a given virtual method pointer
  // The method SHOULD be virtual, otherwise corruption can occur!
  template <typename Method>
  static size_t GetVirtualMethodIndex(Method methodPtr)
  {
    // Make another method that looks like our counter class methods
    typedef size_t (VirtualTableCounter::*IndexFn)();

    // Reinterpret cast the given method into our counter method
    // Because the method SHOULD be virtual, calling this on the counter will
    // return its index NOTE: If the function given is actually not virtual,
    // this will call the function on a BAD instance! If this occurs, memory may
    // be corrupted (we can't really counter this very well currently...)
    IndexFn indexMethodPtr = reinterpret_cast<IndexFn>(methodPtr);

    // This class has a virtual-table where every function returns its own index
    VirtualTableCounter counter;

    // Invoke the virtual method on our counter class (it better be virtual!)
    size_t index = (counter.*indexMethodPtr)();

    // Ensure the function we invoked a virtual function (if it hasn't already
    // been corrupted by this point)
    counter.AssertIfNotVirtual();

    // Return the index of the virtual function
    return index;
  }

  // A guid generator for functions (primarily virtual)
  static GuidType& GetFunctionCounter()
  {
    static GuidType counter = 0;
    return counter;
  }

  // Gives a unique id to a function (primarily used for virtual function
  // thunks)
  template <typename Function, Function function>
  static GuidType GetFunctionUniqueId()
  {
    static GuidType guid = GetFunctionCounter()++;
    return guid;
  };
};

// These helpers exist to fix template order issues by forcing implementation in the cpp files.
bool BoundTypeHelperIsRawCastable(BoundType* fromType, BoundType* toType);
bool BoundTypeHelperIsInitialized(BoundType* type);
bool BoundTypeHelperIsInitializedAssert(BoundType* type);
String BoundTypeHelperGetName(BoundType* type);
void LibraryBuilderHelperAddNativeBoundType(LibraryBuilder& builder,
                                            BoundType* type,
                                            BoundType* base,
                                            TypeCopyMode::Enum mode);
void InitializeTypeHelper(StringParam originalName, BoundType* type, size_t size, size_t rawVirtualcount);
template <typename T>
PlasmaSharedTemplate T InternalReadRef(byte* stackFrame);

template <typename T>
Handle::Handle(const HandleOf<T>& rhs) :
    StoredType(rhs.StoredType),
    Manager(rhs.Manager),
    Offset(rhs.Offset),
    Flags(rhs.Flags)
{
  // The data of a handle type is always memory-copyable
  memcpy(this->Data, rhs.Data, sizeof(this->Data));

#  ifdef LIGHTNING_HANDLE_DEBUG
  // Link ourselves to the global list of handles
  this->DebugLink();
#  endif

  // Increment the reference count since we're now referencing the same thing
  this->AddReference();
}

template <typename T>
Handle::Handle(const T& value, HandleManager* manager, ExecutableState* state)
{
  typedef typename TypeBinding::StripQualifiers<T>::Type UnqualifiedType;
  const UnqualifiedType* pointer = TypeBinding::ReferenceCast<T&, const UnqualifiedType*>::Cast((T&)value);
  BoundType* type = LightningVirtualTypeId(pointer);
  BoundTypeHelperIsInitializedAssert(type);
  BoundTypeHelperIsInitializedAssert(LightningTypeId(T));
  this->Initialize((byte*)pointer, type, manager, state);
}

template <typename T>
T Handle::Get(GetOptions::Enum options) const
{
  if (this->StoredType == nullptr)
  {
    ErrorIf(options == GetOptions::AssertOnNull, "The value inside the Handle was null");
    return T();
  }

  // Check if we can directly convert the stored type into the requested type
  // This supports derived -> base class casting (but not visa versa), enums to
  // integers, etc
  BoundType* toType = LightningTypeId(T);
  if (BoundTypeHelperIsRawCastable(this->StoredType, toType) == false)
  {
    ErrorIf(options == GetOptions::AssertOnNull,
            "There was a value inside the Handle of type '%s' but it cannot be "
            "converted",
            BoundTypeHelperGetName(this->StoredType).c_str());
    return T();
  }

  return InternalReadRef<T>((byte*)this);
}

template <typename T>
HandleOf<T>::HandleOf()
{
}

template <typename T>
HandleOf<T>::HandleOf(const Handle& other) : Handle(other)
{
  if (this->StoredType != nullptr && this->StoredType->IsA(this->GetCompileType()) == false)
  {
    // Clear the handle out since its invalid
    Error("The handle we're constructing from does not match our type");
    this->Clear();
  }
}

template <typename T>
HandleOf<T>::HandleOf(Handle&& other) : Handle(other)
{
  if (this->StoredType != nullptr && this->StoredType->IsA(this->GetCompileType()) == false)
  {
    // Clear the handle out since its invalid
    Error("The handle we're constructing from does not match our type");
    this->Clear();
  }
}

template <typename T>
HandleOf<T>::HandleOf(const T& value) : Handle((const byte*)&value, LightningVirtualTypeId(&value))
{
}

template <typename T>
HandleOf<T>::HandleOf(const T* value) : Handle((const byte*)value, LightningVirtualTypeId(value))
{
}

template <typename T>
BoundType* HandleOf<T>::GetCompileType() const
{
  return LightningTypeId(T);
}

template <typename T>
HandleOf<T>::operator T*() const
{
  return (T*)Handle::Dereference();
}

template <typename T>
HandleOf<T>::operator T&() const
{
  return *(T*)Handle::Dereference();
}

template <typename T>
T* HandleOf<T>::operator->() const
{
  T* self = (T*)Handle::Dereference();
  ErrorIf(self == nullptr,
          "Attempting to -> off a a null handle. Check the handle via "
          "'if(Type* value = handle)'");
  return self;
}

template <typename T>
T* HandleOf<T>::operator*() const
{
  T* self = (T*)Handle::Dereference();
  return self;
}

template <typename T>
void HandleOf<T>::SafeDestroy()
{
  T* instance = *this;
  if (instance)
    instance->Destroy();
}

template <typename T>
T* HandleOf<T>::Dereference() const
{
  return (T*)Handle::Dereference();
}

/************************************ VOID
 * *************************************/
template <>
class PlasmaShared TypeBinding::StaticTypeId<void>
{
public:
  static BoundType* GetType();
};

/************************************ NO TYPE
 * *************************************/
template <>
class PlasmaShared TypeBinding::StaticTypeId<NoType>
{
public:
  static BoundType* GetType();
};

/************************************ NULL POINTER
 * *************************************/
template <>
class PlasmaShared TypeBinding::StaticTypeId<NullPointerType>
{
public:
  static BoundType* GetType();
};

/************************************ ANY *************************************/
template <>
class PlasmaShared TypeBinding::StaticTypeId<Any>
{
public:
  // The T type (and because partial specializations of 'StaticTypeId', this
  // will always be a stripped type)
  typedef Any UnqualifiedType;

  // This type gets shadowed by the other partial specializations (and will be
  // the actual type when accessed from the LightningStaticType macro)
  typedef Any QualifiedType;

  // The type we use in any generated binding. This is generally the
  // QualifiedType, however if the type is a redirect, it will be unqualified
  typedef Any BindingType;

  // This is the type that we represent when written to the stack
  // (as a value type) or dereferenced from a handle (as a reference type)
  typedef Any RepresentedType;

  // Because our representation is the exact same (whether in C++ or on the
  // stack/in a handle) then our resulting type when read is just a reference
  // (not the case in all specializations!)
  typedef Any& ReadType;

  // Since our representation is the exact same (as mentioned above) this will
  // be a direct read
  static const bool DirectRead = true;

  static AnyType* GetType();
};

/************************************ ANY DELEGATE
 * *************************************/
template <>
class PlasmaShared TypeBinding::StaticTypeId<Delegate>
{
public:
  // The T type (and because partial specializations of 'StaticTypeId', this
  // will always be a stripped type)
  typedef Delegate UnqualifiedType;

  // This type gets shadowed by the other partial specializations (and will be
  // the actual type when accessed from the LightningStaticType macro)
  typedef Delegate QualifiedType;

  // The type we use in any generated binding. This is generally the
  // QualifiedType, however if the type is a redirect, it will be unqualified
  typedef Delegate BindingType;

  // This is the type that we represent when written to the stack
  // (as a value type) or dereferenced from a handle (as a reference type)
  typedef Delegate RepresentedType;

  // Because our representation is the exact same (whether in C++ or on the
  // stack/in a handle) then our resulting type when read is just a reference
  // (not the case in all specializations!)
  typedef Delegate& ReadType;

  // Since our representation is the exact same (as mentioned above) this will
  // be a direct read
  static const bool DirectRead = true;

  static DelegateType* GetType();
};

/************************************ ANY HANDLE
 * *************************************/
template <>
class PlasmaShared TypeBinding::StaticTypeId<Handle>
{
public:
  // The T type (and because partial specializations of 'StaticTypeId', this
  // will always be a stripped type)
  typedef Handle UnqualifiedType;

  // This type gets shadowed by the other partial specializations (and will be
  // the actual type when accessed from the LightningStaticType macro)
  typedef Handle QualifiedType;

  // The type we use in any generated binding. This is generally the
  // QualifiedType, however if the type is a redirect, it will be unqualified
  typedef Handle BindingType;

  // This is the type that we represent when written to the stack
  // (as a value type) or dereferenced from a handle (as a reference type)
  typedef Handle RepresentedType;

  // Because our representation is the exact same (whether in C++ or on the
  // stack/in a handle) then our resulting type when read is just a reference
  // (not the case in all specializations!)
  typedef Handle& ReadType;

  // Since our representation is the exact same (as mentioned above) this will
  // be a direct read
  static const bool DirectRead = true;

  static BoundType* GetType();
};

/*********************************** TRAITS
 * ************************************/

template <typename SelfType, typename BaseType>
class PlasmaSharedTemplate CheckTypesAreRelated
{
public:
  static void Test()
  {
    // Check that the derived type is bigger than the
    // base type (should be since we have Debug_SizeTest)
    static_assert(sizeof(SelfType) >= sizeof(BaseType),
                  "It appears either the derived class or parent class is incorrect");

    // Attempt a static cast to ensure the types given were related
    // We need to use an invalid pointer (but not null) to avoid this getting
    // compiled out
    BaseType* type1 = (BaseType*)0x00000001;
    SelfType* type2 = (SelfType*)0x00000001;
    type2 = static_cast<SelfType*>(type1);
    type1 = static_cast<BaseType*>(type2);
  }
};

template <typename SelfType>
class PlasmaSharedTemplate CheckTypesAreRelated<SelfType, NoType>
{
public:
  static void Test()
  {
  }
};

/*********************************** CHECKS
 * ************************************/

// If we're in debug mode, add extra checks...
#  if PlasmaDebug
// Checks used in the declaration of a C++ type exposed to Lightning
#    define LightningDeclareChecks(SelfType, BaseType)                                                                     \
      /* Do a series of debug checks to ensure the user is using the macros                                            \
       * correctly */                                                                                                  \
      void LightningDebugChecks()                                                                                          \
      {                                                                                                                \
        /* Check that the sizes of the type we declared as 'our type' is the                                           \
         * same as the size of the this reference */                                                                   \
        static_assert(sizeof(SelfType) == sizeof(*this),                                                               \
                      "The type passed into the macro wasn't the same as the "                                         \
                      "class it was declared in");                                                                     \
        /* Check that the two types are related via static casting */                                                  \
        LS::CheckTypesAreRelated<SelfType, BaseType>::Test();                                                          \
      }                                                                                                                \
      static void LightningDebugDerivedHasNotBeenDeclared()                                                                \
      {                                                                                                                \
      }                                                                                                                \
      static void LightningDebugBaseHasNotBeenDeclared()                                                                   \
      {                                                                                                                \
      }
#  else
#    define LightningDeclareChecks(SelfType, BaseType)
#  endif

/**************************** EXTERNAL REDIRECTION
 * *****************************/
#  define LightningDeclareCustomType(SelfType, CustomBoundType, Linkage)                                                   \
    /* A specialization so we know that type info exists for this type */                                              \
    template <>                                                                                                        \
    class Linkage LightningStaticType(SelfType)                                                                            \
    {                                                                                                                  \
    public:                                                                                                            \
      typedef SelfType UnqualifiedType;                                                                                \
      typedef SelfType QualifiedType;                                                                                  \
      typedef SelfType BindingType;                                                                                    \
      typedef SelfType RepresentedType;                                                                                \
      typedef SelfType& ReadType;                                                                                      \
      static const bool DirectRead = true;                                                                             \
      /* Implementation of the 'get type' specialization */                                                            \
      static LS::BoundType*(GetType)()                                                                                 \
      {                                                                                                                \
        static BoundType* type = (CustomBoundType);                                                                    \
        return type;                                                                                                   \
      }                                                                                                                \
      /* Read our object representation from either stack data or handle data                                          \
       */                                                                                                              \
      static typename LightningStaticType(SelfType)::ReadType(Read)(byte * from)                                           \
      {                                                                                                                \
        return *(SelfType*)from;                                                                                       \
      }                                                                                                                \
      /* Write our object representation to either stack data or handle data                                           \
       */                                                                                                              \
      static void(Write)(const SelfType& value, byte* to)                                                              \
      {                                                                                                                \
        memcpy(to, &value, sizeof(SelfType));                                                                          \
      }                                                                                                                \
    };

// Declare an external type
#  define LightningDeclareDefineRedirectType(SelfType, RedirectType, ConvertFromRedirect, ConvertToRedirect, Linkage)      \
    /* A specialization so we know that type info exists for this type */                                              \
    template <>                                                                                                        \
    class Linkage LightningStaticType(SelfType)                                                                            \
    {                                                                                                                  \
    public:                                                                                                            \
      typedef SelfType UnqualifiedType;                                                                                \
      typedef SelfType QualifiedType;                                                                                  \
      typedef SelfType BindingType;                                                                                    \
      typedef RedirectType RepresentedType;                                                                            \
      typedef SelfType ReadType;                                                                                       \
      static const bool DirectRead = false;                                                                            \
      /* Implementation of the 'get type' specialization */                                                            \
      static LS::BoundType*(GetType)()                                                                                 \
      {                                                                                                                \
        return LightningTypeId(RepresentedType);                                                                           \
      }                                                                                                                \
      /* Read our object representation from either stack data or handle data                                          \
       */                                                                                                              \
      static typename LightningStaticType(SelfType)::ReadType(Read)(byte * from)                                           \
      {                                                                                                                \
        return ConvertToRedirect(*(RepresentedType*)from);                                                             \
      }                                                                                                                \
      /* Write our object representation to either stack data or handle data                                           \
       */                                                                                                              \
      static void(Write)(const SelfType& value, byte* to)                                                              \
      {                                                                                                                \
        new (to) RepresentedType(ConvertFromRedirect(value));                                                          \
      }                                                                                                                \
    };

// Can be used by redirection macros to support changing of a type to another
// type
template <typename From, typename To>
To StaticCast(const From& from)
{
  return static_cast<To>(from);
}

// Define an external type with a given name that can be statically casted to
// our redirected type
#  define LightningDeclareDefineImplicitRedirectType(SelfType, RedirectType, Linkage)                                      \
    LightningDeclareDefineRedirectType(                                                                                    \
        SelfType, RedirectType, (StaticCast<SelfType, RedirectType>), (StaticCast<RedirectType, SelfType>), Linkage)

/********************************** BINDING ***********************************/

// This will grab the current type being allocated via a Lightning constructor call
// Note that the Type may be null when allocated directly from C++  (not via
// Lightning API)
class AutoGrabAllocatingType
{
public:
  AutoGrabAllocatingType();

  // This effectively acts as the virtual table / type for Lightning
  BoundType* Type;
};

#  define LightningDeclareInheritableType(SelfType, TypeCopyMode)                                                          \
    LightningDeclareType(SelfType, TypeCopyMode);                                                                          \
    LS::AutoGrabAllocatingType LightningDerivedType;                                                                       \
    LS::BoundType* LightningDerivedTypeOverride() const                                                                    \
    {                                                                                                                  \
      if (this->LightningDerivedType.Type != nullptr)                                                                      \
        return this->LightningDerivedType.Type;                                                                            \
      return LightningTypeId(LightningSelf);                                                                                   \
    }

PlasmaDeclareHasMemberTrait(HasLightningDerivedTypeOverride, LightningDerivedTypeOverride);

// This helper allows us to use SFINAE to detect if we have a base type (only
// for internal binding) If we do not have internal binding, then we have to
// explicitly set the base type
template <typename T>
BoundType* GetDerivedTypeOverride(const T* self, P_ENABLE_IF(HasLightningDerivedTypeOverride<T>::value))
{
  return self->LightningDerivedTypeOverride();
}

template <typename T>
BoundType* GetDerivedTypeOverride(const T* self, P_DISABLE_IF(HasLightningDerivedTypeOverride<T>::value))
{
  return LightningTypeId(T);
}

PlasmaDeclareHasMemberTrait(HasLightningSetupType, LightningSetupType);

// This helper allows us to use SFINAE to detect if we have a base type (only
// for internal binding) If we do not have internal binding, then we have to
// explicitly set the base type
template <typename LightningSelf, typename SetupFunction>
void SetupType(LibraryBuilder& builder,
               BoundType* type,
               SetupFunction setupType,
               P_ENABLE_IF(HasLightningSetupType<LightningSelf>::value))
{
  LibraryBuilderHelperAddNativeBoundType(
      builder, type, LightningTypeId(typename LightningSelf::LightningBase), LightningSelf::LightningCopyMode);
  if (setupType != nullptr)
    setupType(nullptr, builder, type);
  LightningSelf::LightningSetupType(builder, type);
};

template <typename LightningSelf, typename SetupFunction>
void SetupType(LibraryBuilder& builder,
               BoundType* type,
               SetupFunction setupType,
               P_DISABLE_IF(HasLightningSetupType<LightningSelf>::value))
{
  ErrorIf(setupType == nullptr,
          "No setup function provided for externally BoundType %s. "
          "Be sure you are calling the correct initialize function: "
          "LightningInitializeExternalType,"
          "LightningInitializeRange, or LightningInitializeEnum.",
          BoundTypeHelperGetName(type).c_str());
  setupType(nullptr, builder, type);
};

// This function gets called when the static library we belong to is built
template <typename InitializingType, typename StaticLibraryType, typename SetupFunction>
BoundType* InitializeType(const char* initializingTypeName, SetupFunction setupType = nullptr)
{
  // Check if we've already been initialized
  BoundType* type = LightningTypeId(InitializingType);
  if (BoundTypeHelperIsInitialized(type))
    return type;

  // First initialize our base type...
  StaticLibraryType& library = StaticLibraryType::GetInstance();
  if (library.CanBuildTypes() == false)
  {
    String message = String::Format("LightningInitializeType can only be called on "
                                    "the type %s when its library is in the process of being "
                                    "initialized via %s::GetInstance().BuildLibrary()",
                                    initializingTypeName,
                                    library.Name.c_str());
    Error(message.c_str());
    return type;
  }
  String typeName = initializingTypeName;
  InitializeTypeHelper(typeName, type, sizeof(InitializingType), TypeBinding::GetVirtualTableCount<InitializingType>());
  LibraryBuilder& builder = *library.GetBuilder();
  SetupType<InitializingType>(builder, type, setupType);
  return type;
};

  // A helper for initializing types that belong to a library
#  define LightningInitializeTypeAs(Type, Name)                                                                            \
    LS::InitializeType<Type, LightningLibrary, void (*)(Type*, LS::LibraryBuilder&, LS::BoundType*)>(Name)
#  define LightningInitializeType(Type) LightningInitializeTypeAs(Type, #  Type)

  // A helper for initializing types that belong to a library
#  define LightningInitializeExternalTypeAs(Type, Name)                                                                    \
    LS::InitializeType<Type, LightningLibrary, void (*)(Type*, LS::LibraryBuilder&, LS::BoundType*)>(Name, LightningSetupType)
#  define LightningInitializeExternalType(Type) LightningInitializeExternalTypeAs(Type, #  Type)

  // Declares a lightning derived type (belongs inside the type definition)
#  define LightningDeclareDerivedTypeExplicit(SelfType, BaseType, CopyMode)                                                \
  public:                                                                                                              \
    /* Current class being bound and it's base class */                                                                \
    /* Classes without a base class will have LightningBase defined as NoType */                                           \
    typedef SelfType LightningSelf;                                                                                        \
    typedef BaseType LightningBase;                                                                                        \
    typedef LightningSelf* LightningSelfPointer;                                                                               \
    typedef LightningBase* LightningBasePointer;                                                                               \
    /* Get the most derived type from an instance of the class */                                                      \
    /* Inheriting from 'ILightningObject' will make these virtual */                                                       \
    /* We don't want to implicitly introduce v-tables to anyone's structures                                           \
     */                                                                                                                \
    /*virtual*/ LS::BoundType* LightningGetDerivedType() const                                                             \
    {                                                                                                                  \
      return LS::GetDerivedTypeOverride<LightningSelf>(this);                                                              \
    }                                                                                                                  \
    /* This lets binding know how the class should be treated when just                                                \
     * referred to by its name */                                                                                      \
    static const LS::TypeCopyMode::Enum LightningCopyMode = CopyMode;                                                      \
    /* This function is to be implemented by the user so that */                                                       \
    /* they may choose what they want to bind to reflection */                                                         \
    static void LightningSetupType(LS::LibraryBuilder& builder, LS::BoundType* type)

  // Declares a lightning base type (belongs inside the type definition)
#  define LightningDeclareBaseTypeExplicit(SelfType, CopyMode)                                                             \
    LightningDeclareDerivedTypeExplicit(SelfType, LS::NoType, CopyMode)

  // Completely prevents binding from occurring with this type,
  // including calls like LightningTypeId() and any sort of bind macro
#  define LightningDoNotAllowBinding() static void LightningDoNotBind();

  // Declares a lightning type (belongs inside the type definition)
  // Implicitly deduces self and base types being bound
#  define LightningDeclareType(ClassType, CopyMode)                                                                        \
  public:                                                                                                              \
    /* Binding macros need the current class being bound */                                                            \
    /* This is a really neat trick where we never need to declare our base                                             \
     * type */                                                                                                         \
    /* because of how typedefs are inherited (and they only apply to members                                           \
     * below */                                                                                                        \
    /* The only issue is the base case (no base class), which we use SFINAE */                                         \
    /* to detect if we even inherited a LightningSelf typedef */                                                           \
    template <typename LightningT>                                                                                         \
    static typename LightningT::LightningSelf* SfinaeBase(int);                                                                \
    template <typename LightningT>                                                                                         \
    static LS::NoType SfinaeBase(...);                                                                                 \
    typedef typename PE::remove_pointer<decltype(SfinaeBase<ClassType>(0))>::type LightningTempBase;                       \
    LightningDeclareDerivedTypeExplicit(ClassType, LightningTempBase, CopyMode)

#  define LightningDefineType(SelfType, builder, type)                                                                     \
    void SelfType::LightningSetupType(LS::LibraryBuilder& builder, LS::BoundType* type)

#  define LightningDefineTemplateType(SelfType, builder, type)                                                             \
    template <>                                                                                                        \
    void SelfType::LightningSetupType(LS::LibraryBuilder& builder, LS::BoundType* type)

#  define LightningDeclareExternalType(SelfType)                                                                           \
    void LightningSetupType(SelfType*, LS::LibraryBuilder& builder, LS::BoundType* type);

  // The declaration of the external type must be within the same namespace as
  // the definition Alternatively, all the externally bound types can be defined
  // ABOVE the library initialization
#  define LightningDefineExternalDerivedType(SelfType, SelfBaseType, TypeCopyMode, builder, type)                          \
    /* This is just a forward declared template which is literally defined                                             \
     * below by the user */                                                                                            \
    /* We want LightningSelf and LightningBase to act as if they are typedefs (because                                         \
     * many binding macros depend on them) */                                                                          \
    template <typename LightningSelf, typename LightningBase>                                                                  \
    void LightningSetupExternalType(SelfType*, LS::LibraryBuilder& builder, LS::BoundType* type);                          \
    void LightningSetupType(SelfType*, LS::LibraryBuilder& builder, LS::BoundType* type)                                   \
    {                                                                                                                  \
      builder.AddNativeBoundType(type, LightningTypeId(SelfBaseType), TypeCopyMode);                                       \
      LightningSetupExternalType<SelfType, SelfBaseType>((SelfType*)nullptr, builder, type);                               \
    }                                                                                                                  \
    template <typename LightningSelf, typename LightningBase>                                                                  \
    void LightningSetupExternalType(SelfType*, LS::LibraryBuilder& builder, LS::BoundType* type)

#  define LightningDefineExternalBaseType(SelfType, CopyMode, builder, type)                                               \
    LightningDefineExternalDerivedType(SelfType, LS::NoType, CopyMode, builder, type)

  /************************************* ENUM
   * ************************************/
#  define LightningBindEnumValues(enumType)                                                                                \
    for (uint i = 0; i < enumType::Size; ++i)                                                                          \
    {                                                                                                                  \
      LightningFullBindEnumValue(builder, type, enumType::Values[i], enumType::Names[i]);                                  \
    }

#  define LightningDefineEnum(enumType)                                                                                    \
    LightningDefineExternalBaseType(enumType::Enum, LS::TypeCopyMode::ValueType, builder, type)                            \
    {                                                                                                                  \
      LightningFullBindEnum(builder, type, LS::SpecialType::Enumeration);                                                  \
      LightningBindEnumValues(enumType);                                                                                   \
    }

#  define LightningInitializeEnum(enumType) LightningInitializeExternalTypeAs(enumType::Enum, #  enumType);
#  define LightningInitializeEnumAs(enumType, name) LightningInitializeExternalTypeAs(enumType::Enum, name);

/********************************** PRIMITIVE
 * **********************************/
LightningDeclareExternalType(Boolean);
LightningDeclareExternalType(Boolean2);
LightningDeclareExternalType(Boolean3);
LightningDeclareExternalType(Boolean4);
LightningDeclareExternalType(Byte);
LightningDeclareExternalType(Integer);
LightningDeclareExternalType(Integer2);
LightningDeclareExternalType(Integer3);
LightningDeclareExternalType(Integer4);
LightningDeclareExternalType(Real);
LightningDeclareExternalType(Real2);
LightningDeclareExternalType(Real3);
LightningDeclareExternalType(Real4);
LightningDeclareExternalType(Quaternion);
LightningDeclareExternalType(Real2x2);
LightningDeclareExternalType(Real3x3);
LightningDeclareExternalType(Real4x4);
LightningDeclareExternalType(String);
LightningDeclareExternalType(DoubleReal);
LightningDeclareExternalType(DoubleInteger);

// All the redirection types
LightningDeclareDefineImplicitRedirectType(char, Integer, PlasmaShared);
LightningDeclareDefineImplicitRedirectType(signed char, Integer, PlasmaShared);
LightningDeclareDefineImplicitRedirectType(signed short, Integer, PlasmaShared);
LightningDeclareDefineImplicitRedirectType(unsigned short, Integer, PlasmaShared);

LightningDeclareDefineImplicitRedirectType(unsigned int, Integer, PlasmaShared);
LightningDeclareDefineImplicitRedirectType(signed long, Integer, PlasmaShared);
LightningDeclareDefineImplicitRedirectType(unsigned long, Integer, PlasmaShared);

LightningDeclareDefineImplicitRedirectType(unsigned long long, DoubleInteger, PlasmaShared);
} // namespace Lightning

#endif
