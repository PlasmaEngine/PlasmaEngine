// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
//
// NetHost Events
//

// Host Info:
DefineEvent(AcquireBasicNetHostInfo);
DefineEvent(AcquireExtraNetHostInfo);

// Host Discovery:
DefineEvent(NetHostDiscovered);
DefineEvent(NetHostListDiscovered);
DefineEvent(NetHostRefreshed);
DefineEvent(NetHostListRefreshed);

//
// NetPeer Events
//

// Peer Status:
DefineEvent(NetPeerOpened);
DefineEvent(NetPeerClosed);

// Game Scope:
DefineEvent(NetGameOnline);
DefineEvent(NetGameOffline);

// Game State:
DefineEvent(NetGameStarted);

//
// NetLink Events
//

// Link Handshake Sequence:
DefineEvent(NetPeerSentConnectRequest);
DefineEvent(NetPeerReceivedConnectRequest);
DefineEvent(NetPeerSentConnectResponse);
DefineEvent(NetPeerReceivedConnectResponse);

// Link Status:
DefineEvent(NetLinkConnected);
DefineEvent(NetLinkDisconnected);

//
// NetSpace Events
//

// Space Scope:
DefineEvent(NetSpaceOnline);
DefineEvent(NetSpaceOffline);

// Level State:
DefineEvent(NetLevelStarted);

//
// NetUser Events
//

// User Add Handshake Sequence:
DefineEvent(NetPeerSentUserAddRequest);
DefineEvent(NetPeerReceivedUserAddRequest);
DefineEvent(NetPeerSentUserAddResponse);
DefineEvent(NetPeerReceivedUserAddResponse);

// User Scope:
DefineEvent(NetUserOnline);
DefineEvent(NetUserOffline);

// Network Ownership:
DefineEvent(NetUserLostObjectOwnership);
DefineEvent(NetUserAcquiredObjectOwnership);

//
// NetObject Events
//

// Object Initialization:
DefineEvent(RegisterCppNetProperties);

// Object Scope:
DefineEvent(NetObjectOnline);
DefineEvent(NetObjectOffline);

// Network Ownership:
DefineEvent(NetUserOwnerChanged);

// Network Channel Property Change:
DefineEvent(NetChannelOutgoingPropertyInitialized);
DefineEvent(NetChannelIncomingPropertyInitialized);
DefineEvent(NetChannelOutgoingPropertyUninitialized);
DefineEvent(NetChannelIncomingPropertyUninitialized);
DefineEvent(NetChannelOutgoingPropertyChanged);
DefineEvent(NetChannelIncomingPropertyChanged);

//
// NetEvent Events
//

// Event Handling:
DefineEvent(NetEventSent);
DefineEvent(NetEventReceived);

//
// Master Server
//

DefineEvent(NetHostRecordDiscovered);
DefineEvent(NetHostRecordUpdate);
DefineEvent(NetHostRecordExpired);
} // namespace Events

