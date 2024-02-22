#ifndef IPENDPOINT_H
#define IPENDPOINT_H

#include "IPVersion.h"
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include "StringTrimHelper.h"

class IPEndpoint
{
public:
	IPEndpoint() {};
	IPEndpoint(const char* ip, unsigned short port);
	IPEndpoint(sockaddr* addr);

	IPVersion GetIPVersion();
	std::string GetHostname();
	std::string GetIPString();
	std::vector<uint8_t> GetIPBytes();
	unsigned short GetPort();
	sockaddr_in GetSockaddrIPv4();
	sockaddr_in6 GetSockaddrIPv6();

	void Print();

private:
	IPVersion m_ipversion = IPVersion::Unknown;
	std::string m_hostname = "";
	std::string m_ipString = "";
	std::vector<uint8_t> m_ipBytes;
	unsigned short m_port = 0;
};
#endif // !IPENDPOINT_H
