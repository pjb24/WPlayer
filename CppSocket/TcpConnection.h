#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "Socket.h"
#include "PacketManager.h"

class TcpConnection
{
public:
	TcpConnection(Socket socket, IPEndpoint endpoint);
	TcpConnection() :m_socket(Socket()) {};
	void Close();
	std::string ToString();
	Socket m_socket;

	PacketManager m_pmIncoming;
	PacketManager m_pmOutgoing;
	char m_buffer[g_maxPacketSize];

    IPEndpoint get_ip_endpoint() { return m_endpoint; }

private:
	IPEndpoint m_endpoint;
	std::string m_stringRepresentation = "";
};

#endif // !TCPCONNECTION_H
