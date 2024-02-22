#include "Client.h"

bool Client::Connect(IPEndpoint ip)
{
	m_isConnected = false;

	Socket socket = Socket(ip.GetIPVersion());
	if (socket.Create() == Result::Success)
	{
		if (socket.SetBlocking(true) != Result::Success)
		{
			return false;
		}

		std::cout << "Socket successfully created." << std::endl;

		if (socket.Connect(ip) == Result::Success)
		{
			if (socket.SetBlocking(false) == Result::Success)
			{
				m_connection = TcpConnection(socket, ip);

				m_masterFd.fd = m_connection.m_socket.GetHandle();
				m_masterFd.events = POLLRDNORM;
				m_masterFd.revents = 0;

				m_isConnected = true;
				OnConnect();
				return true;
			}
		}
		else
		{
			std::cerr << "Failed to connect to server." << std::endl;
		}

		socket.Close();
	}
	else
	{
		std::cerr << "Socket failed to create." << std::endl;
	}

	OnConnectFail();

	return false;
}

bool Client::IsConnected()
{
	return m_isConnected;
}

bool Client::Frame()
{
	if (m_connection.m_pmOutgoing.HasPendingPackets())
	{
		m_masterFd.events = POLLRDNORM | POLLWRNORM;
	}

	m_useFd = m_masterFd;

	if (WSAPoll(&m_useFd, 1, 1) > 0)
	{
		if (m_useFd.revents & POLLERR)	// If error occurred on this socket
		{
			CloseConnection("POLLERR");
			return false;
		}

		if (m_useFd.revents & POLLHUP)	// If poll hangup occurred on this socket
		{
			CloseConnection("POLLHUP");
			return false;
		}

		if (m_useFd.revents & POLLNVAL)	// If invalid socket
		{
			CloseConnection("POLLNVAL");
			return false;
		}

		if (m_useFd.revents & POLLRDNORM)	// If normal data can be read without blocking
		{
			int bytesReceived = 0;

			if (m_connection.m_pmIncoming.m_currentTask == PacketTask::ProcessPacketSize)
			{
				bytesReceived = recv(m_useFd.fd, (char*)&m_connection.m_pmIncoming.m_currentPacketSize + m_connection.m_pmIncoming.m_currentPacketExtractionOffset, sizeof(uint16_t) - m_connection.m_pmIncoming.m_currentPacketExtractionOffset, 0);
			}
			else	// Process packet contents
			{
				bytesReceived = recv(m_useFd.fd, (char*)&m_connection.m_buffer + m_connection.m_pmIncoming.m_currentPacketExtractionOffset, m_connection.m_pmIncoming.m_currentPacketSize - m_connection.m_pmIncoming.m_currentPacketExtractionOffset, 0);
			}

			if (bytesReceived == 0)	// If connection was lost
			{
				CloseConnection("Recv==0");
				return false;
			}

			if (bytesReceived == SOCKET_ERROR)	// If error occurred on socket
			{
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK)
				{
					CloseConnection("Recv<0");
					return false;
				}
			}

			if (bytesReceived > 0)
			{
				m_connection.m_pmIncoming.m_currentPacketExtractionOffset += bytesReceived;
				if (m_connection.m_pmIncoming.m_currentTask == PacketTask::ProcessPacketSize)
				{
					if (m_connection.m_pmIncoming.m_currentPacketExtractionOffset == sizeof(uint16_t))
					{
						m_connection.m_pmIncoming.m_currentPacketSize = ntohs(m_connection.m_pmIncoming.m_currentPacketSize);
						if (m_connection.m_pmIncoming.m_currentPacketSize > g_maxPacketSize)
						{
							CloseConnection("Packet size too large.");
							return false;
						}

						m_connection.m_pmIncoming.m_currentPacketExtractionOffset = 0;
						m_connection.m_pmIncoming.m_currentTask = PacketTask::ProcessPacketContents;
					}
				}
				else	// Processing packet contents
				{
					if (m_connection.m_pmIncoming.m_currentPacketExtractionOffset == m_connection.m_pmIncoming.m_currentPacketSize)
					{
						std::shared_ptr<Packet> packet = std::make_shared<Packet>();
						packet->buffer.resize(m_connection.m_pmIncoming.m_currentPacketSize);
						memcpy(&packet->buffer[0], m_connection.m_buffer, m_connection.m_pmIncoming.m_currentPacketSize);

						m_connection.m_pmIncoming.Append(packet);

						m_connection.m_pmIncoming.m_currentPacketSize = 0;
						m_connection.m_pmIncoming.m_currentPacketExtractionOffset = 0;
						m_connection.m_pmIncoming.m_currentTask = PacketTask::ProcessPacketSize;
					}
				}
			}
		}

		if (m_useFd.revents & POLLWRNORM)	// If normal data can be written without blocking
		{
			PacketManager& pm = m_connection.m_pmOutgoing;
			while (pm.HasPendingPackets())
			{
				if (pm.m_currentTask == PacketTask::ProcessPacketSize)	// Sending packet size
				{
					pm.m_currentPacketSize = pm.Retrieve()->buffer.size();
					uint16_t bigEndianPacketSize = htons(pm.m_currentPacketSize);
					int bytesSent = send(m_useFd.fd, (char*)(&bigEndianPacketSize) + pm.m_currentPacketExtractionOffset, sizeof(uint16_t) - pm.m_currentPacketExtractionOffset, 0);
					if (bytesSent > 0)
					{
						pm.m_currentPacketExtractionOffset += bytesSent;
					}

					if (pm.m_currentPacketExtractionOffset == sizeof(uint16_t))	// If full packet size was sent
					{
						pm.m_currentPacketExtractionOffset = 0;
						pm.m_currentTask = PacketTask::ProcessPacketContents;
					}
					else	// If full packet size was not sent, break out of the loop for sending outgoing packets for this connection - we'll have to try again next time we are able to write normal data without blocking
					{
						break;
					}
				}
				else	// Sending packet contents
				{
					char* bufferPtr = &pm.Retrieve()->buffer[0];
					int bytesSent = send(m_useFd.fd, (char*)(bufferPtr)+pm.m_currentPacketExtractionOffset, pm.m_currentPacketSize - pm.m_currentPacketExtractionOffset, 0);
					if (bytesSent > 0)
					{
						pm.m_currentPacketExtractionOffset += bytesSent;
					}

					if (pm.m_currentPacketExtractionOffset == pm.m_currentPacketSize)	// If full packet contents have been sent
					{
						pm.m_currentPacketExtractionOffset = 0;
						pm.m_currentTask = PacketTask::ProcessPacketSize;
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
				m_masterFd.events = POLLRDNORM;
			}
		}
	}

	while (m_connection.m_pmIncoming.HasPendingPackets())
	{
		std::shared_ptr<Packet> frontPacket = m_connection.m_pmIncoming.Retrieve();
		if (!ProcessPacket(frontPacket))
		{
			CloseConnection("Failed to process incoming packet.");
			return false;
		}
		m_connection.m_pmIncoming.Pop();
	}
}

bool Client::ProcessPacket(std::shared_ptr<Packet> packet)
{
	std::cout << "Packet received with size: " << packet->buffer.size() << std::endl;

	return true;
}

void Client::OnConnect()
{
	std::cout << "Successfully connected!" << std::endl;
}

void Client::OnConnectFail()
{
	std::cout << "Failed to connect." << std::endl;
}

void Client::OnDisconnect(std::string reason)
{
	std::cout << "Lost connection. Reason: " << reason << "." << std::endl;
}

void Client::CloseConnection(std::string reason)
{
	OnDisconnect(reason);
	m_masterFd.fd = 0;
	m_isConnected = false;
	m_connection.Close();
}
