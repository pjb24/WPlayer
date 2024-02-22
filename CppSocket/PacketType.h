#ifndef PACKETTYPE_H
#define PACKETTYPE_H

#include <stdint.h>

enum class PacketType : uint16_t
{
	Invalid,
	ChatMessage,
	IntegerArray,
	StructuredData,
};

enum class CommandType : uint16_t
{
    Play,
    Pause,
    Stop,
    Move,
};

#endif // !PACKETTYPE_H
