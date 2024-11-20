#include "Server.h"

Server::~Server()
{
    if (m_connections.size() > 0)
    {
        while (m_connections.size() != 0)
        {
            size_t i = m_connections.size() - 1;

            TcpConnection* connection = m_connections[i];
            OnDisconnect(connection, std::string("Server Destructor"));

            m_masterFd.erase(m_masterFd.begin() + (i + 1));
            m_useFd.erase(m_useFd.begin() + (i + 1));
            connection->Close();
            delete connection;
            m_connections.erase(m_connections.begin() + i);
        }
    }
}

bool Server::Initialize(IPEndpoint ip)
{
	m_masterFd.clear();
	m_connections.clear();
	m_useFd.clear();

	m_listeningSocket = Socket(ip.GetIPVersion());
	if (m_listeningSocket.Create() == e_result::Success)
	{
		std::cout << "Socket successfully created." << std::endl;

		if (m_listeningSocket.Listen(ip, 5) == e_result::Success)
		{
			WSAPOLLFD listeningSocketFd = {};
			listeningSocketFd.fd = m_listeningSocket.GetHandle();
			listeningSocketFd.events = POLLRDNORM; // | POLLWRNORM;
			listeningSocketFd.revents = 0; // POLLRDNORM; // | POLLWRNORM;

			m_masterFd.push_back(listeningSocketFd);

			std::cout << "Socket successfully listening." << std::endl;
			return true;
		}
		else
		{
			std::cerr << "Failed to listen." << std::endl;
		}

		m_listeningSocket.Close();
	}
	else
	{
		std::cerr << "Socket failed to create." << std::endl;
	}

    return false;
}

