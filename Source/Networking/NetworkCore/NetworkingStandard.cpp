// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

#define InitializePropertyFilterForType(typeName) LightningInitializeType(PropertyFilter##typeName)

// Ranges
LightningDefineRange(EventRange);
LightningDefineRange(NetUserRange);
LightningDefineRange(NetHostRange);
LightningDefineRange(WebServerHeaderRange);

// Enums
LightningDefineEnum(TcpSocketBind);
LightningDefineEnum(NetUserAddResponse);
LightningDefineEnum(Network);
LightningDefineEnum(NetRefreshResult);
LightningDefineEnum(Role);
LightningDefineEnum(Authority);
LightningDefineEnum(AuthorityMode);
LightningDefineEnum(DetectionMode);
LightningDefineEnum(ReliabilityMode);
LightningDefineEnum(SerializationMode);
LightningDefineEnum(RouteMode);
LightningDefineEnum(ReplicationPhase);
LightningDefineEnum(ConvergenceState);
LightningDefineEnum(BasicNetType);
LightningDefineEnum(TransportProtocol);
LightningDefineEnum(ConnectResponseMode);
LightningDefineEnum(TransmissionDirection);
LightningDefineEnum(LinkStatus);
LightningDefineEnum(LinkState);
LightningDefineEnum(ConnectResponse);
LightningDefineEnum(DisconnectReason);
LightningDefineEnum(UserConnectResponse);
LightningDefineEnum(TransferMode);
LightningDefineEnum(Receipt);
LightningDefineEnum(WebServerRequestMethod);

LightningDefineExternalBaseType(WebResponseCode::Enum, TypeCopyMode::ValueType, builder, type)
{
  LightningFullBindEnum(builder, type, SpecialType::Enumeration);
  LightningFullBindEnumValue(builder, type, WebResponseCode::Invalid, "Invalid");
  LightningFullBindEnumValue(builder, type, WebResponseCode::NoServerResponse, "NoServerResponse");
  LightningFullBindEnumValue(builder, type, WebResponseCode::Continue, "Continue");
  LightningFullBindEnumValue(builder, type, WebResponseCode::SwitchingProtocols, "SwitchingProtocols");
  LightningFullBindEnumValue(builder, type, WebResponseCode::OK, "OK");
  LightningFullBindEnumValue(builder, type, WebResponseCode::Created, "Created");
  LightningFullBindEnumValue(builder, type, WebResponseCode::Accepted, "Accepted");
  LightningFullBindEnumValue(builder, type, WebResponseCode::NonauthoritativeInformation, "NonauthoritativeInformation");
  LightningFullBindEnumValue(builder, type, WebResponseCode::NoContent, "NoContent");
  LightningFullBindEnumValue(builder, type, WebResponseCode::ResetContent, "ResetContent");
  LightningFullBindEnumValue(builder, type, WebResponseCode::PartialContent, "PartialContent");
  LightningFullBindEnumValue(builder, type, WebResponseCode::MovedPermanently, "MovedPermanently");
  LightningFullBindEnumValue(builder, type, WebResponseCode::ObjectMovedTemporarily, "ObjectMovedTemporarily");
  LightningFullBindEnumValue(builder, type, WebResponseCode::SeeOther, "SeeOther");
  LightningFullBindEnumValue(builder, type, WebResponseCode::NotModified, "NotModified");
  LightningFullBindEnumValue(builder, type, WebResponseCode::TemporaryRedirect, "TemporaryRedirect");
  LightningFullBindEnumValue(builder, type, WebResponseCode::PermanentRedirect, "PermanentRedirect");
  LightningFullBindEnumValue(builder, type, WebResponseCode::BadRequest, "BadRequest");
  LightningFullBindEnumValue(builder, type, WebResponseCode::AccessDenied, "AccessDenied");
  LightningFullBindEnumValue(builder, type, WebResponseCode::Forbidden, "Forbidden");
  LightningFullBindEnumValue(builder, type, WebResponseCode::NotFound, "NotFound");
  LightningFullBindEnumValue(builder, type, WebResponseCode::HTTPVerbNotAllowed, "HTTPVerbNotAllowed");
  LightningFullBindEnumValue(builder, type, WebResponseCode::ClientBrowserRejectsMIME, "ClientBrowserRejectsMIME");
  LightningFullBindEnumValue(builder, type, WebResponseCode::ProxyAuthenticationRequired, "ProxyAuthenticationRequired");
  LightningFullBindEnumValue(builder, type, WebResponseCode::PreconditionFailed, "PreconditionFailed");
  LightningFullBindEnumValue(builder, type, WebResponseCode::RequestEntityTooLarge, "RequestEntityTooLarge");
  LightningFullBindEnumValue(builder, type, WebResponseCode::RequestURITooLarge, "RequestURITooLarge");
  LightningFullBindEnumValue(builder, type, WebResponseCode::UnsupportedMediaType, "UnsupportedMediaType");
  LightningFullBindEnumValue(builder, type, WebResponseCode::RequestedRangeNotSatisfiable, "RequestedRangeNotSatisfiable");
  LightningFullBindEnumValue(builder, type, WebResponseCode::ExecutionFailed, "ExecutionFailed");
  LightningFullBindEnumValue(builder, type, WebResponseCode::LockedError, "LockedError");
  LightningFullBindEnumValue(builder, type, WebResponseCode::InternalServerError, "InternalServerError");
  LightningFullBindEnumValue(builder, type, WebResponseCode::UnimplementedHeaderValueUsed, "UnimplementedHeaderValueUsed");
  LightningFullBindEnumValue(builder, type, WebResponseCode::GatewayProxyReceivedInvalid, "GatewayProxyReceivedInvalid");
  LightningFullBindEnumValue(builder, type, WebResponseCode::ServiceUnavailable, "ServiceUnavailable");
  LightningFullBindEnumValue(builder, type, WebResponseCode::GatewayTimedOut, "GatewayTimedOut");
  LightningFullBindEnumValue(builder, type, WebResponseCode::HTTPVersionNotSupported, "HTTPVersionNotSupported");
}

