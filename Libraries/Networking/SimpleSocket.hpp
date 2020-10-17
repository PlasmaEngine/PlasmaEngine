// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class TcpSocket;

class SimpleSocket : public Component
{
public:
  LightningDeclareType(SimpleSocket, TypeCopyMode::ReferenceType);

  /// Constructor.
  SimpleSocket();

  /// Returns the socket.
  TcpSocket* GetSocket();

private:
  /// Socket.
  TcpSocket mSocket;
};

} // namespace Plasma
