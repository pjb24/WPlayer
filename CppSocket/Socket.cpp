#include "Socket.h"
#include <assert.h>

Socket::Socket(IPVersion ipversion, SocketHandle handle)
	:m_ipversion(ipversion)
	, m_handle(handle)
{
	assert(ipversion == IPVersion::IPv4 || ipversion == IPVersion::IPv6);
}

Result Socket::Create()
{
	assert(m_ipversion == IPVersion::IPv4 || m_ipversion == IPVersion::IPv6);

	if (m_handle != INVALID_SOCKET)
	{
		return Result::NotYetImplemented;
	}

	m_handle = socket((m_ipversion == IPVersion::IPv4) ? AF_INET : AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (m_handle == INVALID_SOCKET)
	{
		int error = WSAGetLastError();

		return Result::NotYetImplemented;
	}

	if (setSocketOption(SocketOption::TCP_NoDelay, TRUE) != Result::Success)
	{
		return Result::NotYetImplemented;
	}

	return Result::Success;
}

Result Socket::Close()
{
	if (m_handle == INVALID_SOCKET)
	{
		return Result::NotYetImplemented;
	}

	int result = closesocket(m_handle);
	if (result != 0)
	{
		int error = WSAGetLastError();

		return Result::NotYetImplemented;
	}

	m_handle = INVALID_SOCKET;

	return Result::Success;
}

Result Socket::Bind(IPEndpoint endpoint)
{
	assert(m_ipversion == IPVersion::IPv4 || m_ipversion == IPVersion::IPv6);

	int result = 0;

	if (m_ipversion == IPVersion::IPv4)
	{
		sockaddr_in addr = endpoint.GetSockaddrIPv4();
		result = bind(m_handle, (sockaddr*)&addr, sizeof(sockaddr_in));
	}
	else
	{
		sockaddr_in6 addr = endpoint.GetSockaddrIPv6();
		result = bind(m_handle, (sockaddr*)&addr, sizeof(sockaddr_in6));
	}

	if (result != 0)
	{
		int error = WSAGetLastError();

		return Result::NotYetImplemented;
	}

	return Result::Success;
}

Result Socket::Listen(IPEndpoint endpoint, int backlog)
{
	if (m_ipversion == IPVersion::IPv6)
	{
		if (setSocketOption(SocketOption::IPV6_Only, FALSE) != Result::Success)
		{
			return Result::NotYetImplemented;
		}
	}

	if (Bind(endpoint) != Result::Success)
	{
		return Result::NotYetImplemented;
	}

	int result = listen(m_handle, backlog);
	if (result != 0)
	{
		int error = WSAGetLastError();

		return Result::NotYetImplemented;
	}

	return Result::Success;
}

Result Socket::Accept(Socket& outSocket, IPEndpoint* endpoint)
{
	assert(m_ipversion == IPVersion::IPv4 || m_ipversion == IPVersion::IPv6);

	if (m_ipversion == IPVersion::IPv4)
	{
		sockaddr_in addr = {};
		int length = sizeof(sockaddr_in);

		SocketHandle acceptedConnectionHandle = accept(m_handle, (sockaddr*)&addr, &length);
		if (acceptedConnectionHandle == INVALID_SOCKET)
		{
			int error = WSAGetLastError();

			return Result::NotYetImplemented;
		}

		if (endpoint != nullptr)
		{
			*endpoint = IPEndpoint((sockaddr*)&addr);
		}

		outSocket = Socket(IPVersion::IPv4, acceptedConnectionHandle);
	}
	else
	{
		sockaddr_in6 addr = {};
		int length = sizeof(sockaddr_in6);

		SocketHandle acceptedConnectionHandle = accept(m_handle, (sockaddr*)&addr, &length);
		if (acceptedConnectionHandle == INVALID_SOCKET)
		{
			int error = WSAGetLastError();

			return Result::NotYetImplemented;
		}

		if (endpoint != nullptr)
		{
			*endpoint = IPEndpoint((sockaddr*)&addr);
		}

		outSocket = Socket(IPVersion::IPv6, acceptedConnectionHandle);
	}

	return Result::Success;
}

Result Socket::Receive(void* destination, int numberOfBytes, int& bytesReceived)
{
	bytesReceived = recv(m_handle, (char*)destination, numberOfBytes, NULL);
	if (bytesReceived == 0)
	{
		return Result::NotYetImplemented;
	}
	
	if (bytesReceived == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		return Result::NotYetImplemented;
	}

	return Result::Success;
}

Result Socket::ReceiveAll(void* destination, int numberOfBytes)
{
	int totalBytesReceived = 0;

	while (totalBytesReceived < numberOfBytes)
	{
		int bytesRemaining = numberOfBytes - totalBytesReceived;
		int bytesReceived = 0;
		char* bufferOffset = (char*)destination + totalBytesReceived;
		Result result = Receive(bufferOffset, bytesRemaining, bytesReceived);
		if (result != Result::Success)
		{
			return Result::NotYetImplemented;
		}
		totalBytesReceived += bytesReceived;
	}

	return Result::Success;
}

Result Socket::Receive(Packet& packet)
{
	packet.Clear();

	uint16_t encodedSize = 0;

	Result result = ReceiveAll(&encodedSize, sizeof(uint16_t));
	if (result != Result::Success)
	{
		return Result::NotYetImplemented;
	}

	uint16_t bufferSize = ntohs(encodedSize);

	if (bufferSize > g_maxPacketSize)
	{
		return Result::NotYetImplemented;
	}

	packet.buffer.resize(bufferSize);
	result = ReceiveAll(&packet.buffer[0], bufferSize);
	if (result != Result::Success)
	{
		return Result::NotYetImplemented;
	}

	return Result::Success;
}

Result Socket::Connect(IPEndpoint endpoint)
{
	assert(m_ipversion == endpoint.GetIPVersion());

	int result = 0;

	if (m_ipversion == IPVersion::IPv4)
	{
		sockaddr_in addr = endpoint.GetSockaddrIPv4();

		result = connect(m_handle, (sockaddr*)&addr, sizeof(sockaddr_in));
	}
	else
	{
		sockaddr_in6 addr = endpoint.GetSockaddrIPv6();

		result = connect(m_handle, (sockaddr*)&addr, sizeof(sockaddr_in6));
	}
	
	if (result != 0)
	{
		int error = WSAGetLastError();

		return Result::NotYetImplemented;
	}

	return Result::Success;
}

Result Socket::Send(const void* data, int numberOfBytes, int& bytesSent)
{
	bytesSent = send(m_handle, (const char*)data, numberOfBytes, NULL);
	if (bytesSent == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		return Result::NotYetImplemented;
	}

	return Result::Success;
}

Result Socket::SendAll(const void* data, int numberOfBytes)
{
	int totalBytesSent = 0;

	while (totalBytesSent < numberOfBytes)
	{
		int bytesRemaining = numberOfBytes - totalBytesSent;
		int bytesSent = 0;
		char* bufferOffset = (char*)data + totalBytesSent;
		Result result = Send(bufferOffset, bytesRemaining, bytesSent);
		if (result != Result::Success)
		{
			return Result::NotYetImplemented;
		}
		totalBytesSent += bytesSent;
	}

    return Result::Success;
}

Result Socket::Send(Packet& packet)
{
	uint16_t encodedPacketSize = htons(packet.buffer.size());

	Result result = SendAll(&encodedPacketSize, sizeof(uint16_t));
	if (result != Result::Success)
	{
		return Result::NotYetImplemented;
	}

	result = SendAll(packet.buffer.data(), packet.buffer.size());
	if (result != Result::Success)
	{
		return Result::NotYetImplemented;
	}

	return Result::Success;
}

SocketHandle Socket::GetHandle()
{
	return m_handle;
}

IPVersion Socket::GetIPVersion()
{
	return m_ipversion;
}

Result Socket::SetBlocking(bool isBlocking)
{
	unsigned long nonBlocking = 1;
	unsigned long blocking = 0;
	int result = ioctlsocket(m_handle, FIONBIO, isBlocking ? &blocking : &nonBlocking);
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		return Result::NotYetImplemented;
	}
	return Result::Success;
}

Result Socket::setSocketOption(SocketOption option, int value)
{
	int result = 0;

	switch (option)
	{
	case SocketOption::TCP_NoDelay:
		result = setsockopt(m_handle, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(value));
		break;

	case SocketOption::IPV6_Only:
		result = setsockopt(m_handle, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&value, sizeof(value));
		break;

	default:
		return Result::NotYetImplemented;
	}

	if (result != 0)
	{
		int error = WSAGetLastError();

		return Result::NotYetImplemented;
	}

	return Result::Success;
}
