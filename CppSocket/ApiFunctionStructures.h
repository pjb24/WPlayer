#pragma once

#include <stdint.h>

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#include <Windows.h>

#include "PacketDefine.h"

// client ////////////////////////////////

struct cppsocket_struct_client_send_play
{
    int left;
    int top;
    int width;
    int height;
    const char* url;
    uint16_t url_size;
};

struct cppsocket_struct_client_send_pause
{
    uint32_t scene_index;
};

struct cppsocket_struct_client_send_stop
{
    uint32_t scene_index;
};

struct cppsocket_struct_client_send_move
{
    uint32_t scene_index;
    int left;
    int top;
    int width;
    int height;
};

struct cppsocket_struct_client_send_jump_forward
{
    uint32_t scene_index;
};

struct cppsocket_struct_client_send_jump_backwards
{
    uint32_t scene_index;
};

struct cppsocket_struct_client_send_play_sync_group
{
    int left;
    int top;
    int width;
    int height;
    const char* url;
    uint16_t url_size;
    uint32_t sync_group_index;
    uint16_t sync_group_count;
};

struct cppsocket_struct_client_send_pause_sync_group
{
    uint32_t sync_group_index;
};

struct cppsocket_struct_client_send_stop_sync_group
{
    uint32_t sync_group_index;
};

struct cppsocket_struct_client_send_player_connect
{
    uint32_t player_sync_group_index;
};

struct cppsocket_struct_client_send_gplayer_play_url
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_input_count;

    uint16_t url_size;
    const char* url;
};

struct cppsocket_struct_client_send_gplayer_play_url_different_videos
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_input_count;

    uint16_t url_size;
    const char* url;
};

struct cppsocket_struct_client_send_gplayer_play_rect
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_output_count;

    int left;
    int top;
    int width;
    int height;
};

struct cppsocket_struct_client_send_gplayer_stop
{
    uint32_t player_sync_group_index;
};

struct cppsocket_struct_client_send_dplayer_play_url
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_input_count;

    uint32_t scene_index;

    uint16_t url_size;
    const char* url;
};

struct cppsocket_struct_client_send_dplayer_play_rect
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_output_count;

    uint32_t scene_index;

    int left;
    int top;
    int width;
    int height;
};

struct cppsocket_struct_client_send_dplayer_stop
{
    uint32_t player_sync_group_index;
};

struct cppsocket_struct_client_send_font_create
{
    cppsocket_struct_client_send_font_create()
    {
        index_font = 0;

        content_string = "Font_Test";
        content_size = 9;

        font_family = "Arial";
        font_family_size = 5;

        font_size = 20;

        font_color_r = 255;
        font_color_g = 255;
        font_color_b = 255;
        font_color_a = 255;

        background_color_r = 0;
        background_color_g = 0;
        background_color_b = 0;
        background_color_a = 0;

        movement_type_horizontal = (int)e_movement_type_horizontal::none;
        movement_speed_horizontal = 0;
        movement_threshold_horizontal = INT32_MIN;

        movement_type_horizontal_background = (int)e_movement_type_horizontal::none;
        movement_speed_horizontal_background = 0;
        movement_threshold_horizontal_background = INT32_MIN;

        movement_type_vertical = (int)e_movement_type_vertical::none;
        movement_speed_vertical = 0;
        movement_threshold_vertical = INT32_MIN;

        movement_type_vertical_background = (int)e_movement_type_vertical::none;
        movement_speed_vertical_background = 0;
        movement_threshold_vertical_background = INT32_MIN;

        font_start_coordinate_left = 0;
        font_start_coordinate_top = 0;

        backgound_rectangle_width = 0;
        backgound_rectangle_height = 0;

        font_weight = (int)e_dwrite_font_weight::DWRITE_FONT_WEIGHT_NORMAL;
        font_style = (int)e_dwrite_font_style::DWRITE_FONT_STYLE_NORMAL;
        font_stretch = (int)e_dwrite_font_stretch::DWRITE_FONT_STRETCH_NORMAL;
    }

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

    int movement_type_horizontal_background;
    int movement_speed_horizontal_background;
    int movement_threshold_horizontal_background;

    int movement_type_vertical;
    int movement_speed_vertical;
    int movement_threshold_vertical;

    int movement_type_vertical_background;
    int movement_speed_vertical_background;
    int movement_threshold_vertical_background;

    int font_start_coordinate_left;
    int font_start_coordinate_top;

    int backgound_rectangle_width;
    int backgound_rectangle_height;

    int font_weight;
    int font_style;
    int font_stretch;

    int content_size;
    const char* content_string;

    int font_family_size;
    const char* font_family;
};

struct cppsocket_struct_client_send_font_delete
{
    uint32_t index_font;
};

struct cppsocket_struct_client_send_font_blink_turn_on_off
{
    uint32_t index_font;

