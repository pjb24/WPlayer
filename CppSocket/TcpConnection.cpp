#include "TcpConnection.h"

TcpConnection::TcpConnection(Socket socket, IPEndpoint endpoint)
	:m_socket(socket), m_endpoint(endpoint)
{
	m_stringRepresentation = "[" + endpoint.GetIPString();
	m_stringRepresentation += ":" + std::to_string(endpoint.GetPort()) + "]";
}

void TcpConnection::Close()
{
	m_socket.Close();
}

std::string TcpConnection::ToString()
{
	return m_stringRepresentation;
}
