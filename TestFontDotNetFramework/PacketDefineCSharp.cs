using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TestFontDotNetFramework
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
        font_create = 28,    // 28
        font_delete = 29,    // 29

        font_blink_turn_on_off = 30, // 30
        font_blink_interval = 31,    // 31
        font_blink_duration = 32,    // 32

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