    bool flag_blink_turn_on_off;
};

struct cppsocket_struct_client_send_font_blink_interval
{
    int interval_blink_in_miliseconds;
};

struct cppsocket_struct_client_send_font_blink_duration
{
    int duration_blink_in_miliseconds;
};

// --------------------------------

// server ////////////////////////////////

struct cppsocket_struct_server_send_play
{
    uint32_t scene_index;
    uint16_t result;
    int left;
    int top;
    int width;
    int height;
    char url[260];
    uint16_t url_size;
};

struct cppsocket_struct_server_send_pause
{
    uint32_t scene_index;
    uint16_t result;
};

struct cppsocket_struct_server_send_stop
{
    uint32_t scene_index;
    uint16_t result;
};

struct cppsocket_struct_server_send_move
{
    uint32_t scene_index;
    uint16_t result;
};

struct cppsocket_struct_server_send_jump_forward
{
    uint32_t scene_index;
    uint16_t result;
};

struct cppsocket_struct_server_send_jump_backwards
{
    uint32_t scene_index;
    uint16_t result;
};

struct cppsocket_struct_server_send_play_sync_group
{
    uint32_t scene_index;
    uint16_t result;
    int left;
    int top;
    int width;
    int height;
    char url[260];
    uint16_t url_size;
    uint32_t sync_group_index;
    uint16_t sync_group_count;
};

struct cppsocket_struct_server_send_pause_sync_group
{
    uint32_t sync_group_index;
    uint16_t result;
};

struct cppsocket_struct_server_send_stop_sync_group
{
    uint32_t sync_group_index;
    uint16_t result;
};

struct cppsocket_struct_server_send_gplayer_play_url
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_input_count;
    
    uint16_t result;

    uint16_t url_size;
    char url[260];
};

struct cppsocket_struct_server_send_gplayer_play_url_different_videos
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_input_count;

    uint16_t result;

    uint16_t url_size;
    char url[260];
};

struct cppsocket_struct_server_send_gplayer_play_rect
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_output_count;

    uint16_t result;
    
    int left;
    int top;
    int width;
    int height;
};

struct cppsocket_struct_server_send_gplayer_connect_data_url
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_input_count;

    uint16_t result;

    uint16_t url_size;
    char url[260];
};

struct cppsocket_struct_server_send_gplayer_connect_data_url_different_videos
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_input_count;

    uint16_t result;

    uint16_t url_size;
    char url[260];
};

struct cppsocket_struct_server_send_gplayer_connect_data_rect
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_output_count;

    uint16_t result;

    int left;
    int top;
    int width;
    int height;
};

struct cppsocket_struct_server_send_gplayer_stop
{
    uint16_t result;

    uint32_t player_sync_group_index;
};

struct cppsocket_struct_server_send_dplayer_play_url
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_input_count;

    uint32_t scene_index;

    uint16_t result;

    uint16_t url_size;
    char url[260];
};

struct cppsocket_struct_server_send_dplayer_play_rect
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_output_count;

    uint16_t result;

    uint32_t scene_index;

    int left;
    int top;
    int width;
    int height;
};

struct cppsocket_struct_server_send_dplayer_connect_data_url
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_input_count;

    uint32_t scene_index;

    uint16_t result;

    uint16_t url_size;
    char url[260];
};

struct cppsocket_struct_server_send_dplayer_connect_data_rect
{
    uint32_t player_sync_group_index;
    uint16_t player_sync_group_output_count;

    uint32_t scene_index;

    uint16_t result;

    int left;
    int top;
    int width;
    int height;
};

struct cppsocket_struct_server_send_dplayer_stop
{
    uint16_t result;

    uint32_t player_sync_group_index;
};

struct cppsocket_struct_server_send_font_create
{
    uint16_t result;

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

    int movement_type_horizontal_background;
    int movement_speed_horizontal_background;
    int movement_threshold_horizontal_background;

    int movement_type_vertical;
    int movement_speed_vertical;
    int movement_threshold_vertical;

    int movement_type_vertical_background;
    int movement_speed_vertical_background;
    int movement_threshold_vertical_background;

    int font_start_coordinate_left;
    int font_start_coordinate_top;

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

struct cppsocket_struct_server_send_font_delete
{
    uint16_t result;

    uint32_t index_font;
};

struct cppsocket_struct_server_send_font_blink_turn_on_off
{
    uint16_t result;

    uint32_t index_font;

    bool flag_blink_turn_on_off;
};

struct cppsocket_struct_server_send_font_blink_interval
{
    uint16_t result;

    int interval_blink_in_miliseconds;
};

struct cppsocket_struct_server_send_font_blink_duration
{
    uint16_t result;

    int duration_blink_in_miliseconds;
};

// --------------------------------
