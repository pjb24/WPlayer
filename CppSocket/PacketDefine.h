#pragma once

#include "TypesDefine.h"

enum class PacketType : uint16_t
{
    Invalid,        // 0
    ChatMessage,    // 1
    IntegerArray,   // 2
    structured_data_from_client, // 3
    structured_data_from_server, // 4
};

enum class packet_result : uint16_t
{
    ok,     // 0
    fail,   // 1
	pause,  // 2
    resume, // 3
};

enum class command_type : uint16_t
{
    play,   // 0
    pause,  // 1
    stop,   // 2
    move,   // 3
    jump_forward,   // 4
    jump_backwards, // 5
    seek_repeat_self,   // 6
};

struct packet_header
{
    command_type    cmd;    // 명령
    uint32_t        size;   // 패킷 전체 크기
};

// play ////////////////////////////////
struct packet_play_from_client
{
    packet_header   header;
    RECT            rect;       // 좌표
    uint16_t        url_size;
    char            url[260];   // URL
};

struct packet_play_from_server
{
    packet_header   header;
    packet_result   result;     // 명령 수행 결과
    uint32_t        scene_index;
};
// --------------------------------

// --------------------------------
// pause ////////////////////////////////
struct packet_pause_from_client
{
    packet_header   header;
    uint32_t        scene_index;
};

struct packet_pause_from_server
{
    packet_header   header;
    packet_result   result;     // 명령 수행 결과
    uint32_t        scene_index;
};
// --------------------------------

// stop ////////////////////////////////
struct packet_stop_from_client
{
    packet_header   header;
    uint32_t        scene_index;
};

struct packet_stop_from_server
{
    packet_header   header;
    packet_result   result;     // 명령 수행 결과
    uint32_t        scene_index;
};
// --------------------------------

// move ////////////////////////////////
struct packet_move_from_client
{
    packet_header   header;
    uint32_t        scene_index;
    RECT            rect;   // 좌표
};

struct packet_move_from_server
{
    packet_header   header;
    packet_result   result; // 명령 수행 결과
    uint32_t        scene_index;
};
// --------------------------------

// jump forward ////////////////////////////////
struct packet_jump_forward_from_client
{
    packet_header   header;
    uint32_t        scene_index;
};

struct packet_jump_forward_from_server
{
    packet_header   header;
    packet_result   result; // 명령 수행 결과
    uint32_t        scene_index;
};
// --------------------------------

// jump backwards ////////////////////////////////
struct packet_jump_backwards_from_client
{
    packet_header   header;
    uint32_t        scene_index;
};

struct packet_jump_backwards_from_server
{
    packet_header   header;
    packet_result   result; // 명령 수행 결과
    uint32_t        scene_index;
};
// --------------------------------

// seek repeat self ////////////////////////////////
struct packet_seek_repeat_self
{
    packet_header   header;
    uint32_t        scene_index;
};

// --------------------------------
