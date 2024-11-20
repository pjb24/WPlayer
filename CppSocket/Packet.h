#ifndef PACKET_H
#define PACKET_H

#include <WinSock2.h>
#include <vector>
#include <string>
#include "PacketException.h"
#include "PacketDefine.h"

class Packet
{
public:
	Packet(e_packet_type packet_type = e_packet_type::Invalid);
    e_packet_type GetPacketType();
    void AssignPacketType(e_packet_type packet_type);

	void Clear();
	void Append(const void* data, uint32_t size);

	Packet& operator << (uint32_t data);
	Packet& operator >> (uint32_t& data);

	Packet& operator << (const std::string& data);
	Packet& operator >> (std::string& data);

    Packet& operator << (void* data);
    Packet& operator >> (void*& data);

	uint32_t extractionOffset = 0;
	std::vector<char> buffer;
};

#endif // !PACKET_H
