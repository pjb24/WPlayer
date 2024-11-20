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

	e_result Create();
    e_result Close();
    e_result Bind(IPEndpoint endpoint);
    e_result Listen(IPEndpoint endpoint, int backlog);
    e_result Accept(Socket& outSocket, IPEndpoint* endpoint = nullptr);
    e_result Receive(void* destination, int numberOfBytes, int& bytesReceived);
    e_result ReceiveAll(void* data, int numberOfBytes);
    e_result Receive(Packet& packet);

    e_result Connect(IPEndpoint endpoint);
    e_result Send(const void* data, int numberOfBytes, int& bytesSent);
    e_result SendAll(const void* data, int numberOfBytes);
    e_result Send(Packet& packet);

	SocketHandle GetHandle();
	IPVersion GetIPVersion();
    e_result SetBlocking(bool isBlocking);

private:
    e_result setSocketOption(e_socket_option option, int value);

	IPVersion m_ipversion = IPVersion::IPv4;
	SocketHandle m_handle = INVALID_SOCKET;
};

#endif // !SOCKET_H
