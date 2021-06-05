// MIT Licensed (see LICENSE.md).
#pragma once

// Standard Library Dependencies
#include "Core/Common/CommonStandard.hpp"
#include "PlatformStandard.hpp"
#include "Core/Meta/MetaStandard.hpp"
#include "Core/Support/SupportStandard.hpp"

// Lightning Library Dependencies
#include "Lightning/LightningCore/Precompiled.hpp"
using namespace Lightning;

namespace Plasma
{

// Networking library
class PlasmaNoImportExport NetworkingLibrary : public Lightning::StaticLibrary
{
public:
  LightningDeclareStaticLibraryInternals(NetworkingLibrary, "PlasmaEngine");

  static void Initialize();
  static void Shutdown();
};

} // namespace Plasma

// Extension Library Dependencies
#include "Networking/Replication/ReplicationStandard.hpp"

// Core Library Dependencies
#include "Core/Engine/EngineStandard.hpp"
#include "Physics/PhysicsStandard.hpp"

// Other Networking Includes
#include "SendableEvent.hpp"
#include "TcpSocket.hpp"
#include "SimpleSocket.hpp"
#include "IrcClient.hpp"
#include "WebRequester.hpp"
#include "WebServer.hpp"

// NetPeer Forward Declarations
namespace Plasma
{
class BitStreamExtended;
class EventBundle;
class NetHostRecord;
class FamilyTree;
class NetHost;
class NetProperty;
class NetPropertyType;
class NetPropertyConfig;
class NetPropertyConfigManager;
class NetPropertyInfo;
class NetChannel;
class NetChannelType;
class NetChannelConfig;
class NetChannelConfigManager;
class NetDiscoveryInterface;
class NetObject;
class NetUser;
struct PendingNetUser;
class NetSpace;
class NetPeer;
} // namespace Plasma

// NetPeer Includes
#include "BitStreamExtended.hpp"
#include "EventBundle.hpp"
#include "NetHostRecord.hpp"
#include "NetTypes.hpp"
#include "NetEvents.hpp"
#include "NetHost.hpp"
#include "NetProperty.hpp"
#include "NetChannel.hpp"
#include "NetObject.hpp"
#include "NetUser.hpp"
#include "NetSpace.hpp"
#include "NetPeerConnectionInterface.hpp"
#include "NetPeerMessageInterface.hpp"
#include "PendingHostPing.hpp"
#include "PingManager.hpp"
#include "NetDiscoveryInterface.hpp"
#include "InternetHostDiscovery.hpp"
#include "LanHostDiscovery.hpp"
#include "NetPeer.hpp"
#include "NetworkingBindingExtensions.hpp"
