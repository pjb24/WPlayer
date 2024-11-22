#pragma once

#include "TypesDefine.h"

#include <dwrite.h>
#pragma comment(lib, "Dwrite")

enum class e_packet_type : uint16_t
{
    Invalid,        // 0
    ChatMessage,    // 1
    IntegerArray,   // 2
    structured_data_from_client, // 3
    structured_data_from_server, // 4
};

enum class e_packet_result : uint16_t
{
    ok,     // 0
    fail,   // 1
	pause,  // 2
    resume, // 3

    invalid = u16_invalid_id
};

enum class e_command_type : uint16_t
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


    dplayer_play_url,   // 20
    dplayer_play_rect,  // 21
    dplayer_stop,   // 22

    dplayer_connect,    // 23
    dplayer_connect_data_url,    // 24
    dplayer_connect_data_rect,   // 25

    gplayer_play_url_different_videos,   // 26
    gplayer_connect_data_url_different_videos,   // 27

    font_create,    // 28
    font_delete,    // 29

    invalid = u16_invalid_id
};

enum class e_dwrite_font_weight : int
{
    DWRITE_FONT_WEIGHT_THIN = 100,
    DWRITE_FONT_WEIGHT_EXTRA_LIGHT = 200,
    DWRITE_FONT_WEIGHT_ULTRA_LIGHT = 200,
    DWRITE_FONT_WEIGHT_LIGHT = 300,
    DWRITE_FONT_WEIGHT_SEMI_LIGHT = 350,
    DWRITE_FONT_WEIGHT_NORMAL = 400,
    DWRITE_FONT_WEIGHT_REGULAR = 400,
    DWRITE_FONT_WEIGHT_MEDIUM = 500,
    DWRITE_FONT_WEIGHT_DEMI_BOLD = 600,
    DWRITE_FONT_WEIGHT_SEMI_BOLD = 600,
    DWRITE_FONT_WEIGHT_BOLD = 700,
    DWRITE_FONT_WEIGHT_EXTRA_BOLD = 800,
    DWRITE_FONT_WEIGHT_ULTRA_BOLD = 800,
    DWRITE_FONT_WEIGHT_BLACK = 900,
    DWRITE_FONT_WEIGHT_HEAVY = 900,
    DWRITE_FONT_WEIGHT_EXTRA_BLACK = 950,
    DWRITE_FONT_WEIGHT_ULTRA_BLACK = 950
};

enum class e_dwrite_font_style : int
{
    DWRITE_FONT_STYLE_NORMAL = 0,
    DWRITE_FONT_STYLE_OBLIQUE = 1,
    DWRITE_FONT_STYLE_ITALIC = 2
};

enum class e_dwrite_font_stretch : int
{
    DWRITE_FONT_STRETCH_UNDEFINED = 0,
    DWRITE_FONT_STRETCH_ULTRA_CONDENSED = 1,
    DWRITE_FONT_STRETCH_EXTRA_CONDENSED = 2,
    DWRITE_FONT_STRETCH_CONDENSED = 3,
    DWRITE_FONT_STRETCH_SEMI_CONDENSED = 4,
    DWRITE_FONT_STRETCH_NORMAL = 5,
    DWRITE_FONT_STRETCH_MEDIUM = 5,
    DWRITE_FONT_STRETCH_SEMI_EXPANDED = 6,
    DWRITE_FONT_STRETCH_EXPANDED = 7,
    DWRITE_FONT_STRETCH_EXTRA_EXPANDED = 8,
    DWRITE_FONT_STRETCH_ULTRA_EXPANDED = 9
};

enum class e_movement_type_horizontal : int
{
    none = 0,
    left = 1,
    right = 2,

};

enum class e_movement_type_vertical : int
{
    none = 0,
    top = 1,
    bottom = 2,

};

struct packet_header
{
    e_command_type    cmd;    // 명령
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
    e_packet_result   result;     // 명령 수행 결과
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
    e_packet_result   result;     // 명령 수행 결과
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
    e_packet_result   result;     // 명령 수행 결과
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
    e_packet_result   result; // 명령 수행 결과
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
    e_packet_result   result; // 명령 수행 결과
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
    e_packet_result   result; // 명령 수행 결과
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
    e_packet_result   result;     // 명령 수행 결과
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
    e_packet_result   result;     // 명령 수행 결과
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
    e_packet_result   result;     // 명령 수행 결과
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
    e_packet_result result;

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
    e_packet_result   result;     // 명령 수행 결과
    
    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_input_count;

    uint16_t        url_size;
    char            url[260];   // URL
};

// --------------------------------

// gplayer_play_url_different_videos ////////////////////////////////
struct packet_gplayer_play_url_different_videos_from_client
{
    packet_header   header;

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_input_count;   // player_sync_group_index에 해당하는 sync group의 재생 목록 개수

    uint16_t        url_size;
    char            url[260];   // URL
};

struct packet_gplayer_play_url_different_videos_from_server
{
    packet_header   header;
    e_packet_result   result;     // 명령 수행 결과

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
    e_packet_result   result;     // 명령 수행 결과
    
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
    e_packet_result result;

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
    e_packet_result result;

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
    e_packet_result   result;     // 명령 수행 결과

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_input_count;