// Arrays
PlasmaDefineArrayType(NetPropertyInfoArray);

LightningDefineStaticLibrary(NetworkingLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  LightningInitializeRange(EventRange);
  LightningInitializeRange(NetUserRange);
  LightningInitializeRange(NetHostRange);
  LightningInitializeRange(WebServerHeaderRange);

  // Enums
  LightningInitializeEnum(TcpSocketBind);
  LightningInitializeEnum(NetUserAddResponse);
  LightningInitializeEnum(Network);
  LightningInitializeEnum(NetRefreshResult);
  LightningInitializeEnumAs(Role, "NetRole");
  LightningInitializeEnum(Authority);
  LightningInitializeEnum(AuthorityMode);
  LightningInitializeEnum(DetectionMode);
  LightningInitializeEnum(ReliabilityMode);
  LightningInitializeEnum(SerializationMode);
  LightningInitializeEnum(RouteMode);
  LightningInitializeEnum(ReplicationPhase);
  LightningInitializeEnum(ConvergenceState);
  LightningInitializeEnum(BasicNetType);
  LightningInitializeEnum(TransportProtocol);
  LightningInitializeEnum(ConnectResponseMode);
  LightningInitializeEnum(TransmissionDirection);
  LightningInitializeEnum(LinkStatus);
  LightningInitializeEnum(LinkState);
  LightningInitializeEnum(ConnectResponse);
  LightningInitializeEnum(DisconnectReason);
  LightningInitializeEnum(UserConnectResponse);
  LightningInitializeEnum(TransferMode);
  LightningInitializeEnum(Receipt);
  LightningInitializeEnum(WebServerRequestMethod);
  LightningInitializeEnum(WebResponseCode);

  // Meta Arrays
  PlasmaInitializeArrayTypeAs(NetPropertyInfoArray, "NetPropertyInfos");

  // Events
  LightningInitializeType(ConnectionEvent);
  LightningInitializeType(ReceivedDataEvent);
  LightningInitializeType(SendableEvent);
  LightningInitializeType(WebResponseEvent);
  LightningInitializeType(WebServerRequestEvent);
  LightningInitializeType(AcquireNetHostInfo);
  LightningInitializeType(NetHostUpdate);
  LightningInitializeType(NetHostListUpdate);
  LightningInitializeType(NetPeerOpened);
  LightningInitializeType(NetPeerClosed);
  LightningInitializeType(NetGameStarted);
  LightningInitializeType(NetPeerSentConnectRequest);
  LightningInitializeType(NetPeerReceivedConnectRequest);
  LightningInitializeType(NetPeerSentConnectResponse);
  LightningInitializeType(NetPeerReceivedConnectResponse);
  LightningInitializeType(NetLinkConnected);
  LightningInitializeType(NetLinkDisconnected);
  LightningInitializeType(NetLevelStarted);
  LightningInitializeType(NetPeerSentUserAddRequest);
  LightningInitializeType(NetPeerReceivedUserAddRequest);
  LightningInitializeType(NetPeerSentUserAddResponse);
  LightningInitializeType(NetPeerReceivedUserAddResponse);
  LightningInitializeType(NetUserLostObjectOwnership);
  LightningInitializeType(NetUserAcquiredObjectOwnership);
  LightningInitializeType(RegisterCppNetProperties);
  LightningInitializeType(NetObjectOnline);
  LightningInitializeType(NetObjectOffline);
  LightningInitializeType(NetUserOwnerChanged);
  LightningInitializeType(NetChannelPropertyChange);
  LightningInitializeType(NetEventSent);
  LightningInitializeType(NetEventReceived);
  LightningInitializeType(NetHostRecordEvent);

  // Meta Components
  LightningInitializeType(EventBundleMetaComposition);
  LightningInitializeType(PropertyFilterMultiPrimitiveTypes);
  LightningInitializeType(PropertyFilterFloatingPointTypes);
  LightningInitializeType(PropertyFilterArithmeticTypes);
  LightningInitializeType(EditInGameFilter);
  LightningInitializeType(MetaNetProperty);

  // Net property filters by type
  InitializePropertyFilterForType(Other);
  InitializePropertyFilterForType(Boolean);
  InitializePropertyFilterForType(Integer);
  InitializePropertyFilterForType(DoubleInteger);
  InitializePropertyFilterForType(Integer2);
  InitializePropertyFilterForType(Integer3);
  InitializePropertyFilterForType(Integer4);
  InitializePropertyFilterForType(Real);
  InitializePropertyFilterForType(DoubleReal);
  InitializePropertyFilterForType(Real2);
  InitializePropertyFilterForType(Real3);
  InitializePropertyFilterForType(Real4);
  InitializePropertyFilterForType(Quaternion);
  InitializePropertyFilterForType(String);

  // Other Networking Type Initialization
  LightningInitializeType(TcpSocket);
  LightningInitializeType(SimpleSocket);
  LightningInitializeType(ConnectionData);
  LightningInitializeType(WebServer);
  LightningInitializeType(AsyncWebRequest);
  LightningInitializeType(WebRequester);

  // NetPeer Type Initialization
  LightningInitializeTypeAs(BitStreamExtended, "BitStream");
  LightningInitializeType(EventBundle);
  LightningInitializeType(NetPropertyInfo);
  LightningInitializeType(NetPropertyConfig);
  LightningInitializeType(NetPropertyType);
  LightningInitializeType(NetProperty);
  LightningInitializeType(NetChannelConfig);
  LightningInitializeType(NetChannelType);
  LightningInitializeType(NetChannel);
  LightningInitializeType(NetHost);
  LightningInitializeType(NetHostRecord);
  LightningInitializeType(NetObject);
  LightningInitializeType(NetSpace);
  LightningInitializeType(NetUser);
  LightningInitializeType(NetPeer);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void NetworkingLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  // Resource Managers
  InitializeResourceManager(NetChannelConfigManager);
  InitializeResourceManager(NetPropertyConfigManager);

  RegisterPropertyAttributeType(PropertyAttributes::cNetProperty, MetaNetProperty);
  RegisterPropertyAttribute(PropertyAttributes::cNetPeerId)->TypeMustBe(int);
}

void NetworkingLibrary::Shutdown()
{
  AsyncWebRequest::CancelAllActiveRequests();

  GetLibrary()->ClearComponents();
}

} // namespace Plasma
