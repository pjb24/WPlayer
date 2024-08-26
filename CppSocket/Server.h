#ifndef SERVER_H
#define SERVER_H

#include "framework.h"
#include "TcpConnection.h"

class Server
{
public:
    virtual ~Server();

	bool Initialize(IPEndpoint ip);
	void Frame();

protected:
	virtual void OnConnect(TcpConnection* newConnection);
	virtual void OnDisconnect(TcpConnection* lostConnection, std::string reason);
	void CloseConnection(int connectionIndex, std::string reason);
	virtual bool ProcessPacket(std::shared_ptr<Packet> packet);
	virtual bool ProcessPacket(std::shared_ptr<Packet> packet, TcpConnection* connection);

	Socket m_listeningSocket;
	std::vector<TcpConnection*> m_connections;
	std::vector<WSAPOLLFD> m_masterFd;
	std::vector<WSAPOLLFD> m_useFd;
};

#endif // !SERVER_H
