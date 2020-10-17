// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// Basic handles
// DefineReferenceCountedHandle(ReferenceCountedEmpty);
// LightningDefineType(ReferenceCountedEmpty, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineSafeIdHandle(SafeId32);
// LightningDefineType(SafeId32, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineSafeIdHandle(SafeId64);
// LightningDefineType(SafeId64, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineThreadSafeIdHandle(ThreadSafeId32);
// LightningDefineType(ThreadSafeId32, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineThreadSafeIdHandle(ThreadSafeId64);
// LightningDefineType(ThreadSafeId64, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineReferenceCountedSafeIdHandle(ReferenceCountedSafeId32);
// LightningDefineType(ReferenceCountedSafeId32, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineReferenceCountedSafeIdHandle(ReferenceCountedSafeId64);
// LightningDefineType(ReferenceCountedSafeId64, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineReferenceCountedThreadSafeIdHandle(ReferenceCountedThreadSafeId32);
// LightningDefineType(ReferenceCountedThreadSafeId32, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineReferenceCountedThreadSafeIdHandle(ReferenceCountedThreadSafeId64);
// LightningDefineType(ReferenceCountedThreadSafeId64, builder, type)
//{
//  PlasmaBindHandle();
//}
//
/// Object handles
// DefineReferenceCountedHandle(ReferenceCountedObject);
// LightningDefineType(ReferenceCountedObject, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineSafeIdHandle(SafeId32Object);
// LightningDefineType(SafeId32Object, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineSafeIdHandle(SafeId64Object);
// LightningDefineType(SafeId64Object, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineThreadSafeIdHandle(ThreadSafeId32Object);
// LightningDefineType(ThreadSafeId32Object, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineThreadSafeIdHandle(ThreadSafeId64Object);
// LightningDefineType(ThreadSafeId64Object, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineReferenceCountedSafeIdHandle(ReferenceCountedSafeId32Object);
// LightningDefineType(ReferenceCountedSafeId32Object, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineReferenceCountedSafeIdHandle(ReferenceCountedSafeId64Object);
// LightningDefineType(ReferenceCountedSafeId64Object, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineReferenceCountedThreadSafeIdHandle(ReferenceCountedThreadSafeId32Object);
// LightningDefineType(ReferenceCountedThreadSafeId32Object, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineReferenceCountedThreadSafeIdHandle(ReferenceCountedThreadSafeId64Object);
// LightningDefineType(ReferenceCountedThreadSafeId64Object, builder, type)
//{
//  PlasmaBindHandle();
//}
//
/// EventObject handles
// DefineReferenceCountedHandle(ReferenceCountedEventObject);
// LightningDefineType(ReferenceCountedEventObject, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineSafeIdHandle(SafeId32EventObject);
// LightningDefineType(SafeId32EventObject, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineSafeIdHandle(SafeId64EventObject);
// LightningDefineType(SafeId64EventObject, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineThreadSafeIdHandle(ThreadSafeId32EventObject);
// LightningDefineType(ThreadSafeId32EventObject, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineThreadSafeIdHandle(ThreadSafeId64EventObject);
// LightningDefineType(ThreadSafeId64EventObject, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineReferenceCountedSafeIdHandle(ReferenceCountedSafeId32EventObject);
// LightningDefineType(ReferenceCountedSafeId32EventObject, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineReferenceCountedSafeIdHandle(ReferenceCountedSafeId64EventObject);
// LightningDefineType(ReferenceCountedSafeId64EventObject, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineReferenceCountedThreadSafeIdHandle(ReferenceCountedThreadSafeId32EventObject);
// LightningDefineType(ReferenceCountedThreadSafeId32EventObject, builder, type)
//{
//  PlasmaBindHandle();
//}
//
// DefineReferenceCountedThreadSafeIdHandle(ReferenceCountedThreadSafeId64EventObject);
// LightningDefineType(ReferenceCountedThreadSafeId64EventObject, builder, type)
//{
//  PlasmaBindHandle();
//}

void RegisterCommonHandleManagers()
{
  // Basic handles
  PlasmaRegisterHandleManager(ReferenceCountedEmpty);
  PlasmaRegisterHandleManager(SafeId32);
  PlasmaRegisterHandleManager(SafeId64);
  PlasmaRegisterHandleManager(ThreadSafeId32);
  PlasmaRegisterHandleManager(ThreadSafeId64);
  PlasmaRegisterHandleManager(ReferenceCountedSafeId32);
  PlasmaRegisterHandleManager(ReferenceCountedSafeId64);
  PlasmaRegisterHandleManager(ReferenceCountedThreadSafeId32);
  PlasmaRegisterHandleManager(ReferenceCountedThreadSafeId64);

  // Object handles
  PlasmaRegisterHandleManager(ReferenceCountedObject);
  PlasmaRegisterHandleManager(SafeId32Object);
  PlasmaRegisterHandleManager(SafeId64Object);
  PlasmaRegisterHandleManager(ThreadSafeId32Object);
  PlasmaRegisterHandleManager(ThreadSafeId64Object);
  PlasmaRegisterHandleManager(ReferenceCountedSafeId32Object);
  PlasmaRegisterHandleManager(ReferenceCountedSafeId64Object);
  PlasmaRegisterHandleManager(ReferenceCountedThreadSafeId32Object);
  PlasmaRegisterHandleManager(ReferenceCountedThreadSafeId64Object);

  // EventObject handles
  PlasmaRegisterHandleManager(ReferenceCountedEventObject);
  PlasmaRegisterHandleManager(SafeId32EventObject);
  PlasmaRegisterHandleManager(SafeId64EventObject);
  PlasmaRegisterHandleManager(ThreadSafeId32EventObject);
  PlasmaRegisterHandleManager(ThreadSafeId64EventObject);
  PlasmaRegisterHandleManager(ReferenceCountedSafeId32EventObject);
  PlasmaRegisterHandleManager(ReferenceCountedSafeId64EventObject);
  PlasmaRegisterHandleManager(ReferenceCountedThreadSafeId32EventObject);
  PlasmaRegisterHandleManager(ReferenceCountedThreadSafeId64EventObject);
}

} // namespace Plasma