    uint16_t        url_size;
    char            url[260];   // URL
};

// --------------------------------

// gplayer_connect_data_url_different_videos ////////////////////////////////
struct packet_gplayer_connect_data_url_different_videos_from_client
{
    packet_header   header;

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_input_count;   // player_sync_group_index에 해당하는 sync group의 재생 목록 개수

    uint16_t        url_size;
    char            url[260];   // URL
};

struct packet_gplayer_connect_data_url_different_videos_from_server
{
    packet_header   header;
    e_packet_result   result;     // 명령 수행 결과

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
    e_packet_result   result;     // 명령 수행 결과

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_output_count;

    int             left;
    int             top;
    int             width;
    int             height;
};

// --------------------------------

// dplayer_play_url ////////////////////////////////
struct packet_dplayer_play_url_from_client
{
    packet_header   header;

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_input_count;   // player_sync_group_index에 해당하는 sync group의 재생 목록 개수

    uint32_t        scene_index;

    uint16_t        url_size;
    char            url[260];   // URL
};

struct packet_dplayer_play_url_from_server
{
    packet_header   header;
    e_packet_result   result;     // 명령 수행 결과

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_input_count;

    uint32_t        scene_index;

    uint16_t        url_size;
    char            url[260];   // URL
};

// --------------------------------

// dplayer_play_rect ////////////////////////////////
struct packet_dplayer_play_rect_from_client
{
    packet_header   header;

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_output_count;   // player_sync_group_index에 해당하는 sync group의 출력 윈도우 개수

    uint32_t        scene_index;

    int             left;
    int             top;
    int             width;
    int             height;
};

struct packet_dplayer_play_rect_from_server
{
    packet_header   header;
    e_packet_result   result;     // 명령 수행 결과

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_output_count;

    uint32_t        scene_index;

    int             left;
    int             top;
    int             width;
    int             height;
};

// --------------------------------

// dplayer_stop ////////////////////////////////
struct packet_dplayer_stop_from_client
{
    packet_header header;

    uint32_t        player_sync_group_index;   // sync group 번호
};

struct packet_dplayer_stop_from_server
{
    packet_header header;
    e_packet_result result;

    uint32_t        player_sync_group_index;   // sync group 번호
};

// --------------------------------

// dplayer_connect_data_url ////////////////////////////////
struct packet_dplayer_connect_data_url_from_client
{
    packet_header   header;

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_input_count;   // player_sync_group_index에 해당하는 sync group의 재생 목록 개수

    uint32_t        scene_index;    // url과 rect를 연결할 정보

    uint16_t        url_size;
    char            url[260];   // URL
};

struct packet_dplayer_connect_data_url_from_server
{
    packet_header   header;
    e_packet_result   result;     // 명령 수행 결과

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_input_count;

    uint32_t        scene_index;    // url과 rect를 연결할 정보

    uint16_t        url_size;
    char            url[260];   // URL
};

// --------------------------------

// dplayer_connect_data_rect ////////////////////////////////
struct packet_dplayer_connect_data_rect_from_client
{
    packet_header   header;

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_output_count;   // player_sync_group_index에 해당하는 sync group의 출력 윈도우 개수

    uint32_t        scene_index;

    int             left;
    int             top;
    int             width;
    int             height;
};

struct packet_dplayer_connect_data_rect_from_server
{
    packet_header   header;
    e_packet_result   result;     // 명령 수행 결과

    uint32_t        player_sync_group_index;   // sync group 번호
    uint16_t        player_sync_group_output_count;

    uint32_t        scene_index;

    int             left;
    int             top;
    int             width;
    int             height;
};

// --------------------------------

// font_create ////////////////////////////////
struct packet_font_create_from_client
{
    packet_header   header;

    uint32_t index_font;

    int font_size;

    int font_color_r;
    int font_color_g;
    int font_color_b;
    int font_color_a;

    int background_color_r;
    int background_color_g;
    int background_color_b;
    int background_color_a;

    int movement_type_horizontal;
    int movement_speed_horizontal;
    int movement_threshold_horizontal;

    int movement_type_vertical;
    int movement_speed_vertical;
    int movement_threshold_vertical;

    int font_start_coordinate_x;
    int font_start_coordinate_y;

    int background_rectangle_left;
    int backgound_rectangle_top;
    int backgound_rectangle_width;
    int backgound_rectangle_height;

    int font_weight;
    int font_style;
    int font_stretch;

    int content_size;
    char content_string[260];

    int font_family_size;
    char font_family[100];
};

struct packet_font_create_from_server
{
    packet_header   header;
    e_packet_result   result;     // 명령 수행 결과

    uint32_t index_font;
};

// --------------------------------

// font_delete ////////////////////////////////
struct packet_font_delete_from_client
{
    packet_header   header;

    uint32_t index_font;
};

struct packet_font_delete_from_server
{
    packet_header   header;
    e_packet_result   result;     // 명령 수행 결과

    uint32_t index_font;
};

// --------------------------------