#include "Packet.h"
#include "Constants.h"

Packet::Packet(e_packet_type packetType)
{
	Clear();
	AssignPacketType(packetType);
}

e_packet_type Packet::GetPacketType()
{
    e_packet_type* packetTypePtr = reinterpret_cast<e_packet_type*>(&buffer[0]);
    return static_cast<e_packet_type>(ntohs((uint16_t)*packetTypePtr));
}

void Packet::AssignPacketType(e_packet_type packetType)
{
    e_packet_type* packetTypePtr = reinterpret_cast<e_packet_type*>(&buffer[0]);
    *packetTypePtr = static_cast<e_packet_type>(htons((uint16_t)packetType));
}

void Packet::Clear()
{
    buffer.resize(sizeof(e_packet_type));
    AssignPacketType(e_packet_type::Invalid);
	extractionOffset = sizeof(e_packet_type);
}

void Packet::Append(const void* data, uint32_t size)
{
	if ((buffer.size() + size) > g_maxPacketSize)
	{
		throw PacketException("[Packet::Append(const void*, uint32_t)] - Packet size exceeded max packet size.");
	}

	buffer.insert(buffer.end(), (char*)data, (char*)data + size);
}

Packet& Packet::operator<<(uint32_t data)
{
	data = htonl(data);
	Append(&data, sizeof(uint32_t));
	return *this;
}

Packet& Packet::operator>>(uint32_t& data)
{
	if ((extractionOffset + sizeof(uint32_t)) > buffer.size())
	{
		throw PacketException("[Packet::operator>>(uint32_t&)] - Extraction offset exceeded buffer size.");
	}

	data = *reinterpret_cast<uint32_t*>(&buffer[extractionOffset]);
	data = ntohl(data);
	extractionOffset += sizeof(uint32_t);
	return *this;
}

Packet& Packet::operator<<(const std::string& data)
{
	*this << (uint32_t)data.size();
	Append(data.data(), data.size());
	return *this;
}

Packet& Packet::operator>>(std::string& data)
{
	data.clear();

	uint32_t stringSize = 0;
	*this >> stringSize;

	if ((extractionOffset + stringSize) > buffer.size())
	{
		throw PacketException("[Packet::operator>>(std::string&)] - Extraction offset exceeded buffer size.");
	}

	data.resize(stringSize);
	data.assign(&buffer[extractionOffset], stringSize);
	extractionOffset += stringSize;
	return *this;
}

Packet & Packet::operator<<(void * data)
{
    packet_header* header = nullptr;
    header = (packet_header*)data;

    *this << (uint32_t)header->size;
    Append(data, header->size);
    return *this;
}

Packet & Packet::operator>>(void *& data)
{
    uint32_t size = 0;
    *this >> size;
    
    if ((extractionOffset + size) > buffer.size())
    {
        throw PacketException("[Packet::operator>>(void*&)] - Extraction offset exceeded buffer size.");
    }

    data = malloc(size);

    memcpy(data, &buffer[extractionOffset], size);
    extractionOffset += size;
    return *this;
}
