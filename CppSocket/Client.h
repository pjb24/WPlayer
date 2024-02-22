#ifndef CLIENT_H
#define CLIENT_H

#include "framework.h"
#include "TcpConnection.h"

class Client
{
public:
	bool Connect(IPEndpoint ip);
	bool IsConnected();
	bool Frame();

protected:
	virtual bool ProcessPacket(std::shared_ptr<Packet> packet);
	virtual void OnConnect();
	virtual void OnConnectFail();
	virtual void OnDisconnect(std::string reason);
	void CloseConnection(std::string reason);

	TcpConnection m_connection;

private:
	bool m_isConnected = false;
	WSAPOLLFD m_masterFd;
	WSAPOLLFD m_useFd;
};

#endif // !CLIENT_H
