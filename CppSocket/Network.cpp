#include "Network.h"

bool Network::Initialize()
{
	WSADATA wsaData;
	
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		std::cerr << "Failed to start up the winsock API." << std::endl;

		printf("WSAStartup failed: %d\n", result);
		return false;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		std::cerr << "Could not find a usable version of the winsock api dll." << std::endl;
		return false;
	}

	return true;
}

void Network::Shutdown()
{
	WSACleanup();
}
