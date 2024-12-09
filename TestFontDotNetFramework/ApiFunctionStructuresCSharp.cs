using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TestFontDotNetFramework
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void CALLBACK_DATA(IntPtr data);

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
}
