#include "IPEndpoint.h"
#include <assert.h>
#include <iostream>

IPEndpoint::IPEndpoint(const char* ip, unsigned short port)
{
	m_port = port;

	// IPv4
	in_addr addr;
	int result = inet_pton(AF_INET, ip, &addr);
	if (result == 1)
	{
		if (addr.S_un.S_addr != INADDR_NONE)
		{
			m_ipString = ip;
			m_hostname = ip;
			trim(m_ipString);
			trim(m_hostname);

			m_ipBytes.resize(sizeof(ULONG));
			memcpy(&m_ipBytes[0], &addr.S_un.S_addr, sizeof(ULONG));
			m_ipversion = IPVersion::IPv4;
		}

		return;
	}
	
	addrinfo hints = {};
	hints.ai_family = AF_INET;
	addrinfo* hostinfo = nullptr;
	result = getaddrinfo(ip, NULL, &hints, &hostinfo);
	if (result == 0)
	{
		sockaddr_in* hostAddr = reinterpret_cast<sockaddr_in*>(hostinfo->ai_addr);

		m_ipString.resize(16);
		inet_ntop(AF_INET, &hostAddr->sin_addr.S_un.S_addr, &m_ipString[0], 16);

		m_hostname = ip;
		trim(m_ipString);
		trim(m_hostname);

		ULONG ipLong = hostAddr->sin_addr.S_un.S_addr;
		m_ipBytes.resize(sizeof(ULONG));
		memcpy(&m_ipBytes[0], &ipLong, sizeof(ULONG));

		m_ipversion = IPVersion::IPv4;

		freeaddrinfo(hostinfo);

		return;
	}

	// IPv6
	in6_addr addr6;
	result = inet_pton(AF_INET6, ip, &addr6);
	if (result == 1)
	{
		m_ipString = ip;
		m_hostname = ip;
		trim(m_ipString);
		trim(m_hostname);

		m_ipBytes.resize(16);
		memcpy(&m_ipBytes[0], &addr6.u, 16);
		m_ipversion = IPVersion::IPv6;

		return;
	}

	addrinfo hintsV6 = {};
	hintsV6.ai_family = AF_INET6;
	addrinfo* hostinfoV6 = nullptr;
	result = getaddrinfo(ip, NULL, &hintsV6, &hostinfoV6);
	if (result == 0)
	{
		sockaddr_in6* hostAddrV6 = reinterpret_cast<sockaddr_in6*>(hostinfoV6->ai_addr);

		m_ipString.resize(46);
		inet_ntop(AF_INET6, &hostAddrV6->sin6_addr, &m_ipString[0], 46);

		m_hostname = ip;
		trim(m_ipString);
		trim(m_hostname);

		m_ipBytes.resize(16);
		memcpy(&m_ipBytes[0], &hostAddrV6->sin6_addr, 16);

		m_ipversion = IPVersion::IPv6;

		freeaddrinfo(hostinfoV6);

		return;
	}
}

IPEndpoint::IPEndpoint(sockaddr* addr)
{
	assert(addr->sa_family == AF_INET || addr->sa_family == AF_INET6);

	if (addr->sa_family == AF_INET)
	{
		sockaddr_in* addrV4 = reinterpret_cast<sockaddr_in*>(addr);
		m_ipversion = IPVersion::IPv4;
		m_port = ntohs(addrV4->sin_port);
		m_ipBytes.resize(sizeof(ULONG));
		memcpy(&m_ipBytes[0], &addrV4->sin_addr.S_un.S_addr, sizeof(ULONG));
		m_ipString.resize(16);
		inet_ntop(AF_INET, &addrV4->sin_addr.S_un.S_addr, &m_ipString[0], 16);

		m_hostname = m_ipString;
	}
	else
	{
		sockaddr_in6* addrV6 = reinterpret_cast<sockaddr_in6*>(addr);
		m_ipversion = IPVersion::IPv6;
		m_port = ntohs(addrV6->sin6_port);
		m_ipBytes.resize(16);
		memcpy(&m_ipBytes[0], &addrV6->sin6_addr, 16);
		m_ipString.resize(46);
		inet_ntop(AF_INET6, &addrV6->sin6_addr, &m_ipString[0], 46);

		m_hostname = m_ipString;
	}

	trim(m_ipString);
	trim(m_hostname);
}

IPVersion IPEndpoint::GetIPVersion()
{
	return m_ipversion;
}

std::string IPEndpoint::GetHostname()
{
	return m_hostname;
}

std::string IPEndpoint::GetIPString()
{
	return m_ipString;
}

std::vector<uint8_t> IPEndpoint::GetIPBytes()
{
	return m_ipBytes;
}

unsigned short IPEndpoint::GetPort()
{
	return m_port;
}

sockaddr_in IPEndpoint::GetSockaddrIPv4()
{
	assert(m_ipversion == IPVersion::IPv4);

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	memcpy(&addr.sin_addr, &m_ipBytes[0], sizeof(ULONG));
	addr.sin_port = htons(m_port);

	return addr;
}

sockaddr_in6 IPEndpoint::GetSockaddrIPv6()
{
	assert(m_ipversion == IPVersion::IPv6);

	sockaddr_in6 addr = {};
	addr.sin6_family = AF_INET6;
	memcpy(&addr.sin6_addr, &m_ipBytes[0], 16);
	addr.sin6_port = htons(m_port);

	return addr;
}

void IPEndpoint::Print()
{
	switch (m_ipversion)
	{
	case IPVersion::IPv4:
		std::cout << "IP Version: IPv4" << std::endl;
		break;
	case IPVersion::IPv6:
		std::cout << "IP Version: IPv6" << std::endl;
		break;
	default:
		std::cout << "IP Version: Unknown" << std::endl;
		break;
	}

	std::cout << "Hostname: " << m_hostname << std::endl;
	std::cout << "IP: " << m_ipString << std::endl;
	std::cout << "Port: " << m_port << std::endl;
	std::cout << "IP bytes..." << std::endl;
	for (auto & digit : m_ipBytes)
	{
		std::cout << (int)digit << std::endl;
	}
}
