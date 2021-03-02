// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

// ReferenceCounted
// SafeId
// ThreadSafe

// This is used to avoid people copy constructing their classes and copying over
// the Id.
template <typename IdType>
struct HandleIdData
{
  HandleIdData()
  {
  }
  HandleIdData(const HandleIdData& rhs)
  {
  }
  HandleIdData& operator=(const HandleIdData& rhs)
  {
    return *this;
  }

  IdType mId;
};

// Prevents count from being copied
class ReferenceCountData
{
public:
  ReferenceCountData()
  {
  }
  ReferenceCountData(const ReferenceCountData&)
  {
  }
  ReferenceCountData& operator=(const ReferenceCountData&)
  {
    return *this;
  }

  Atomic<int> mCount;
};

// Declare
// Call within the class definition
#define DeclareReferenceCountedHandle() DeclareReferenceCountedHandleInternals() typedef u32 HandleIdType;

// The reason for adding this internals version was so that we could avoid two
// HandleIdType typedefs. This should never be called outside this file.
#define DeclareReferenceCountedHandleInternals()                                                                       \
  ReferenceCountData mPlasmaHandleReferenceCount;                                                                        \
  void AddReference()                                                                                                  \
  {                                                                                                                    \
    ++mPlasmaHandleReferenceCount.mCount;                                                                                \
  }                                                                                                                    \
  int Release()                                                                                                        \
  {                                                                                                                    \
    ErrorIf(mPlasmaHandleReferenceCount.mCount == 0, "Invalid Release. ReferenceCount is plasma.");                        \
    int referenceCount = --mPlasmaHandleReferenceCount.mCount;                                                           \
    if (referenceCount == 0)                                                                                           \
      delete this;                                                                                                     \
    return referenceCount;                                                                                             \
  }

#define DeclareSafeIdHandle(idType)                                                                                    \
  typedef idType HandleIdType;                                                                                         \
  HandleIdData<HandleIdType> mPlasmaHandleId;                                                                            \
  static HandleIdType mPlasmaHandleCurrentId;                                                                            \
  static HashMap<HandleIdType, LightningSelf*> mPlasmaHandleLiveObjects;

#define DeclareThreadSafeIdHandle(idType) DeclareSafeIdHandle(idType) static ThreadLock mPlasmaHandleLock;

#define DeclareReferenceCountedSafeIdHandle(idType) DeclareReferenceCountedHandleInternals() DeclareSafeIdHandle(idType)

#define DeclareReferenceCountedThreadSafeIdHandle(idType)                                                              \
  DeclareReferenceCountedHandleInternals() DeclareThreadSafeIdHandle(idType)

// Define
#define DefineSafeIdHandle(type)                                                                                       \
  type::HandleIdType type::mPlasmaHandleCurrentId = 1;                                                                   \
  HashMap<type::HandleIdType, type*> type::mPlasmaHandleLiveObjects;

#define DefineThreadSafeIdHandle(type) DefineSafeIdHandle(type) ThreadLock type::mPlasmaHandleLock;

#define DefineReferenceCountedSafeIdHandle(type) DefineReferenceCountedHandle(type) DefineSafeIdHandle(type)

#define DefineReferenceCountedThreadSafeIdHandle(type) DefineReferenceCountedHandle(type) DefineThreadSafeIdHandle(type)

// Constructor
// Call in the constructor and copy constructor of the object
#define ConstructReferenceCountedHandle() mPlasmaHandleReferenceCount.mCount = 0;

#define ConstructSafeIdHandle()                                                                                        \
  mPlasmaHandleId.mId = mPlasmaHandleCurrentId++;                                                                          \
  mPlasmaHandleLiveObjects.Insert(mPlasmaHandleId.mId, this);

#define ConstructThreadSafeIdHandle()                                                                                  \
  mPlasmaHandleLock.Lock();                                                                                              \
  ConstructSafeIdHandle();                                                                                             \
  mPlasmaHandleLock.Unlock();

#define ConstructReferenceCountedSafeIdHandle()                                                                        \
  ConstructReferenceCountedHandle();                                                                                   \
  ConstructSafeIdHandle();

#define ConstructReferenceCountedThreadSafeIdHandle()                                                                  \
  ConstructReferenceCountedHandle();                                                                                   \
  ConstructThreadSafeIdHandle();

// Destructor
// Call in the destructor of the object
#define DestructReferenceCountedHandle()                                                                               \
  ErrorIf(mPlasmaHandleReferenceCount.mCount != 0, "Bad reference Count. Object is being deleted with references!");

