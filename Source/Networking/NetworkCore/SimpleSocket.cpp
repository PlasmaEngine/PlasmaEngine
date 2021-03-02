// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(SimpleSocket, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::DefaultSerialization);

  LightningBindGetterProperty(Socket);
}

SimpleSocket::SimpleSocket() : mSocket(Protocol::Chunks | Protocol::Events, "SimpleSocket")
{
}

TcpSocket* SimpleSocket::GetSocket()
{
  return &mSocket;
}

} // namespace Plasma
