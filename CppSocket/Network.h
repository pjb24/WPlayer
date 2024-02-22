#ifndef NETWORK_H
#define NETWORK_H

#include "framework.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "Ws2_32.lib")

#include <iostream>

class Network
{
public:
	static bool Initialize();
	static void Shutdown();
};

#endif // !NETWORK_H
