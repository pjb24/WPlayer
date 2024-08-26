using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TestCSharpConsoleCppSocket
{
    enum packet_result : UInt16
    {
        ok,     // 0
        fail,   // 1
        pause,  // 2
        resume, // 3

        invalid = UInt16.MaxValue
    };

    enum command_type : UInt16
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

        invalid = UInt16.MaxValue
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_header
    {
        public command_type cmd;    // 명령
        public UInt32 size;   // 패킷 전체 크기
    };


    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    struct packet_play_from_server
    {
        public packet_header header;
        public packet_result result;     // 명령 수행 결과
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
        public packet_result result;     // 명령 수행 결과
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_stop_from_server
    {
        public packet_header header;
        public packet_result result;     // 명령 수행 결과
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_move_from_server
    {
        public packet_header header;
        public packet_result result; // 명령 수행 결과
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_jump_forward_from_server
    {
        public packet_header header;
        public packet_result result; // 명령 수행 결과
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_jump_backwards_from_server
    {
        public packet_header header;
        public packet_result result; // 명령 수행 결과
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    struct packet_play_sync_group_from_server
    {
        public packet_header header;
        public packet_result result;     // 명령 수행 결과
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
        public packet_result result;     // 명령 수행 결과
        public UInt32 sync_group_index;       // sync group 번호
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_stop_sync_group_from_server
    {
        public packet_header header;
        public packet_result result;     // 명령 수행 결과
        public UInt32 sync_group_index;       // sync group 번호
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_gplayer_play_url_from_server
    {
        public packet_header header;
        public packet_result result;     // 명령 수행 결과

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
        public packet_result result;     // 명령 수행 결과

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
        public packet_result result;     // 명령 수행 결과

        public UInt32 player_sync_group_index;       // sync group 번호
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_gplayer_pause_from_server
    {
        public packet_header header;
        public packet_result result;     // 명령 수행 결과
    };

    [StructLayout(LayoutKind.Sequential)]
    struct packet_dplayer_play_url_from_server
    {
        public packet_header header;
        public packet_result result;     // 명령 수행 결과

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
        public packet_result result;     // 명령 수행 결과

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
        public packet_result result;     // 명령 수행 결과

        public UInt32 player_sync_group_index;       // sync group 번호
    };
}