#define DestructSafeIdHandle()                                                                                         \
  bool isErased = mPlasmaHandleLiveObjects.Erase(mPlasmaHandleId.mId);                                                     \
  ErrorIf(!isErased, "The handle was not in the live objects map, but should have been");

#define DestructThreadSafeIdHandle()                                                                                   \
  mPlasmaHandleLock.Lock();                                                                                              \
  DestructSafeIdHandle();                                                                                              \
  mPlasmaHandleLock.Unlock();

// For the last two, we don't want to call 'DestructReferenceCountedHandle'
// because it's safe to delete the object even if it still has references
#define DestructReferenceCountedSafeIdHandle() DestructSafeIdHandle();

#define DestructReferenceCountedThreadSafeIdHandle() DestructThreadSafeIdHandle();

// Bind Manager
// Call in the meta initialization of the class type
#define PlasmaBindHandle() type->HandleManager = LightningManagerId(PlasmaHandleManager<LightningSelf>);

// Register Manager
// Call in the meta initialization of the library
#define PlasmaRegisterHandleManager(type) LightningRegisterSharedHandleManager(PlasmaHandleManager<type>);

// Handle Manager
template <typename T>
class PlasmaHandleManager : public HandleManager
{
public:
  PlasmaHandleManager(ExecutableState* state) : HandleManager(state)
  {
  }

  typedef typename T::HandleIdType IdType;

  PlasmaDeclareHasMemberTrait(IsReferenceCounted, mPlasmaHandleReferenceCount);
  PlasmaDeclareHasMemberTrait(IsSafeId, mPlasmaHandleId);
  PlasmaDeclareHasMemberTrait(IsThreadSafe, mPlasmaHandleLock);

  // Handle Data
  struct HandleData
  {
    void* mRawObject;
    IdType mId;
  };

  void Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags) override
  {
    if (IsReferenceCounted<T>::value)
      handleToInitialize.Flags |= HandleFlags::NoReferenceCounting;

    if (IsSafeId<T>::value)
    {
      // METAREFACTOR - If we only ever go through LightningAllocate for objects
      // that use this handle manager, we can assign the id here and not have to
      // deal with the raw object issue. Is this something we should do? Or is
      // that annoying to LightningAllocate everything?
      HandleData& data = *(HandleData*)(handleToInitialize.Data);
      data.mId = (IdType)-1;
      data.mRawObject = plAllocate(type->Size);
    }
  }

  void ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize) override
  {
    ObjectToHandleInternal(object, type, handleToInitialize);
  }

  byte* HandleToObject(const Handle& handle) override
  {
    return HandleToObjectInternal(handle);
  }

  void AddReference(const Handle& handle) override
  {
    AddReferenceInternal(handle);
  }

  ReleaseResult::Enum ReleaseReference(const Handle& handle) override
  {
    return ReleaseReferenceInternal(handle);
  }

  // Internals
  // Internal versions of each function were added because they need to be
  // templated for SFINAE

  // ObjectToHandle
  // Only reference counted
  template <typename U = T>
  void ObjectToHandleInternal(const byte* object,
                              BoundType* type,
                              Handle& handleToInitialize,
                              P_ENABLE_IF(IsReferenceCounted<U>::value && !IsSafeId<U>::value))
  {
    T* instance = (T*)object;

    if (instance != nullptr)
      instance->AddReference();

    handleToInitialize.HandlePointer = (byte*)object;
  }

  // Only safe id
  template <typename U = T>
  void ObjectToHandleInternal(const byte* object,
                              BoundType* type,
                              Handle& handleToInitialize,
                              P_ENABLE_IF(!IsReferenceCounted<U>::value && IsSafeId<U>::value))
  {
    T* instance = (T*)object;

    HandleData& data = *(HandleData*)(handleToInitialize.Data);
    if (instance != nullptr)
      data.mId = instance->mPlasmaHandleId.mId;
  }

  // Reference counted and safe id
  template <typename U = T>
  void ObjectToHandleInternal(const byte* object,
                              BoundType* type,
                              Handle& handleToInitialize,
                              P_ENABLE_IF(IsReferenceCounted<U>::value&& IsSafeId<U>::value))
  {
    T* instance = (T*)object;

    HandleData& data = *(HandleData*)(handleToInitialize.Data);
    data.mId = 0;

    if (instance != nullptr)
    {
      instance->AddReference();
      data.mId = instance->mPlasmaHandleId.mId;
    }
  }

  // HandleToObject
  // Only reference counted
  template <typename U = T>
  byte* HandleToObjectInternal(const Handle& handle, P_ENABLE_IF(IsReferenceCounted<U>::value && !IsSafeId<U>::value))
  {
    return (byte*)handle.HandlePointer;
  }

  // Only safe id
  template <typename U = T>
  byte* HandleToObjectInternal(const Handle& handle, P_ENABLE_IF(IsSafeId<U>::value && !IsThreadSafe<U>::value))
  {
    HandleData& data = *(HandleData*)(handle.Data);

    if (data.mRawObject)
      return (byte*)data.mRawObject;

    T* object = T::mPlasmaHandleLiveObjects.FindValue(data.mId, nullptr);
    return (byte*)object;
  }

  // Thread safe id
  template <typename U = T>
  byte* HandleToObjectInternal(const Handle& handle, P_ENABLE_IF(IsSafeId<U>::value&& IsThreadSafe<U>::value))
  {
    HandleData& data = *(HandleData*)(handle.Data);

    if (data.mRawObject)
      return (byte*)data.mRawObject;

    T::mPlasmaHandleLock.Lock();
    T* object = T::mPlasmaHandleLiveObjects.FindValue(data.mId, nullptr);
    T::mPlasmaHandleLock.Unlock();
    return (byte*)object;
  }

  // AddReference
  template <typename U = T>
  void AddReferenceInternal(const Handle& handle, P_ENABLE_IF(IsReferenceCounted<U>::value))
  {
    T* instance = (T*)HandleToObject(handle);
    instance->AddReference();
  }

  template <typename U = T>
  void AddReferenceInternal(const Handle& handle, P_DISABLE_IF(IsReferenceCounted<U>::value))
  {
  }

  // ReleaseReference
  template <typename U = T>
  ReleaseResult::Enum ReleaseReferenceInternal(const Handle& handle, P_ENABLE_IF(IsReferenceCounted<U>::value))
  {
    T* instance = (T*)HandleToObject(handle);
    instance->Release();

    // METAREFACTOR - To get rid of Reference class below, Release has to change
    // to not manually deleting itself. This will not call the lightning destructor
    // on inherited types, causing leaks.
    return ReleaseResult::TakeNoAction;
  }

  template <typename U = T>
  ReleaseResult::Enum ReleaseReferenceInternal(const Handle& handle, P_DISABLE_IF(IsReferenceCounted<U>::value))
  {
    return ReleaseResult::TakeNoAction;
  }
};

