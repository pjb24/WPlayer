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
    struct cppsocket_struct_client_send_gplayer_play_url_different_videos
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

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    struct cppsocket_struct_client_send_font_create
    {
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
        [MarshalAs(UnmanagedType.LPStr)]
        public string content_string;

        public int font_family_size;
        [MarshalAs(UnmanagedType.LPStr)]
        public string font_family;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_font_delete
    {
        public UInt32 index_font;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_font_blink_turn_on_off
    {
        public UInt32 index_font;

        public bool flag_blink_turn_on_off;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_font_blink_interval
    {
        public int interval_blink_in_miliseconds;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct cppsocket_struct_client_send_font_blink_duration
    {
        public int duration_blink_in_miliseconds;
    }

    // --------------------------------
}
