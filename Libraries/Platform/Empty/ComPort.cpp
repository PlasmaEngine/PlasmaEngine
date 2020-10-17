// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

ComPort::ComPort()
{
}

ComPort::~ComPort()
{
}

void ComPort::Close()
{
}

bool ComPort::Open(StringParam name, uint baudRate)
{
  return false;
}

uint ComPort::Read(byte* buffer, uint bytesToRead)
{
  return 0;
}

void ComPort::Write(byte* buffer, uint bytesToWrite)
{
}

} // namespace Plasma
