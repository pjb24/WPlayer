#include "Socket.h"
#include <assert.h>

Socket::Socket(IPVersion ipversion, SocketHandle handle)
	:m_ipversion(ipversion)
	, m_handle(handle)
{
	assert(ipversion == IPVersion::IPv4 || ipversion == IPVersion::IPv6);
}

e_result Socket::Create()
{
    assert(m_ipversion == IPVersion::IPv4 || m_ipversion == IPVersion::IPv6);

    if (m_handle != INVALID_SOCKET)
    {
        return e_result::NotYetImplemented;
    }

    m_handle = socket((m_ipversion == IPVersion::IPv4) ? AF_INET : AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (m_handle == INVALID_SOCKET)
    {
        int error = WSAGetLastError();

        return e_result::NotYetImplemented;
    }

    if (setSocketOption(e_socket_option::TCP_NoDelay, TRUE) != e_result::Success)
    {
        return e_result::NotYetImplemented;
    }

    return e_result::Success;
}

e_result Socket::Close()
{
    if (m_handle == INVALID_SOCKET)
    {
        return e_result::NotYetImplemented;
    }

    int result = closesocket(m_handle);
    if (result != 0)
    {
        int error = WSAGetLastError();

        return e_result::NotYetImplemented;
	}

	m_handle = INVALID_SOCKET;

	return e_result::Success;
}

e_result Socket::Bind(IPEndpoint endpoint)
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

		return e_result::NotYetImplemented;
    }

    return e_result::Success;
}

e_result Socket::Listen(IPEndpoint endpoint, int backlog)
{
    if (m_ipversion == IPVersion::IPv6)
    {
        if (setSocketOption(e_socket_option::IPV6_Only, FALSE) != e_result::Success)
        {
            return e_result::NotYetImplemented;
        }
    }

    if (Bind(endpoint) != e_result::Success)
    {
        return e_result::NotYetImplemented;
    }

    int result = listen(m_handle, backlog);
    if (result != 0)
    {
        int error = WSAGetLastError();

        return e_result::NotYetImplemented;
    }

    return e_result::Success;
}

e_result Socket::Accept(Socket& outSocket, IPEndpoint* endpoint)
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

			return e_result::NotYetImplemented;
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

            return e_result::NotYetImplemented;
        }

        if (endpoint != nullptr)
        {
            *endpoint = IPEndpoint((sockaddr*)&addr);
        }

        outSocket = Socket(IPVersion::IPv6, acceptedConnectionHandle);
    }

    return e_result::Success;
}

e_result Socket::Receive(void* destination, int numberOfBytes, int& bytesReceived)
{
    bytesReceived = recv(m_handle, (char*)destination, numberOfBytes, NULL);
    if (bytesReceived == 0)
    {
        return e_result::NotYetImplemented;
	}
	
	if (bytesReceived == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		return e_result::NotYetImplemented;
    }

    return e_result::Success;
}

e_result Socket::ReceiveAll(void* destination, int numberOfBytes)
{
    int totalBytesReceived = 0;

    while (totalBytesReceived < numberOfBytes)
    {
        int bytesRemaining = numberOfBytes - totalBytesReceived;
        int bytesReceived = 0;
        char* bufferOffset = (char*)destination + totalBytesReceived;
        e_result result = Receive(bufferOffset, bytesRemaining, bytesReceived);
        if (result != e_result::Success)
        {
            return e_result::NotYetImplemented;
        }
        totalBytesReceived += bytesReceived;
    }

    return e_result::Success;
}

e_result Socket::Receive(Packet& packet)
{
    packet.Clear();

    uint16_t encodedSize = 0;

    e_result result = ReceiveAll(&encodedSize, sizeof(uint16_t));
    if (result != e_result::Success)
    {
        return e_result::NotYetImplemented;
	}

	uint16_t bufferSize = ntohs(encodedSize);

	if (bufferSize > g_maxPacketSize)
	{
		return e_result::NotYetImplemented;
    }

    packet.buffer.resize(bufferSize);
    result = ReceiveAll(&packet.buffer[0], bufferSize);
    if (result != e_result::Success)
    {
        return e_result::NotYetImplemented;
    }

    return e_result::Success;
}

e_result Socket::Connect(IPEndpoint endpoint)
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

        return e_result::NotYetImplemented;
    }

    return e_result::Success;
}

e_result Socket::Send(const void* data, int numberOfBytes, int& bytesSent)
{
    bytesSent = send(m_handle, (const char*)data, numberOfBytes, NULL);
    if (bytesSent == SOCKET_ERROR)
    {
        int error = WSAGetLastError();

        return e_result::NotYetImplemented;
    }

    return e_result::Success;
}

e_result Socket::SendAll(const void* data, int numberOfBytes)
{
    int totalBytesSent = 0;

    while (totalBytesSent < numberOfBytes)
    {
        int bytesRemaining = numberOfBytes - totalBytesSent;
        int bytesSent = 0;
        char* bufferOffset = (char*)data + totalBytesSent;
        e_result result = Send(bufferOffset, bytesRemaining, bytesSent);
        if (result != e_result::Success)
        {
            return e_result::NotYetImplemented;
        }
        totalBytesSent += bytesSent;
    }

    return e_result::Success;
}

e_result Socket::Send(Packet& packet)
{
    uint16_t encodedPacketSize = htons(packet.buffer.size());

    e_result result = SendAll(&encodedPacketSize, sizeof(uint16_t));
    if (result != e_result::Success)
    {
        return e_result::NotYetImplemented;
	}

	result = SendAll(packet.buffer.data(), packet.buffer.size());
	if (result != e_result::Success)
    {
        return e_result::NotYetImplemented;
    }

    return e_result::Success;
}

SocketHandle Socket::GetHandle()
{
    return m_handle;
}

IPVersion Socket::GetIPVersion()
{
    return m_ipversion;
}

e_result Socket::SetBlocking(bool isBlocking)
{
    unsigned long nonBlocking = 1;
    unsigned long blocking = 0;
    int result = ioctlsocket(m_handle, FIONBIO, isBlocking ? &blocking : &nonBlocking);
    if (result == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        return e_result::NotYetImplemented;
    }
    return e_result::Success;
}

e_result Socket::setSocketOption(e_socket_option option, int value)
{
	int result = 0;

	switch (option)
	{
	case e_socket_option::TCP_NoDelay:
        result = setsockopt(m_handle, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(value));
        break;

    case e_socket_option::IPV6_Only:
		result = setsockopt(m_handle, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&value, sizeof(value));
		break;

	default:
		return e_result::NotYetImplemented;
    }

    if (result != 0)
    {
        int error = WSAGetLastError();

        return e_result::NotYetImplemented;
    }

    return e_result::Success;
}
