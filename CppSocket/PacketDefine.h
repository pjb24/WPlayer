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

    invalid = u16_invalid_id
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
    play_sync_group,    // 7
    pause_sync_group,   // 8
    stop_sync_group,    // 9

    sync_group_frame_numbering,     // 10

    seek_repeat_self_sync_group,    // 11
    
    program_quit, // 12

    gplayer_play_url,   // 13
    gplayer_play_rect,  // 14
    gplayer_stop,   // 15
    gplayer_pause,  // 16

    gplayer_connect,    // 17
    gplayer_connect_data_url,    // 18
    gplayer_connect_data_rect,   // 19

    invalid = u16_invalid_id
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
    int             left;
    int             top;
    int             width;
    int             height;
    uint16_t        url_size;
    char            url[260];   // URL
};

struct packet_play_from_server
{
    packet_header   header;
    packet_result   result;     // 명령 수행 결과
    uint32_t        scene_index;
    int             left;
    int             top;
    int             width;
    int             height;
    uint16_t        url_size;
    char            url[260];
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
    int             left;
    int             top;
    int             width;
    int             height;
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

// play sync group ////////////////////////////////
struct packet_play_sync_group_from_client
{
    packet_header   header;
    int             left;
    int             top;
    int             width;
    int             height;
    uint32_t        sync_group_index;   // sync group 번호
    uint16_t        sync_group_count;   // sync group index에 해당하는 sync group에 소속될 scene 개수
    uint16_t        url_size;
    char            url[260];   // URL
};

struct packet_play_sync_group_from_server
{
    packet_header   header;
    packet_result   result;     // 명령 수행 결과
    uint32_t        scene_index;
    int             left;
    int             top;
    int             width;
    int             height;
    uint32_t        sync_group_index;   // sync group 번호
    uint16_t        sync_group_count;
    uint16_t        url_size;
    char            url[260];   // URL
};

// --------------------------------

// pause sync group ////////////////////////////////
struct packet_pause_sync_group_from_client
{
    packet_header   header;
    uint32_t        sync_group_index;   // sync group 번호
};

struct packet_pause_sync_group_from_server
{
    packet_header   header;
    packet_result   result;     // 명령 수행 결과
    uint32_t        sync_group_index;       // sync group 번호
};

// --------------------------------

// stop sync group ////////////////////////////////
struct packet_stop_sync_group_from_client
{
    packet_header   header;
    uint32_t        sync_group_index;   // sync group 번호
};

struct packet_stop_sync_group_from_server
{
    packet_header   header;
    packet_result   result;     // 명령 수행 결과
    uint32_t        sync_group_index;       // sync group 번호
};

// --------------------------------

// player_connect ////////////////////////////////
struct packet_player_connect_from_client
{
    packet_header header;

    uint32_t        player_sync_group_index;   // sync group 번호
};

struct packet_player_connect_from_server
{
    packet_header header;
    packet_result result;

    uint32_t        player_sync_group_index;   // sync group 번호
};

// --------------------------------

// gplayer_play_url ////////////////////////////////
struct packet_gplayer_play_url_from_client
{
    packet_header   header;
    
    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_input_count;   // player_sync_group_index에 해당하는 sync group의 재생 목록 개수
    
    uint16_t        url_size;
    char            url[260];   // URL
};

struct packet_gplayer_play_url_from_server
{
    packet_header   header;
    packet_result   result;     // 명령 수행 결과
    
    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_input_count;

    uint16_t        url_size;
    char            url[260];   // URL
};

// --------------------------------

// gplayer_play_rect ////////////////////////////////
struct packet_gplayer_play_rect_from_client
{
    packet_header   header;

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_output_count;   // player_sync_group_index에 해당하는 sync group의 출력 윈도우 개수

    int             left;
    int             top;
    int             width;
    int             height;
};

struct packet_gplayer_play_rect_from_server
{
    packet_header   header;
    packet_result   result;     // 명령 수행 결과
    
    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_output_count;

    int             left;
    int             top;
    int             width;
    int             height;
};

// --------------------------------

// gplayer_stop ////////////////////////////////
struct packet_gplayer_stop_from_client
{
    packet_header header;

    uint32_t        player_sync_group_index;   // sync group 번호
};

struct packet_gplayer_stop_from_server
{
    packet_header header;
    packet_result result;

    uint32_t        player_sync_group_index;   // sync group 번호
};

// --------------------------------

// gplayer_pause ////////////////////////////////
struct packet_gplayer_pause_from_client
{
    packet_header header;

};

struct packet_gplayer_pause_from_server
{
    packet_header header;
    packet_result result;

};

// --------------------------------

// gplayer_connect_data_url ////////////////////////////////
struct packet_gplayer_connect_data_url_from_client
{
    packet_header   header;

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_input_count;   // player_sync_group_index에 해당하는 sync group의 재생 목록 개수

    uint16_t        url_size;
    char            url[260];   // URL
};

struct packet_gplayer_connect_data_url_from_server
{
    packet_header   header;
    packet_result   result;     // 명령 수행 결과

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_input_count;

    uint16_t        url_size;
    char            url[260];   // URL
};

// --------------------------------

// gplayer_connect_data_rect ////////////////////////////////
struct packet_gplayer_connect_data_rect_from_client
{
    packet_header   header;

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_output_count;   // player_sync_group_index에 해당하는 sync group의 출력 윈도우 개수

    int             left;
    int             top;
    int             width;
    int             height;
};

struct packet_gplayer_connect_data_rect_from_server
{
    packet_header   header;
    packet_result   result;     // 명령 수행 결과

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_output_count;

    int             left;
    int             top;
    int             width;
    int             height;
};

// --------------------------------

