#ifndef PACKETEXCEPTION_H
#define PACKETEXCEPTION_H

#include <string>

class PacketException
{
public:
	PacketException(std::string exception)
		:m_exception(exception)
	{
	}
	
	const char* what()
	{
		return m_exception.c_str();
	}

	std::string ToString()
	{
		return m_exception;
	}

private:
	std::string m_exception;
};

#endif // !PACKETEXCEPTION_H
