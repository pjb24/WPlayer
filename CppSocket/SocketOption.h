#ifndef SOCKETOPTION_H
#define SOCKETOPTION

enum class SocketOption
{
	TCP_NoDelay,	// True = disable nagle's algorithm
	IPV6_Only,		// True = Only ipv6 can connect. False = ipv4 and ipv6 can both connect.
};

#endif // !SOCKETOPTION_H