void Server::Frame()
{
	for (size_t i = 0; i < m_connections.size(); i++)
	{
		if (m_connections[i]->m_pmOutgoing.HasPendingPackets())
		{
			m_masterFd[i + 1].events = POLLRDNORM | POLLWRNORM;
		}
	}

	m_useFd = m_masterFd;

	if (WSAPoll(m_useFd.data(), m_useFd.size(), 1) > 0)
	{
#pragma region Listener
		WSAPOLLFD& listeningSocketFd = m_useFd[0];

		if (listeningSocketFd.revents & POLLRDNORM)
		{
			Socket newConnection;
			IPEndpoint newConnectionEndpoint;
			if (m_listeningSocket.Accept(newConnection, &newConnectionEndpoint) == e_result::Success)
			{
				m_connections.emplace_back(new TcpConnection(newConnection, newConnectionEndpoint));
				TcpConnection* acceptedConnection = m_connections[m_connections.size() - 1];
				
				WSAPOLLFD newConnectionFd = {};
				newConnectionFd.fd = newConnection.GetHandle();
				newConnectionFd.events = POLLRDNORM;
				newConnectionFd.revents = 0;
				m_masterFd.push_back(newConnectionFd);
				OnConnect(acceptedConnection);
			}
			else
			{
				std::cerr << "Failed to accept New connection." << std::endl;
			}
		}
#pragma endregion Code specific to the listening socket
		
		// useFd[0] is Listening Socket
		for (int i = m_useFd.size() - 1; i >= 1; i--)
		{
			int connectionIndex = i - 1;
			TcpConnection* connection = m_connections[connectionIndex];

			if (m_useFd[i].revents & POLLERR)	// If error occurred on this socket
			{
				CloseConnection(connectionIndex, "POLLERR");
				continue;
			}

			if (m_useFd[i].revents & POLLHUP)	// If poll hangup occurred on this socket
			{
				CloseConnection(connectionIndex, "POLLHUP");
				continue;
			}

			if (m_useFd[i].revents & POLLNVAL)	// If invalid socket
			{
				CloseConnection(connectionIndex, "POLLNVAL");
				continue;
			}

			if (m_useFd[i].revents & POLLRDNORM)	// If normal data can be read without blocking
			{
				int bytesReceived = 0;

				if (connection->m_pmIncoming.m_currentTask == e_packet_task::ProcessPacketSize)
				{
					bytesReceived = recv(m_useFd[i].fd, (char*)&connection->m_pmIncoming.m_currentPacketSize + connection->m_pmIncoming.m_currentPacketExtractionOffset, sizeof(uint16_t) - connection->m_pmIncoming.m_currentPacketExtractionOffset, 0);
				}
				else	// Process packet contents
				{
					bytesReceived = recv(m_useFd[i].fd, (char*)&connection->m_buffer + connection->m_pmIncoming.m_currentPacketExtractionOffset, connection->m_pmIncoming.m_currentPacketSize - connection->m_pmIncoming.m_currentPacketExtractionOffset, 0);
				}

				if (bytesReceived == 0)	// If connection was lost
				{
					CloseConnection(connectionIndex, "Recv==0");
					continue;
				}

				if (bytesReceived == SOCKET_ERROR)	// If error occurred on socket
				{
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK)
					{
						CloseConnection(connectionIndex, "Recv<0");
						continue;
					}
				}

				if (bytesReceived > 0)
				{
					connection->m_pmIncoming.m_currentPacketExtractionOffset += bytesReceived;
					if (connection->m_pmIncoming.m_currentTask == e_packet_task::ProcessPacketSize)
                    {
                        if (connection->m_pmIncoming.m_currentPacketExtractionOffset == sizeof(uint16_t))
                        {
                            connection->m_pmIncoming.m_currentPacketSize = ntohs(connection->m_pmIncoming.m_currentPacketSize);
                            if (connection->m_pmIncoming.m_currentPacketSize > g_maxPacketSize)
                            {
                                CloseConnection(connectionIndex, "Packet size too large.");
                                continue;
                            }

                            connection->m_pmIncoming.m_currentPacketExtractionOffset = 0;
                            connection->m_pmIncoming.m_currentTask = e_packet_task::ProcessPacketContents;
						}
					}
					else	// Processing packet contents
					{
						if (connection->m_pmIncoming.m_currentPacketExtractionOffset == connection->m_pmIncoming.m_currentPacketSize)
						{
							std::shared_ptr<Packet> packet = std::make_shared<Packet>();
							packet->buffer.resize(connection->m_pmIncoming.m_currentPacketSize);
							memcpy(&packet->buffer[0], connection->m_buffer, connection->m_pmIncoming.m_currentPacketSize);

							connection->m_pmIncoming.Append(packet);

							connection->m_pmIncoming.m_currentPacketSize = 0;
							connection->m_pmIncoming.m_currentPacketExtractionOffset = 0;
							connection->m_pmIncoming.m_currentTask = e_packet_task::ProcessPacketSize;
						}
					}
				}
			}

			if (m_useFd[i].revents & POLLWRNORM)	// If normal data can be written without blocking
			{
				PacketManager& pm = connection->m_pmOutgoing;
				while (pm.HasPendingPackets())
				{
					if (pm.m_currentTask == e_packet_task::ProcessPacketSize)	// Sending packet size
                    {
                        pm.m_currentPacketSize = pm.Retrieve()->buffer.size();
                        uint16_t bigEndianPacketSize = htons(pm.m_currentPacketSize);
                        int bytesSent = send(m_useFd[i].fd, (char*)(&bigEndianPacketSize) + pm.m_currentPacketExtractionOffset, sizeof(uint16_t) - pm.m_currentPacketExtractionOffset, 0);
                        if (bytesSent > 0)
                        {
                            pm.m_currentPacketExtractionOffset += bytesSent;
                        }

                        if (pm.m_currentPacketExtractionOffset == sizeof(uint16_t))	// If full packet size was sent
                        {
                            pm.m_currentPacketExtractionOffset = 0;
                            pm.m_currentTask = e_packet_task::ProcessPacketContents;
						}
						else	// If full packet size was not sent, break out of the loop for sending outgoing packets for this connection - we'll have to try again next time we are able to write normal data without blocking
						{
							break;
						}
					}
					else	// Sending packet contents
					{
						char* bufferPtr = &pm.Retrieve()->buffer[0];
						int bytesSent = send(m_useFd[i].fd, (char*)(bufferPtr) + pm.m_currentPacketExtractionOffset, pm.m_currentPacketSize - pm.m_currentPacketExtractionOffset, 0);
						if (bytesSent > 0)
						{
							pm.m_currentPacketExtractionOffset += bytesSent;
						}

						if (pm.m_currentPacketExtractionOffset == pm.m_currentPacketSize)	// If full packet contents have been sent
						{
							pm.m_currentPacketExtractionOffset = 0;
							pm.m_currentTask = e_packet_task::ProcessPacketSize;
							pm.Pop();	// Remove packet from queue after finished processing
						}
						else
						{
							break;
						}
					}
				}

				if (!pm.HasPendingPackets())
				{
					m_masterFd[i].events = POLLRDNORM;
				}
			}
		}
	}

	for (int i = m_connections.size() - 1; i >= 0; i--)
	{
		while (m_connections[i]->m_pmIncoming.HasPendingPackets())
		{
			std::shared_ptr<Packet> frontPacket = m_connections[i]->m_pmIncoming.Retrieve();
			//if (!ProcessPacket(frontPacket))
			if (!ProcessPacket(frontPacket, m_connections[i]))
			{
				CloseConnection(i, "Failed to process incoming packet.");
				break;
			}
			m_connections[i]->m_pmIncoming.Pop();
		}
	}
}

void Server::OnConnect(TcpConnection* newConnection)
{
	std::cout << newConnection->ToString() << " - New connection accepted." << std::endl;
}

void Server::OnDisconnect(TcpConnection* lostConnection, std::string reason)
{
	std::cout << "[" << reason << "] Connection lost: " << lostConnection->ToString() << "." << std::endl;
}

void Server::CloseConnection(int connectionIndex, std::string reason)
{
	TcpConnection* connection = m_connections[connectionIndex];
	OnDisconnect(connection, reason);

	m_masterFd.erase(m_masterFd.begin() + (connectionIndex + 1));
	m_useFd.erase(m_useFd.begin() + (connectionIndex + 1));
	connection->Close();
    delete connection;
	m_connections.erase(m_connections.begin() + connectionIndex);
}

bool Server::ProcessPacket(std::shared_ptr<Packet> packet)
{
	std::cout << "Packet received with size: " << packet->buffer.size() << std::endl;

	return true;
}

bool Server::ProcessPacket(std::shared_ptr<Packet> packet, TcpConnection* connection)
{
    std::cout << "Packet received with size: " << packet->buffer.size() << std::endl;

    return true;
}