void BindNetEvents(LibraryBuilder& builder, BoundType* type)
{
  //
  // NetHost Events
  //

  // Host Info:
  PlasmaBindEvent(Events::AcquireBasicNetHostInfo, AcquireNetHostInfo);
  PlasmaBindEvent(Events::AcquireExtraNetHostInfo, AcquireNetHostInfo);

  // Host Discovery:
  PlasmaBindEvent(Events::NetHostDiscovered, NetHostUpdate);
  PlasmaBindEvent(Events::NetHostListDiscovered, NetHostListUpdate);
  PlasmaBindEvent(Events::NetHostRefreshed, NetHostUpdate);
  PlasmaBindEvent(Events::NetHostListRefreshed, NetHostListUpdate);

  //
  // NetPeer Events
  //

  // Peer Status:
  PlasmaBindEvent(Events::NetPeerOpened, NetPeerOpened);
  PlasmaBindEvent(Events::NetPeerClosed, NetPeerClosed);

  // Game Scope:
  PlasmaBindEvent(Events::NetGameOnline, NetObjectOnline);
  PlasmaBindEvent(Events::NetGameOffline, NetObjectOffline);

  // Game State:
  PlasmaBindEvent(Events::NetGameStarted, NetGameStarted);

  //
  // NetLink Events
  //

  // Link Handshake Sequence:
  PlasmaBindEvent(Events::NetPeerSentConnectRequest, NetPeerSentConnectRequest);
  PlasmaBindEvent(Events::NetPeerReceivedConnectRequest, NetPeerReceivedConnectRequest);
  PlasmaBindEvent(Events::NetPeerSentConnectResponse, NetPeerSentConnectResponse);
  PlasmaBindEvent(Events::NetPeerReceivedConnectResponse, NetPeerReceivedConnectResponse);

  // Link Status:
  PlasmaBindEvent(Events::NetLinkConnected, NetLinkConnected);
  PlasmaBindEvent(Events::NetLinkDisconnected, NetLinkDisconnected);

  //
  // NetSpace Events
  //

  // Space Scope:
  PlasmaBindEvent(Events::NetSpaceOnline, NetObjectOnline);
  PlasmaBindEvent(Events::NetSpaceOffline, NetObjectOffline);

  // Level State:
  PlasmaBindEvent(Events::NetLevelStarted, NetLevelStarted);

  //
  // NetUser Events
  //

  // User Add Handshake Sequence:
  PlasmaBindEvent(Events::NetPeerSentUserAddRequest, NetPeerSentUserAddRequest);
  PlasmaBindEvent(Events::NetPeerReceivedUserAddRequest, NetPeerReceivedUserAddRequest);
  PlasmaBindEvent(Events::NetPeerSentUserAddResponse, NetPeerSentUserAddResponse);
  PlasmaBindEvent(Events::NetPeerReceivedUserAddResponse, NetPeerReceivedUserAddResponse);

  // User Scope:
  PlasmaBindEvent(Events::NetUserOnline, NetObjectOnline);
  PlasmaBindEvent(Events::NetUserOffline, NetObjectOffline);

  // Network Ownership:
  PlasmaBindEvent(Events::NetUserLostObjectOwnership, NetUserLostObjectOwnership);
  PlasmaBindEvent(Events::NetUserAcquiredObjectOwnership, NetUserAcquiredObjectOwnership);

  //
  // NetObject Events
  //

  // Object Scope:
  PlasmaBindEvent(Events::NetObjectOnline, NetObjectOnline);
  PlasmaBindEvent(Events::NetObjectOffline, NetObjectOffline);

  // Network Ownership:
  PlasmaBindEvent(Events::NetUserOwnerChanged, NetUserOwnerChanged);

  // Network Channel Property Change:
  PlasmaBindEvent(Events::NetChannelOutgoingPropertyInitialized, NetChannelPropertyChange);
  PlasmaBindEvent(Events::NetChannelIncomingPropertyInitialized, NetChannelPropertyChange);
  PlasmaBindEvent(Events::NetChannelOutgoingPropertyUninitialized, NetChannelPropertyChange);
  PlasmaBindEvent(Events::NetChannelIncomingPropertyUninitialized, NetChannelPropertyChange);
  PlasmaBindEvent(Events::NetChannelOutgoingPropertyChanged, NetChannelPropertyChange);
  PlasmaBindEvent(Events::NetChannelIncomingPropertyChanged, NetChannelPropertyChange);

  //
  // NetEvent Events
  //

  // Event Handling:
  PlasmaBindEvent(Events::NetEventSent, NetEventSent);
  PlasmaBindEvent(Events::NetEventReceived, NetEventReceived);

  //
  // MasterServer Events
  //

  PlasmaBindEvent(Events::NetHostRecordDiscovered, NetHostRecordEvent);
  PlasmaBindEvent(Events::NetHostRecordUpdate, NetHostRecordEvent);
  PlasmaBindEvent(Events::NetHostRecordExpired, NetHostRecordEvent);
}

//                                  NetRequest //

NetRequest::NetRequest(NetRequestType::Enum netRequestType,
                       const IpAddress& ipAddress,
                       const EventBundle& requestBundle) :
    mNetRequestType(netRequestType),
    mTheirIpAddress(ipAddress),
    mOurRequestBundle(requestBundle)
{
}

//                                NetHost Events //

/////////////////
//  Host Info  //
/////////////////

//                              AcquireNetHostInfo //

LightningDefineType(AcquireNetHostInfo, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldProperty(mReturnHostInfo);
}

AcquireNetHostInfo::AcquireNetHostInfo(GameSession* gameSession) : mReturnHostInfo(gameSession)
{
}