// Plasma Handle Object
// Used as the default base for inheritable handle types
class EmptyClass
{
};

// Reference Counted
template <typename Base = EmptyClass>
class ReferenceCounted : public Base
{
public:
  LightningDeclareType(ReferenceCounted, TypeCopyMode::ReferenceType);
  DeclareReferenceCountedHandle();

  ReferenceCounted()
  {
    ConstructReferenceCountedHandle();
  }

  ReferenceCounted(const ReferenceCounted&)
  {
    ConstructReferenceCountedHandle();
  }

  virtual ~ReferenceCounted()
  {
    DestructReferenceCountedHandle();
  }
};

template <typename Base>
void ReferenceCounted<Base>::LightningSetupType(LS::LibraryBuilder& builder, LS::BoundType* type)
{
  PlasmaBindHandle();
}

// Safe Id
template <typename idType, typename Base = EmptyClass>
class SafeId : public Base
{
public:
  LightningDeclareType(SafeId, TypeCopyMode::ReferenceType);
  DeclareSafeIdHandle(idType);

  SafeId()
  {
    ConstructSafeIdHandle();
  }

  SafeId(const SafeId&)
  {
    ConstructSafeIdHandle();
  }

  virtual ~SafeId()
  {
    DestructSafeIdHandle();
  }

  SafeId& operator=(const SafeId& rhs)
  {
    return *this;
  }
};

template <typename idType, typename Base>
typename SafeId<idType, Base>::HandleIdType SafeId<idType, Base>::mPlasmaHandleCurrentId = 1;
template <typename idType, typename Base>
HashMap<typename SafeId<idType, Base>::HandleIdType, SafeId<idType, Base>*>
    SafeId<idType, Base>::mPlasmaHandleLiveObjects;

template <typename idType, typename Base>
void SafeId<idType, Base>::LightningSetupType(LS::LibraryBuilder& builder, LS::BoundType* type)
{
  PlasmaBindHandle();
}

// Thread Safe Id
template <typename idType, typename Base = EmptyClass>
class ThreadSafeId : public Base
{
public:
  LightningDeclareType(ThreadSafeId, TypeCopyMode::ReferenceType);
  DeclareThreadSafeIdHandle(idType);

