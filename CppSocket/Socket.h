#ifndef SOCKET_H
#define SOCKET_H

#include "framework.h"
#include "Network.h"
#include "SocketHandle.h"
#include "Result.h"
#include "IPVersion.h"
#include "SocketOption.h"
#include "IPEndpoint.h"
#include "Constants.h"
#include "Packet.h"

class Socket
{
public:
	Socket(IPVersion ipversion = IPVersion::IPv4
		, SocketHandle handle = INVALID_SOCKET);

	Result Create();
	Result Close();
	Result Bind(IPEndpoint endpoint);
	Result Listen(IPEndpoint endpoint, int backlog);
	Result Accept(Socket& outSocket, IPEndpoint* endpoint = nullptr);
	Result Receive(void* destination, int numberOfBytes, int& bytesReceived);
	Result ReceiveAll(void* data, int numberOfBytes);
	Result Receive(Packet& packet);

	Result Connect(IPEndpoint endpoint);
	Result Send(const void* data, int numberOfBytes, int& bytesSent);
	Result SendAll(const void* data, int numberOfBytes);
	Result Send(Packet& packet);

	SocketHandle GetHandle();
	IPVersion GetIPVersion();
	Result SetBlocking(bool isBlocking);

private:
	Result setSocketOption(SocketOption option, int value);

	IPVersion m_ipversion = IPVersion::IPv4;
	SocketHandle m_handle = INVALID_SOCKET;
};

#endif // !SOCKET_H