//////////////////////
//  Host Discovery  //
//////////////////////

//                                 NetHostUpdate //

LightningDefineType(NetHostUpdate, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mRefreshResult);
  LightningBindFieldGetterProperty(mResponseTime);
  LightningBindFieldGetterProperty(mNetwork);
  LightningBindFieldGetterProperty(mHost);
}

NetHostUpdate::NetHostUpdate() :
    mRefreshResult(NetRefreshResult::NoResponse),
    mResponseTime(0),
    mNetwork(Network::LAN),
    mHost(nullptr)
{
}

//                                 NetHostListUpdate //

LightningDefineType(NetHostListUpdate, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mNetwork);
}

NetHostListUpdate::NetHostListUpdate() : mNetwork(Network::LAN)
{
}

//                                NetPeer Events //

////////////////
// Peer Scope //
////////////////

//                                NetPeerOpened //

LightningDefineType(NetPeerOpened, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();
}

//                                 NetPeerClosed //

LightningDefineType(NetPeerClosed, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();
}

////////////////
// Game State //
////////////////

//                                 NetGameStarted //

LightningDefineType(NetGameStarted, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mGameSession);
}

NetGameStarted::NetGameStarted() : mGameSession(nullptr)
{
}

//                                NetLink Events //

/////////////////////////////
// Link Handshake Sequence //
/////////////////////////////

//                           NetPeerSentConnectRequest //

LightningDefineType(NetPeerSentConnectRequest, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mTheirIpAddress);
  LightningBindFieldGetterProperty(mOurRequestBundle);
  LightningBindFieldGetterProperty(mOurPendingUserAddRequestCount);
}

NetPeerSentConnectRequest::NetPeerSentConnectRequest(GameSession* gameSession) :
    mTheirIpAddress(),
    mOurRequestBundle(gameSession),
    mOurPendingUserAddRequestCount(0)
{
}

//                         NetPeerReceivedConnectRequest //

LightningDefineType(NetPeerReceivedConnectRequest, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mTheirIpAddress);
  LightningBindFieldGetterProperty(mTheirRequestBundle);
  LightningBindFieldGetterProperty(mTheirPendingUserAddRequestCount);
  LightningBindFieldGetterProperty(mOurIpAddress);
  LightningBindFieldProperty(mReturnOurConnectResponse);
  LightningBindFieldProperty(mReturnOurResponseBundle);
}

NetPeerReceivedConnectRequest::NetPeerReceivedConnectRequest(GameSession* gameSession) :
    mTheirIpAddress(),
    mTheirRequestBundle(gameSession),
    mTheirPendingUserAddRequestCount(0),
    mOurIpAddress(),
    mReturnOurConnectResponse(false),
    mReturnOurResponseBundle(gameSession)
{
}

//                           NetPeerSentConnectResponse //

LightningDefineType(NetPeerSentConnectResponse, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mTheirNetPeerId);
  LightningBindFieldGetterProperty(mTheirIpAddress);
  LightningBindFieldGetterProperty(mTheirRequestBundle);
  LightningBindFieldGetterProperty(mTheirPendingUserAddRequestCount);
  LightningBindFieldGetterProperty(mOurIpAddress);
  LightningBindFieldGetterProperty(mOurConnectResponse);
  LightningBindFieldGetterProperty(mOurResponseBundle);
}

NetPeerSentConnectResponse::NetPeerSentConnectResponse(GameSession* gameSession) :
    mTheirNetPeerId(0),
    mTheirIpAddress(),
    mTheirRequestBundle(gameSession),
    mTheirPendingUserAddRequestCount(0),
    mOurIpAddress(),
    mOurConnectResponse(ConnectResponse::Deny),
    mOurResponseBundle(gameSession)
{
}

//                        NetPeerReceivedConnectResponse //

LightningDefineType(NetPeerReceivedConnectResponse, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mTheirIpAddress);
  LightningBindFieldGetterProperty(mOurRequestBundle);
  LightningBindFieldGetterProperty(mOurPendingUserAddRequestCount);
  LightningBindFieldGetterProperty(mOurIpAddress);
  LightningBindFieldGetterProperty(mTheirConnectResponse);
  LightningBindFieldGetterProperty(mTheirResponseBundle);
  LightningBindFieldGetterProperty(mOurNetPeerId);
}