  ThreadSafeId()
  {
    ConstructThreadSafeIdHandle();
  }

  ThreadSafeId(const ThreadSafeId&)
  {
    ConstructThreadSafeIdHandle();
  }

  virtual ~ThreadSafeId()
  {
    DestructThreadSafeIdHandle();
  }

  ThreadSafeId& operator=(const ThreadSafeId& rhs)
  {
    return *this;
  }
};

template <typename idType, typename Base>
typename ThreadSafeId<idType, Base>::HandleIdType ThreadSafeId<idType, Base>::mPlasmaHandleCurrentId = 1;
template <typename idType, typename Base>
HashMap<typename ThreadSafeId<idType, Base>::HandleIdType, ThreadSafeId<idType, Base>*>
    ThreadSafeId<idType, Base>::mPlasmaHandleLiveObjects;
template <typename idType, typename Base>
ThreadLock ThreadSafeId<idType, Base>::mPlasmaHandleLock;

template <typename idType, typename Base>
void ThreadSafeId<idType, Base>::LightningSetupType(LS::LibraryBuilder& builder, LS::BoundType* type)
{
  PlasmaBindHandle();
}
// Reference Counted Safe Id
template <typename idType, typename Base = EmptyClass>
class ReferenceCountedSafeId : public Base
{
public:
  LightningDeclareType(ReferenceCountedSafeId, TypeCopyMode::ReferenceType);
  DeclareReferenceCountedSafeIdHandle(idType);

  ReferenceCountedSafeId()
  {
    ConstructReferenceCountedSafeIdHandle();
  }

  ReferenceCountedSafeId(const ReferenceCountedSafeId&)
  {
    ConstructReferenceCountedSafeIdHandle();
  }

  virtual ~ReferenceCountedSafeId()
  {
    DestructReferenceCountedSafeIdHandle();
  }

  ReferenceCountedSafeId& operator=(const ReferenceCountedSafeId& rhs)
  {
    return *this;
  }
};

template <typename idType, typename Base>
typename ReferenceCountedSafeId<idType, Base>::HandleIdType ReferenceCountedSafeId<idType, Base>::mPlasmaHandleCurrentId =
    1;
template <typename idType, typename Base>
HashMap<typename ReferenceCountedSafeId<idType, Base>::HandleIdType, ReferenceCountedSafeId<idType, Base>*>
    ReferenceCountedSafeId<idType, Base>::mPlasmaHandleLiveObjects;

template <typename idType, typename Base>
void ReferenceCountedSafeId<idType, Base>::LightningSetupType(LS::LibraryBuilder& builder, LS::BoundType* type)
{
  PlasmaBindHandle();
}
// Counted Thread Safe Id
template <typename idType, typename Base = EmptyClass>
class ReferenceCountedThreadSafeId : public Base
{
public:
  LightningDeclareType(ReferenceCountedThreadSafeId, TypeCopyMode::ReferenceType);
  DeclareReferenceCountedThreadSafeIdHandle(idType);

  ReferenceCountedThreadSafeId()
  {
    ConstructReferenceCountedThreadSafeIdHandle();
  }

  ReferenceCountedThreadSafeId(const ReferenceCountedThreadSafeId&)
  {
    ConstructReferenceCountedThreadSafeIdHandle();
  }

  virtual ~ReferenceCountedThreadSafeId()
  {
    DestructReferenceCountedThreadSafeIdHandle();
  }

  ReferenceCountedThreadSafeId& operator=(const ReferenceCountedThreadSafeId& rhs)
  {
    return *this;
  }
};

template <typename idType, typename Base>
typename ReferenceCountedThreadSafeId<idType, Base>::HandleIdType
    ReferenceCountedThreadSafeId<idType, Base>::mPlasmaHandleCurrentId = 1;
template <typename idType, typename Base>
HashMap<typename ReferenceCountedThreadSafeId<idType, Base>::HandleIdType, ReferenceCountedThreadSafeId<idType, Base>*>
    ReferenceCountedThreadSafeId<idType, Base>::mPlasmaHandleLiveObjects;
template <typename idType, typename Base>
ThreadLock ReferenceCountedThreadSafeId<idType, Base>::mPlasmaHandleLock;

template <typename idType, typename Base>
void ReferenceCountedThreadSafeId<idType, Base>::LightningSetupType(LS::LibraryBuilder& builder, LS::BoundType* type)
{
  PlasmaBindHandle();
}
} // namespace Plasma
