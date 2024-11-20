#ifndef PACKETTYPE_H
#define PACKETTYPE_H

#include <stdint.h>

enum class e_packet_type : uint16_t
{
	Invalid,
	ChatMessage,
	IntegerArray,
	StructuredData,
};

enum class e_command_type : uint16_t
{
    Play,
    Pause,
    Stop,
    Move,
};

#endif // !PACKETTYPE_H