NetPeerReceivedConnectResponse::NetPeerReceivedConnectResponse(GameSession* gameSession) :
    mTheirIpAddress(),
    mOurRequestBundle(gameSession),
    mOurPendingUserAddRequestCount(0),
    mOurIpAddress(),
    mTheirConnectResponse(ConnectResponse::Deny),
    mTheirResponseBundle(gameSession),
    mOurNetPeerId(0)
{
}

////////////////
// Link Scope //
////////////////

//                                NetLinkConnected //

LightningDefineType(NetLinkConnected, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mTheirNetPeerId);
  LightningBindFieldGetterProperty(mTheirIpAddress);
  LightningBindFieldGetterProperty(mDirection);
}

NetLinkConnected::NetLinkConnected() :
    mTheirNetPeerId(0),
    mTheirIpAddress(),
    mDirection(TransmissionDirection::Unspecified)
{
}

//                              NetLinkDisconnected //

LightningDefineType(NetLinkDisconnected, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mTheirNetPeerId);
  LightningBindFieldGetterProperty(mTheirIpAddress);
  LightningBindFieldGetterProperty(mDisconnectReason);
  LightningBindFieldGetterProperty(mRequestBundle);
  LightningBindFieldGetterProperty(mDirection);
}

NetLinkDisconnected::NetLinkDisconnected(GameSession* gameSession) :
    mTheirNetPeerId(0),
    mTheirIpAddress(),
    mDisconnectReason(DisconnectReason::Request),
    mRequestBundle(gameSession),
    mDirection(TransmissionDirection::Unspecified)
{
}

//                               NetSpace Events //

/////////////////
// Level State //
/////////////////

//                                NetLevelStarted //

LightningDefineType(NetLevelStarted, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mGameSession);
  LightningBindFieldGetterProperty(mSpace);
  LightningBindFieldGetterProperty(mLevelName);
}

NetLevelStarted::NetLevelStarted() : mGameSession(nullptr), mSpace(nullptr), mLevelName()
{
}

//                                NetUser Events //

///////////////////////////////////
//  User Add Handshake Sequence  //
///////////////////////////////////

//                          NetPeerSentUserAddRequest //

LightningDefineType(NetPeerSentUserAddRequest, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mTheirNetPeerId);
  LightningBindFieldGetterProperty(mTheirIpAddress);
  LightningBindFieldGetterProperty(mOurRequestBundle);
}

NetPeerSentUserAddRequest::NetPeerSentUserAddRequest(GameSession* gameSession) :
    mTheirNetPeerId(0),
    mTheirIpAddress(),
    mOurRequestBundle(gameSession)
{
}

//                        NetPeerReceivedUserAddRequest //

LightningDefineType(NetPeerReceivedUserAddRequest, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mTheirNetPeerId);
  LightningBindFieldGetterProperty(mTheirIpAddress);
  LightningBindFieldGetterProperty(mTheirRequestBundle);
  LightningBindFieldProperty(mReturnOurAddResponse);
  LightningBindFieldProperty(mReturnOurResponseBundle);
  LightningBindFieldProperty(mReturnTheirNetUser);
  LightningBindFieldGetterProperty(mTheirNetUserId);
}

NetPeerReceivedUserAddRequest::NetPeerReceivedUserAddRequest(GameSession* gameSession) :
    mTheirNetPeerId(0),
    mTheirIpAddress(),
    mTheirRequestBundle(gameSession),
    mReturnOurAddResponse(false),
    mReturnOurResponseBundle(gameSession),
    mReturnTheirNetUser(nullptr),
    mTheirNetUserId(0)
{
}

//                          NetPeerSentUserAddResponse //

LightningDefineType(NetPeerSentUserAddResponse, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mTheirNetPeerId);
  LightningBindFieldGetterProperty(mTheirIpAddress);
  LightningBindFieldGetterProperty(mTheirRequestBundle);
  LightningBindFieldGetterProperty(mOurAddResponse);
  LightningBindFieldGetterProperty(mOurResponseBundle);
  LightningBindFieldGetterProperty(mTheirNetUserId);
  LightningBindFieldGetterProperty(mTheirNetUser);
}

