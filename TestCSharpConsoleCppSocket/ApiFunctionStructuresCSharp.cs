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

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_player_connect
    {
        public UInt32 player_sync_group_index;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_gplayer_play_url
    {
        public UInt32 player_sync_group_index;
        public UInt16 player_sync_group_input_count;

        public UInt16 url_size;
        [MarshalAs(UnmanagedType.LPStr)]
        public string url;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_gplayer_play_rect
    {
        public UInt32 player_sync_group_index;
        public UInt16 player_sync_group_output_count;

        public int left;
        public int top;
        public int width;
        public int height;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_gplayer_stop
    {
        public UInt32 player_sync_group_index;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_dplayer_play_url
    {
        public UInt32 player_sync_group_index;
        public UInt16 player_sync_group_input_count;

        public UInt32 scene_index;

        public UInt16 url_size;
        [MarshalAs(UnmanagedType.LPStr)]
        public string url;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_dplayer_play_rect
    {
        public UInt32 player_sync_group_index;
        public UInt16 player_sync_group_output_count;

        public UInt32 scene_index;

        public int left;
        public int top;
        public int width;
        public int height;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_dplayer_stop
    {
        public UInt32 player_sync_group_index;
    }

    // --------------------------------
}
