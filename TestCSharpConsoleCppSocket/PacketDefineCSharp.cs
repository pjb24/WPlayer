using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TestCSharpConsoleCppSocket
{
    enum e_packet_result : UInt16
    {
        ok,     // 0
        fail,   // 1
        pause,  // 2
        resume, // 3

        invalid = UInt16.MaxValue
    };

    enum e_command_type : UInt16
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
        gplayer_connect_data_url,   // 18
        gplayer_connect_data_rect,  // 19


        dplayer_play_url,   // 20
        dplayer_play_rect,  // 21
        dplayer_stop,   // 22

        dplayer_connect,    // 23
        dplayer_connect_data_url,   // 24
        dplayer_connect_data_rect,  // 25

        gplayer_play_url_different_videos,   // 26
        gplayer_connect_data_url_different_videos,   // 27

        font_create,    // 28
        font_delete,    // 29

        font_blink_turn_on_off, // 30
        font_blink_interval,    // 31
        font_blink_duration,    // 32

        invalid = UInt16.MaxValue
    };

    // default: DWRITE_FONT_WEIGHT_NORMAL
    enum e_dwrite_font_weight : int
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

    // default: DWRITE_FONT_STYLE_NORMAL
    enum e_dwrite_font_style : int
    {
        DWRITE_FONT_STYLE_NORMAL = 0,
        DWRITE_FONT_STYLE_OBLIQUE = 1,
        DWRITE_FONT_STYLE_ITALIC = 2
    };

    // default: DWRITE_FONT_STRETCH_NORMAL
    enum e_dwrite_font_stretch : int
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

    enum e_movement_type_horizontal : int
    {
        none = 0,
        left = 1,
        right = 2,

    };

    enum e_movement_type_vertical : int
    {
        none = 0,
        top = 1,
        bottom = 2,

    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_header
    {
        public e_command_type cmd;    // 명령
        public UInt32 size;   // 패킷 전체 크기
    };


    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    struct packet_play_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과
        public UInt32 scene_index;
        public int left;
        public int top;
        public int width;
        public int height;
        public UInt16 url_size;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
        public string url;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_pause_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_stop_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_move_from_server
    {
        public packet_header header;
        public e_packet_result result; // 명령 수행 결과
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_jump_forward_from_server
    {
        public packet_header header;
        public e_packet_result result; // 명령 수행 결과
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_jump_backwards_from_server
    {
        public packet_header header;
        public e_packet_result result; // 명령 수행 결과
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    struct packet_play_sync_group_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과
        public UInt32 scene_index;
        public int left;
        public int top;
        public int width;
        public int height;
        public UInt32 sync_group_index;   // sync group 번호
        public UInt16 sync_group_count;
        public UInt16 url_size;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
        public string url;   // URL
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_pause_sync_group_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과
        public UInt32 sync_group_index;       // sync group 번호
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_stop_sync_group_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과
        public UInt32 sync_group_index;       // sync group 번호
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_gplayer_play_url_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과

        public UInt32 player_sync_group_index;       // sync group 번호
        public UInt16 player_sync_group_input_count;

        public UInt16 url_size;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
        public string url;   // URL
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_gplayer_play_url_different_videos_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과

        public UInt32 player_sync_group_index;       // sync group 번호
        public UInt16 player_sync_group_input_count;

        public UInt16 url_size;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
        public string url;   // URL
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_gplayer_play_rect_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과

        public UInt32 player_sync_group_index;       // sync group 번호
        public UInt16 player_sync_group_output_count;

        public int left;
        public int top;
        public int width;
        public int height;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_gplayer_stop_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과

        public UInt32 player_sync_group_index;       // sync group 번호
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_gplayer_pause_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_dplayer_play_url_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과

        public UInt32 player_sync_group_index;       // sync group 번호
        public UInt16 player_sync_group_input_count;

        public UInt32 scene_index;

        public UInt16 url_size;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
        public string url;   // URL
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_dplayer_play_rect_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과

        public UInt32 player_sync_group_index;       // sync group 번호
        public UInt16 player_sync_group_output_count;

        public UInt32 scene_index;

        public int left;
        public int top;
        public int width;
        public int height;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_dplayer_stop_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과

        public UInt32 player_sync_group_index;       // sync group 번호
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    struct packet_font_create_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과

        public UInt32 index_font;

        public int font_size;

        public int font_color_r;
        public int font_color_g;
        public int font_color_b;
        public int font_color_a;

        public int background_color_r;
        public int background_color_g;
        public int background_color_b;
        public int background_color_a;

        public int movement_type_horizontal;
        public int movement_speed_horizontal;
        public int movement_threshold_horizontal;

        public int movement_type_horizontal_background;
        public int movement_speed_horizontal_background;
        public int movement_threshold_horizontal_background;

        public int movement_type_vertical;
        public int movement_speed_vertical;
        public int movement_threshold_vertical;

        public int movement_type_vertical_background;
        public int movement_speed_vertical_background;
        public int movement_threshold_vertical_background;

        public int font_start_coordinate_left;
        public int font_start_coordinate_top;

        public int backgound_rectangle_width;
        public int backgound_rectangle_height;

        public int font_weight;
        public int font_style;
        public int font_stretch;

        public int content_size;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
        public string content_string;

        public int font_family_size;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 100)]
        public string font_family;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_font_delete_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과

        public UInt32 index_font;       // font 번호
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_font_blink_turn_on_off_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과

        public UInt32 index_font;       // font 번호

        public bool flag_blink_turn_on_off;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_font_blink_interval_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과

        public int interval_blink_in_miliseconds;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_font_blink_duration_from_server
    {
        public packet_header header;
        public e_packet_result result;     // 명령 수행 결과

        public int duration_blink_in_miliseconds;
    };
}