/// Constructor.
NetPeerSentUserAddResponse::NetPeerSentUserAddResponse(GameSession* gameSession) :
    mTheirNetPeerId(0),
    mTheirIpAddress(),
    mTheirRequestBundle(gameSession),
    mOurAddResponse(NetUserAddResponse::Deny),
    mOurResponseBundle(gameSession),
    mTheirNetUserId(0),
    mTheirNetUser(nullptr)
{
}

//                        NetPeerReceivedUserAddResponse //

LightningDefineType(NetPeerReceivedUserAddResponse, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mTheirNetPeerId);
  LightningBindFieldGetterProperty(mTheirIpAddress);
  LightningBindFieldGetterProperty(mOurRequestBundle);
  LightningBindFieldGetterProperty(mTheirAddResponse);
  LightningBindFieldGetterProperty(mTheirResponseBundle);
  LightningBindFieldGetterProperty(mOurNetUserId);
}

NetPeerReceivedUserAddResponse::NetPeerReceivedUserAddResponse(GameSession* gameSession) :
    mTheirNetPeerId(0),
    mTheirIpAddress(),
    mOurRequestBundle(gameSession),
    mTheirAddResponse(NetUserAddResponse::Deny),
    mTheirResponseBundle(gameSession),
    mOurNetUserId(0)
{
}

///////////////////////
// Network Ownership //
///////////////////////

//                          NetUserLostObjectOwnership //

LightningDefineType(NetUserLostObjectOwnership, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mLostObject);
  LightningBindFieldGetterProperty(mCurrentNetUserOwner);
}

//                        NetUserAcquiredObjectOwnership //

LightningDefineType(NetUserAcquiredObjectOwnership, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mAcquiredObject);
  LightningBindFieldGetterProperty(mPreviousNetUserOwner);
}

//                               NetObject Events //

///////////////////////////
// Object Initialization //
///////////////////////////

//                           RegisterCppNetProperties //

LightningDefineType(RegisterCppNetProperties, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();
}

//////////////////
// Object Scope //
//////////////////

//                                 NetObjectOnline //

LightningDefineType(NetObjectOnline, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mGameSession);
  LightningBindFieldGetterProperty(mSpace);
  LightningBindFieldGetterProperty(mObject);
  LightningBindFieldGetterProperty(mIsStartOfLifespan);
}

//                               NetObjectOffline //

LightningDefineType(NetObjectOffline, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mGameSession);
  LightningBindFieldGetterProperty(mSpace);
  LightningBindFieldGetterProperty(mObject);
  LightningBindFieldGetterProperty(mIsEndOfLifespan);
}

///////////////////////
// Network Ownership //
///////////////////////

//                             NetUserOwnerChanged //

LightningDefineType(NetUserOwnerChanged, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mPreviousNetUserOwner);
  LightningBindFieldGetterProperty(mCurrentNetUserOwner);
}

/////////////////////////////////////
// Network Channel Property Change //
/////////////////////////////////////

//                           NetChannelPropertyChange //

LightningDefineType(NetChannelPropertyChange, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mTimestamp);
  LightningBindFieldGetterProperty(mReplicationPhase);
  LightningBindFieldGetterProperty(mDirection);
  LightningBindFieldGetterProperty(mObject);
  LightningBindFieldGetterProperty(mChannelName);
  LightningBindFieldGetterProperty(mComponentName);
  LightningBindFieldGetterProperty(mPropertyName);
}

//                                NetEvent Events //

////////////////////
// Event Handling //
////////////////////

//                                 NetEventSent //

LightningDefineType(NetEventSent, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mTheirNetPeerId);
  LightningBindFieldGetterProperty(mNetEvent);
  LightningBindFieldGetterProperty(mDestination);
}

//                               NetEventReceived //

LightningDefineType(NetEventReceived, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mTheirNetPeerId);
  LightningBindFieldGetterProperty(mNetEvent);
  LightningBindFieldGetterProperty(mDestination);
  LightningBindFieldProperty(mReturnAllow);
}

//                                 NetHostUpdate //

LightningDefineType(NetHostRecordEvent, builder, type)
{
  // Bind documentation
  PlasmaBindDocumented();

  // Bind properties
  LightningBindFieldGetterProperty(mHostRecord);
}

NetHostRecordEvent::NetHostRecordEvent() : mHostRecord(nullptr)
{
}

NetHostRecordEvent::NetHostRecordEvent(NetHostRecord* netHostRecord) : mHostRecord(netHostRecord)
{
}

} // namespace Plasma
