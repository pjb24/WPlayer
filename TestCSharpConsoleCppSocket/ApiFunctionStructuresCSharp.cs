using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TestCSharpConsoleCppSocket
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void CALLBACK_DATA(IntPtr data);

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

        invalid = UInt16.MaxValue
    };

    // client ////////////////////////////////

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    struct cppsocket_struct_client_send_play
    {
        public int left;
        public int top;
        public int width;
        public int height;
        [MarshalAs(UnmanagedType.LPStr)]
        public string url;
        public UInt16 url_size;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_pause
    {
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_stop
    {
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_move
    {
        public UInt32 scene_index;
        public int left;
        public int top;
        public int width;
        public int height;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_jump_forward
    {
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_jump_backwards
    {
        public UInt32 scene_index;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    struct cppsocket_struct_client_send_play_sync_group
    {
        public int left;
        public int top;
        public int width;
        public int height;
        [MarshalAs(UnmanagedType.LPStr)]
        public string url;
        public UInt16 url_size;
        public UInt32 sync_group_index;
        public UInt16 sync_group_count;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_pause_sync_group
    {
        public UInt32 sync_group_index;
    };

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_stop_sync_group
    {
        public UInt32 sync_group_index;
    };

    // --------------------------------

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
}
