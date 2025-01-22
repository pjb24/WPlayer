// TestDllConsole.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>

#include "CppSocketAPI.h"
#include "PacketDefine.h"
#include "ApiFunctionStructures.h"

#include "thread"

#include "tchar.h"

static void* _server;
static void* _client;

bool _use_ini_setting = false;
int _type = 1;
std::string _ip;
uint16_t _port = 0;
int _left = 0;
int _top = 0;
int _width = 0;
int _height = 0;
std::string _url;


void callback_data_connection_server(void* data, void* connection)
{
    packet_header* header = (packet_header*)data;

    switch (header->cmd)
    {
    case e_command_type::play:
    {
        packet_play_from_client* packet = new packet_play_from_client();
        memcpy(packet, data, header->size);

        std::cout << "play" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "left: " << packet->left << ", ";
        std::cout << "top: " << packet->top << ", ";
        std::cout << "width: " << packet->width << ", ";
        std::cout << "height: " << packet->height << ", ";
        std::cout << "url_size: " << packet->url_size << ", ";
        std::cout << "url: " << packet->url << std::endl;

        cppsocket_struct_server_send_play data{};
        data.scene_index = 0;
        data.result = (uint16_t)e_packet_result::ok;
        data.left = packet->left;
        data.top = packet->top;
        data.width = packet->width;
        data.height = packet->height;
        data.url_size = packet->url_size;
        memcpy(data.url, packet->url, packet->url_size);

        cppsocket_server_send_play(_server, connection, data);

        delete packet;
    }
    break;
    case e_command_type::pause:
    {
        packet_pause_from_client* packet = new packet_pause_from_client();
        memcpy(packet, data, header->size);

        std::cout << "pause" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << std::endl;

        cppsocket_struct_server_send_pause data{};
        data.scene_index = packet->scene_index;
        data.result = (uint16_t)e_packet_result::ok;

        cppsocket_server_send_pause(_server, connection, data);

        delete packet;
    }
    break;
    case e_command_type::stop:
    {
        packet_stop_from_client* packet = new packet_stop_from_client();
        memcpy(packet, data, header->size);

        std::cout << "stop" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << std::endl;

        cppsocket_struct_server_send_stop data{};
        data.scene_index = packet->scene_index;
        data.result = (uint16_t)e_packet_result::ok;

        cppsocket_server_send_stop(_server, connection, data);

        delete packet;
    }
    break;
    case e_command_type::move:
    {
        // not implemented
    }
    break;
    case e_command_type::jump_forward:
    {
        packet_jump_forward_from_client* packet = new packet_jump_forward_from_client();
        memcpy(packet, data, header->size);

        std::cout << "jump_forwrad" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << std::endl;

        cppsocket_struct_server_send_jump_forward data{};
        data.scene_index = packet->scene_index;
        data.result = (uint16_t)e_packet_result::ok;

        cppsocket_server_send_jump_forward(_server, connection, data);

        delete packet;
    }
    break;
    case e_command_type::jump_backwards:
    {
        packet_jump_backwards_from_client* packet = new packet_jump_backwards_from_client();
        memcpy(packet, data, header->size);

        std::cout << "jump_backwards" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << std::endl;

        cppsocket_struct_server_send_jump_backwards data{};
        data.scene_index = packet->scene_index;
        data.result = (uint16_t)e_packet_result::ok;

        cppsocket_server_send_jump_backwards(_server, connection, data);

        delete packet;
    }
    break;
    case e_command_type::play_sync_group:
    {
        packet_play_sync_group_from_client* packet = new packet_play_sync_group_from_client();
        memcpy(packet, data, header->size);

        std::cout << "play_sync_group" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "left: " << packet->left << ", ";
        std::cout << "top: " << packet->top << ", ";
        std::cout << "width: " << packet->width << ", ";
        std::cout << "height: " << packet->height << ", ";
        std::cout << "url_size: " << packet->url_size << ", ";
        std::cout << "url: " << packet->url << ", ";
        std::cout << "sync_group_index: " << packet->sync_group_index << ", ";
        std::cout << "sync_group_count: " << packet->sync_group_count << std::endl;

        cppsocket_struct_server_send_play_sync_group data{};
        data.scene_index = 0;
        data.result = (uint16_t)e_packet_result::ok;
        data.left = packet->left;
        data.top = packet->top;
        data.width = packet->width;
        data.height = packet->height;
        data.url_size = packet->url_size;
        memcpy(data.url, packet->url, packet->url_size);
        data.sync_group_index = packet->sync_group_index;
        data.sync_group_count = packet->sync_group_count;

        cppsocket_server_send_play_sync_group(_server, connection, data);

        delete packet;
    }
    break;
    case e_command_type::pause_sync_group:
    {
        packet_pause_sync_group_from_client* packet = new packet_pause_sync_group_from_client();
        memcpy(packet, data, header->size);

        std::cout << "pause_sync_group" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "sync_group_index: " << packet->sync_group_index << std::endl;

        cppsocket_struct_server_send_pause_sync_group data{};
        data.sync_group_index = packet->sync_group_index;
        data.result = (uint16_t)e_packet_result::ok;

        cppsocket_server_send_pause_sync_group(_server, connection, data);

        delete packet;
    }
    break;
    case e_command_type::stop_sync_group:
    {
        packet_stop_sync_group_from_client* packet = new packet_stop_sync_group_from_client();
        memcpy(packet, data, header->size);

        std::cout << "stop_sync_group" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "sync_group_index: " << packet->sync_group_index << std::endl;

        cppsocket_struct_server_send_stop_sync_group data{};
        data.sync_group_index = packet->sync_group_index;
        data.result = (uint16_t)e_packet_result::ok;

        cppsocket_server_send_stop_sync_group(_server, connection, data);

        delete packet;
    }
    break;
    case e_command_type::program_quit:
    {
        std::cout << "program_quit" << std::endl;
    }
    break;
    default:
        break;
    }
}

void callback_ptr_client(void* data)
{
    packet_header* header = (packet_header*)data;

    switch (header->cmd)
    {
    case e_command_type::play:
    {
        packet_play_from_server* packet = new packet_play_from_server();
        memcpy(packet, data, header->size);

        std::cout << "play" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "left: " << packet->left << ", ";
        std::cout << "top: " << packet->top << ", ";
        std::cout << "width: " << packet->width << ", ";
        std::cout << "height: " << packet->height << ", ";
        std::cout << "url_size: " << packet->url_size << ", ";
        std::cout << "url: " << packet->url << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::pause:
    {
        packet_pause_from_server* packet = new packet_pause_from_server();
        memcpy(packet, data, header->size);

        std::cout << "pause" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::stop:
    {
        packet_stop_from_server* packet = new packet_stop_from_server();
        memcpy(packet, data, header->size);

        std::cout << "stop" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::move:
    {
        // not implemented
    }
    break;
    case e_command_type::jump_forward:
    {
        packet_jump_forward_from_server* packet = new packet_jump_forward_from_server();
        memcpy(packet, data, header->size);

        std::cout << "jump_forward" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::jump_backwards:
    {
        packet_jump_backwards_from_server* packet = new packet_jump_backwards_from_server();
        memcpy(packet, data, header->size);

        std::cout << "jump_backwards" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::play_sync_group:
    {
        packet_play_sync_group_from_server* packet = new packet_play_sync_group_from_server();
        memcpy(packet, data, header->size);

        std::cout << "play_sync_group" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "left: " << packet->left << ", ";
        std::cout << "top: " << packet->top << ", ";
        std::cout << "width: " << packet->width << ", ";
        std::cout << "height: " << packet->height << ", ";
        std::cout << "sync_group_index: " << packet->sync_group_index << ", ";
        std::cout << "sync_group_count: " << packet->sync_group_count << ", ";
        std::cout << "url_size: " << packet->url_size << ", ";
        std::cout << "url: " << packet->url << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::pause_sync_group:
    {
        packet_pause_sync_group_from_server* packet = new packet_pause_sync_group_from_server();
        memcpy(packet, data, header->size);

        std::cout << "pause_sync_group" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "sync_group_index: " << packet->sync_group_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::stop_sync_group:
    {
        packet_stop_sync_group_from_server* packet = new packet_stop_sync_group_from_server();
        memcpy(packet, data, header->size);

        std::cout << "stop_sync_group" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "sync_group_index: " << packet->sync_group_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;

    case e_command_type::gplayer_play_url:
    {
        packet_gplayer_play_url_from_server* packet = new packet_gplayer_play_url_from_server();
        memcpy(packet, data, header->size);

        std::cout << "gplayer_play_url" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "player_sync_group_input_count: " << packet->player_sync_group_input_count << ", ";
        std::cout << "url: " << packet->url << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::gplayer_play_url_different_videos:
    {
        packet_gplayer_play_url_different_videos_from_server* packet = new packet_gplayer_play_url_different_videos_from_server();
        memcpy(packet, data, header->size);

        std::cout << "gplayer_play_url_different_videos" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "player_sync_group_input_count: " << packet->player_sync_group_input_count << ", ";
        std::cout << "url: " << packet->url << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::gplayer_play_rect:
    {
        packet_gplayer_play_rect_from_server* packet = new packet_gplayer_play_rect_from_server();
        memcpy(packet, data, header->size);

        std::cout << "gplayer_play_rect" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "player_sync_group_output_count: " << packet->player_sync_group_output_count << ", ";
        std::cout << "left: " << packet->left << ", ";
        std::cout << "top: " << packet->top << ", ";
        std::cout << "width: " << packet->width << ", ";
        std::cout << "height: " << packet->height << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::gplayer_stop:
    {
        packet_gplayer_stop_from_server* packet = new packet_gplayer_stop_from_server();
        memcpy(packet, data, header->size);

        std::cout << "gplayer_stop" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;

    case e_command_type::dplayer_play_url:
    {
        packet_dplayer_play_url_from_server* packet = new packet_dplayer_play_url_from_server();
        memcpy(packet, data, header->size);

        std::cout << "dplayer_play_url" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "player_sync_group_input_count: " << packet->player_sync_group_input_count << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "url: " << packet->url << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::dplayer_play_rect:
    {
        packet_dplayer_play_rect_from_server* packet = new packet_dplayer_play_rect_from_server();
        memcpy(packet, data, header->size);

        std::cout << "dplayer_play_rect" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "player_sync_group_output_count: " << packet->player_sync_group_output_count << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "left: " << packet->left << ", ";
        std::cout << "top: " << packet->top << ", ";
        std::cout << "width: " << packet->width << ", ";
        std::cout << "height: " << packet->height << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::dplayer_stop:
    {
        packet_dplayer_stop_from_server* packet = new packet_dplayer_stop_from_server();
        memcpy(packet, data, header->size);

        std::cout << "dplayer_stop" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;

    case e_command_type::font_create:
    {
        packet_font_create_from_server* packet = new packet_font_create_from_server();
        memcpy(packet, data, header->size);

        std::cout << "font_create" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "index_font: " << packet->index_font << ", ";
        std::cout << "font_size: " << packet->font_size << ", ";
        std::cout << "font_color_r: " << packet->font_color_r << ", ";
        std::cout << "font_color_g: " << packet->font_color_g << ", ";
        std::cout << "font_color_b: " << packet->font_color_b << ", ";
        std::cout << "font_color_a: " << packet->font_color_a << ", ";
        std::cout << "background_color_r: " << packet->background_color_r << ", ";
        std::cout << "background_color_g: " << packet->background_color_g << ", ";
        std::cout << "background_color_b: " << packet->background_color_b << ", ";
        std::cout << "background_color_a: " << packet->background_color_a << ", ";
        std::cout << "movement_type_horizontal: " << packet->movement_type_horizontal << ", ";
        std::cout << "movement_speed_horizontal: " << packet->movement_speed_horizontal << ", ";
        std::cout << "movement_threshold_horizontal: " << packet->movement_threshold_horizontal << ", ";
        std::cout << "movement_type_horizontal_background: " << packet->movement_type_horizontal_background << ", ";
        std::cout << "movement_speed_horizontal_background: " << packet->movement_speed_horizontal_background << ", ";
        std::cout << "movement_threshold_horizontal_background: " << packet->movement_threshold_horizontal_background << ", ";
        std::cout << "movement_type_vertical: " << packet->movement_type_vertical << ", ";
        std::cout << "movement_speed_vertical: " << packet->movement_speed_vertical << ", ";
        std::cout << "movement_threshold_vertical: " << packet->movement_threshold_vertical << ", ";
        std::cout << "movement_type_vertical_background: " << packet->movement_type_vertical_background << ", ";
        std::cout << "movement_speed_vertical_background: " << packet->movement_speed_vertical_background << ", ";
        std::cout << "movement_threshold_vertical_background: " << packet->movement_threshold_vertical_background << ", ";
        std::cout << "font_start_coordinate_left: " << packet->font_start_coordinate_left << ", ";
        std::cout << "font_start_coordinate_top: " << packet->font_start_coordinate_top << ", ";
        std::cout << "backgound_rectangle_width: " << packet->backgound_rectangle_width << ", ";
        std::cout << "backgound_rectangle_height: " << packet->backgound_rectangle_height << ", ";
        std::cout << "font_weight: " << packet->font_weight << ", ";
        std::cout << "font_style: " << packet->font_style << ", ";
        std::cout << "font_stretch: " << packet->font_stretch << ", ";
        std::cout << "content_size: " << packet->content_size << ", ";
        std::cout << "content_string: " << packet->content_string << ", ";
        std::cout << "font_family_size: " << packet->font_family_size << ", ";
        std::cout << "font_family: " << packet->font_family << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::font_delete:
    {
        packet_font_delete_from_server* packet = new packet_font_delete_from_server();
        memcpy(packet, data, header->size);

        std::cout << "font_delete" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "index_font: " << packet->index_font << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::font_blink_turn_on_off:
    {
        packet_font_blink_turn_on_off_from_server* packet = new packet_font_blink_turn_on_off_from_server();
        memcpy(packet, data, header->size);

        std::cout << "font_blink_turn_on_off" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "index_font: " << packet->index_font << ", ";
        std::cout << "flag_blink_turn_on_off: " << packet->flag_blink_turn_on_off << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::font_blink_interval:
    {
        packet_font_blink_interval_from_server* packet = new packet_font_blink_interval_from_server();
        memcpy(packet, data, header->size);

        std::cout << "font_blink_interval" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "interval_blink_in_miliseconds: " << packet->interval_blink_in_miliseconds << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::font_blink_duration:
    {
        packet_font_blink_duration_from_server* packet = new packet_font_blink_duration_from_server();
        memcpy(packet, data, header->size);

        std::cout << "font_blink_duration" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "duration_blink_in_miliseconds: " << packet->duration_blink_in_miliseconds << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;

    case e_command_type::cef_create:
    {
        packet_cef_create_from_server* packet = new packet_cef_create_from_server();
        memcpy(packet, data, header->size);

        std::cout << "cef_create" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "index_cef: " << packet->index_cef << ", ";
        std::cout << "left: " << packet->left << ", ";
        std::cout << "top: " << packet->top << ", ";
        std::cout << "width: " << packet->width << ", ";
        std::cout << "height: " << packet->height << ", ";
        std::cout << "url_size: " << packet->url_size << ", ";
        std::cout << "url: " << packet->url << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case e_command_type::cef_delete:
    {
        packet_cef_delete_from_server* packet = new packet_cef_delete_from_server();
        memcpy(packet, data, header->size);

        std::cout << "cef_create" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "index_cef: " << packet->index_cef << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;

    default:
        break;
    }
}

void client_frame_thread(const char * ip, uint16_t port)
{
    // Client
    void* client = cppsocket_client_create();

    cppsocket_client_set_callback_data(client, callback_ptr_client);

    if (cppsocket_client_connect(client, ip, port))
    {
        _client = client;

        while (cppsocket_client_is_connected(client))
        {
            cppsocket_client_frame(client);
        }
    }
    cppsocket_client_delete(client);
}

void client_output_messages_step_1()
{
    std::cout << "command list" << std::endl;
    std::cout << "0(play)" << std::endl;
    std::cout << "1(pause)" << std::endl;
    std::cout << "2(stop)" << std::endl;
    std::cout << "3(move) (not implemented)" << std::endl;
    std::cout << "4(jump_forward)" << std::endl;
    std::cout << "5(jump_backwards)" << std::endl;
    std::cout << "6(seek_repeat_self) (internal command type, not implemented)" << std::endl;
    std::cout << "7(play_sync_group)" << std::endl;
    std::cout << "8(pause_sync_group)" << std::endl;
    std::cout << "9(stop_sync_group)" << std::endl;
    std::cout << "10(sync_group_frame_numbering) (internal command type, not implemented)" << std::endl;
    std::cout << "11(seek_repeat_self_sync_Group) (internal command type, not implemented)" << std::endl;
    std::cout << "12(program_quit)" << std::endl;
    std::cout << std::endl;
    std::cout << "13(gplayer_play_url)" << std::endl;
    std::cout << "14(gplayer_play_rect)" << std::endl;
    std::cout << "15(gplayer_play_stop)" << std::endl;
    std::cout << "16(gplayer_play_pause) (not implemented)" << std::endl;
    std::cout << std::endl;
    std::cout << "20(dplayer_play_url)" << std::endl;
    std::cout << "21(dplayer_play_rect)" << std::endl;
    std::cout << "22(dplayer_play_stop)" << std::endl;
    std::cout << std::endl;
    std::cout << "26(gplayer_play_url_different_videos)" << std::endl;
    std::cout << std::endl;
    std::cout << "28(font_create)" << std::endl;
    std::cout << "29(font_delete)" << std::endl;
    std::cout << "30(font_blink_turn_on_off)" << std::endl;
    std::cout << "31(font_blink_interval)" << std::endl;
    std::cout << "32(font_blink_duration)" << std::endl;
    std::cout << std::endl;
    std::cout << "33(cef_create)" << std::endl;
    std::cout << "34(cef_delete)" << std::endl;

    std::cout << std::endl;
    std::cout << "input 99 to stop program" << std::endl;
}

void config_setting()
{
    wchar_t path_w[260] = { 0, };
    GetModuleFileName(nullptr, path_w, 260);
    std::wstring str_path_w = path_w;
    str_path_w = str_path_w.substr(0, str_path_w.find_last_of(L"\\/"));
    std::wstring str_ini_path_w = str_path_w + L"\\TestDllConsole.ini";

    char path_a[260] = { 0, };
    GetModuleFileNameA(nullptr, path_a, 260);
    std::string str_path_a = path_a;
    str_path_a = str_path_a.substr(0, str_path_a.find_last_of("\\/"));
    std::string str_ini_path_a = str_path_a + "\\TestDllConsole.ini";

    char result_a[255];
    wchar_t result_w[255];
    int result_i = 0;

    GetPrivateProfileString(L"TestDllConsole", L"USE_INI_SETTING", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _use_ini_setting = result_i == 0 ? false : true;

    GetPrivateProfileString(L"TestDllConsole", L"TYPE", L"-1", result_w, 255, str_ini_path_w.c_str());
    _type = _ttoi(result_w);

    GetPrivateProfileStringA("TestDllConsole", "IP", "", result_a, 255, str_ini_path_a.c_str());
    _ip = result_a;

    GetPrivateProfileString(L"TestDllConsole", L"PORT", L"0", result_w, 255, str_ini_path_w.c_str());
    _port = _ttoi(result_w);

    GetPrivateProfileString(L"TestDllConsole", L"Left", L"0", result_w, 255, str_ini_path_w.c_str());
    _left = _ttoi(result_w);

    GetPrivateProfileString(L"TestDllConsole", L"Top", L"0", result_w, 255, str_ini_path_w.c_str());
    _top = _ttoi(result_w);

    GetPrivateProfileString(L"TestDllConsole", L"Width", L"0", result_w, 255, str_ini_path_w.c_str());
    _width = _ttoi(result_w);

    GetPrivateProfileString(L"TestDllConsole", L"Height", L"0", result_w, 255, str_ini_path_w.c_str());
    _height = _ttoi(result_w);
    
    GetPrivateProfileStringA("TestDllConsole", "URL", "", result_a, 255, str_ini_path_a.c_str());
    _url = result_a;
}

int main()
{
    config_setting();

    int input = 0;
    
    if (_type < 0 || _use_ini_setting == false)
    {
        std::cout << "Input" << std::endl;
        std::cout << "0 : server start" << std::endl;
        std::cout << "1 : client start" << std::endl;

        std::cin >> input;
    }
    else
    {
        input = _type;
    }

    if (cppsocket_network_initialize())
    {
        std::cout << "Winsock api successfully initialized." << std::endl;
        
        std::string ip;
        uint16_t port = 0;

        if (_ip.empty() || _port == 0 || _use_ini_setting == false)
        {
            std::cout << "Enter Using ip and port. e.g) 127.0.0.1 53333" << std::endl;
            std::cin >> ip >> port;
        }
        else
        {
            ip = _ip;
            port = _port;
        }

        if (input == 0)
        {
            // Server
            void* server = cppsocket_server_create();
            
            cppsocket_server_set_callback_data_connection(server, callback_data_connection_server);

            if (cppsocket_server_initialize(server, ip.c_str(), port))
            {
                _server = server;
                while (true)
                {
                    cppsocket_server_frame(_server);
                }
            }
            cppsocket_server_delete(server);
        }
        else if (input == 1)
        {
            std::thread client_thread(client_frame_thread, ip.c_str(), port);
            client_thread.detach();

            bool auto_play = false;

            if (!(_left == 0 && _top == 0 && _width == 0 && _height == 0) && _use_ini_setting == true)
            {
                auto_play = true;
            }

            while (true)
            {
                uint16_t cmd;

                if (auto_play == true)
                {
                    auto_play = false;

                    while (!_client)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    while (!cppsocket_client_is_connected(_client))
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }

                    cppsocket_struct_client_send_play data{};
                    data.left = _left;
                    data.top = _top;
                    data.width = _width;
                    data.height = _height;
                    data.url_size = _url.size();
                    data.url = _url.c_str();

                    cppsocket_client_send_play(_client, data);

                    continue;
                }

                client_output_messages_step_1();
                std::cin >> cmd;
                
                switch ((e_command_type)cmd)
                {
                case e_command_type::play:
                {
                    std::cout << "play, Input left top width height url" << std::endl;

                    int left;
                    int top;
                    int width;
                    int height;
                    std::string url;
                    std::cin >> left >> top >> width >> height >> url;

                    cppsocket_struct_client_send_play data{};
                    data.left = left;
                    data.top = top;
                    data.width = width;
                    data.height = height;
                    data.url_size = url.size();
                    data.url = url.c_str();

                    cppsocket_client_send_play(_client, data);
                }
                break;
                case e_command_type::pause:
                {
                    std::cout << "pause, Input scene_index" << std::endl;

                    uint32_t scene_index;
                    std::cin >> scene_index;

                    cppsocket_struct_client_send_pause data{};
                    data.scene_index = scene_index;

                    cppsocket_client_send_pause(_client, data);
                }
                break;
                case e_command_type::stop:
                {
                    std::cout << "stop, Input scene_index" << std::endl;

                    uint32_t scene_index;
                    std::cin >> scene_index;

                    cppsocket_struct_client_send_stop data{};
                    data.scene_index = scene_index;

                    cppsocket_client_send_stop(_client, data);
                }
                break;
                case e_command_type::move:
                {
                    // not implemented
                }
                break;
                case e_command_type::jump_forward:
                {
                    std::cout << "jump_forward, Input scene_index" << std::endl;

                    uint32_t scene_index;
                    std::cin >> scene_index;

                    cppsocket_struct_client_send_jump_forward data{};
                    data.scene_index = scene_index;

                    cppsocket_client_send_jump_forward(_client, data);
                }
                break;
                case e_command_type::jump_backwards:
                {
                    std::cout << "jump_backwards, Input scene_index" << std::endl;

                    uint32_t scene_index;
                    std::cin >> scene_index;

                    cppsocket_struct_client_send_jump_backwards data{};
                    data.scene_index = scene_index;

                    cppsocket_client_send_jump_backwards(_client, data);
                }
                break;
                case e_command_type::play_sync_group:
                {
                    std::cout << "play, Input left top width height url sync_group_index sync_group_count" << std::endl;

                    int left;
                    int top;
                    int width;
                    int height;
                    std::string url;
                    uint32_t sync_group_index;
                    uint16_t sync_group_count;

                    std::cin >> left >> top >> width >> height >> url;
                    std::cin >> sync_group_index >> sync_group_count;

                    cppsocket_struct_client_send_play_sync_group data{};
                    data.left = left;
                    data.top = top;
                    data.width = width;
                    data.height = height;
                    data.url_size = url.size();
                    data.url = url.c_str();
                    data.sync_group_index = sync_group_index;
                    data.sync_group_count = sync_group_count;

                    cppsocket_client_send_play_sync_group(_client, data);
                }
                break;
                case e_command_type::pause_sync_group:
                {
                    std::cout << "pause, Input sync_group_index" << std::endl;

                    uint32_t sync_group_index;
                    std::cin >> sync_group_index;

                    cppsocket_struct_client_send_pause_sync_group data{};
                    data.sync_group_index = sync_group_index;

                    cppsocket_client_send_pause_sync_group(_client, data);
                }
                break;
                case e_command_type::stop_sync_group:
                {
                    std::cout << "stop, Input sync_group_index" << std::endl;

                    uint32_t sync_group_index;
                    std::cin >> sync_group_index;

                    cppsocket_struct_client_send_stop_sync_group data{};
                    data.sync_group_index = sync_group_index;

                    cppsocket_client_send_stop_sync_group(_client, data);
                }
                break;
                case e_command_type::program_quit:
                {
                    cppsocket_client_send_program_quit(_client);
                }
                break;

                case e_command_type::gplayer_play_url:
                {
                    std::cout << "gplayer_play_url, Input url player_sync_group_index player_sync_group_input_count" << std::endl;

                    std::string url;
                    uint32_t player_sync_group_index;
                    uint16_t player_sync_group_input_count;

                    std::cin >> url;
                    std::cin >> player_sync_group_index >> player_sync_group_input_count;

                    cppsocket_struct_client_send_gplayer_play_url data{};
                    data.url_size = url.size();
                    data.url = url.c_str();
                    data.player_sync_group_index = player_sync_group_index;
                    data.player_sync_group_input_count = player_sync_group_input_count;

                    cppsocket_client_send_gplayer_play_url(_client, data);
                }
                break;
                case e_command_type::gplayer_play_url_different_videos:
                {
                    std::cout << "gplayer_play_url_different_videos, Input url player_sync_group_index player_sync_group_input_count" << std::endl;

                    std::string url;
                    uint32_t player_sync_group_index;
                    uint16_t player_sync_group_input_count;

                    std::cin >> url;
                    std::cin >> player_sync_group_index >> player_sync_group_input_count;

                    cppsocket_struct_client_send_gplayer_play_url_different_videos data{};
                    data.url_size = url.size();
                    data.url = url.c_str();
                    data.player_sync_group_index = player_sync_group_index;
                    data.player_sync_group_input_count = player_sync_group_input_count;

                    cppsocket_client_send_gplayer_play_url_different_videos(_client, data);
                }
                break;
                case e_command_type::gplayer_play_rect:
                {
                    std::cout << "gplayer_play_rect, Input left top width height player_sync_group_index player_sync_group_output_count" << std::endl;

                    int left;
                    int top;
                    int width;
                    int height;
                    uint32_t player_sync_group_index;
                    uint16_t player_sync_group_output_count;

                    std::cin >> left >> top >> width >> height;
                    std::cin >> player_sync_group_index >> player_sync_group_output_count;

                    cppsocket_struct_client_send_gplayer_play_rect data{};
                    data.left = left;
                    data.top = top;
                    data.width = width;
                    data.height = height;
                    data.player_sync_group_index = player_sync_group_index;
                    data.player_sync_group_output_count = player_sync_group_output_count;

                    cppsocket_client_send_gplayer_play_rect(_client, data);
                }
                break;
                case e_command_type::gplayer_stop:
                {
                    std::cout << "gplayer_stop, Input player_sync_group_index" << std::endl;

                    uint32_t player_sync_group_index;

                    std::cin >> player_sync_group_index;

                    cppsocket_struct_client_send_gplayer_stop data{};
                    data.player_sync_group_index = player_sync_group_index;

                    cppsocket_client_send_gplayer_stop(_client, data);
                }
                break;

                case e_command_type::dplayer_play_url:
                {
                    std::cout << "dplayer_play_url, Input url player_sync_group_index player_sync_group_input_count scene_index" << std::endl;

                    std::string url;
                    uint32_t player_sync_group_index;
                    uint16_t player_sync_group_input_count;
                    uint32_t scene_index;

                    std::cin >> url;
                    std::cin >> player_sync_group_index >> player_sync_group_input_count >> scene_index;

                    cppsocket_struct_client_send_dplayer_play_url data{};
                    data.url_size = url.size();
                    data.url = url.c_str();
                    data.player_sync_group_index = player_sync_group_index;
                    data.player_sync_group_input_count = player_sync_group_input_count;
                    data.scene_index = scene_index;

                    cppsocket_client_send_dplayer_play_url(_client, data);
                }
                break;
                case e_command_type::dplayer_play_rect:
                {
                    std::cout << "dplayer_play_rect, Input left top width height player_sync_group_index player_sync_group_output_count scene_index" << std::endl;

                    int left;
                    int top;
                    int width;
                    int height;
                    uint32_t player_sync_group_index;
                    uint16_t player_sync_group_output_count;
                    uint32_t scene_index;

                    std::cin >> left >> top >> width >> height;
                    std::cin >> player_sync_group_index >> player_sync_group_output_count >> scene_index;

                    cppsocket_struct_client_send_dplayer_play_rect data{};
                    data.left = left;
                    data.top = top;
                    data.width = width;
                    data.height = height;
                    data.player_sync_group_index = player_sync_group_index;
                    data.player_sync_group_output_count = player_sync_group_output_count;
                    data.scene_index = scene_index;

                    cppsocket_client_send_dplayer_play_rect(_client, data);
                }
                break;
                case e_command_type::dplayer_stop:
                {
                    std::cout << "dplayer_stop, Input player_sync_group_index" << std::endl;

                    uint32_t player_sync_group_index;

                    std::cin >> player_sync_group_index;

                    cppsocket_struct_client_send_dplayer_stop data{};
                    data.player_sync_group_index = player_sync_group_index;

                    cppsocket_client_send_dplayer_stop(_client, data);
                }
                break;

                case e_command_type::font_create:
                {
                    uint32_t index_font;

                    std::cout << "font_create, Input index_font" << std::endl;
                    std::cin >> index_font;

                    // content_string
#pragma region content_string
                    std::string content_string = "Font_Test";
                    int content_size = content_string.size();
                    
                    std::cout << "font_create, Input content_string" << std::endl;
                    std::cin >> content_string;
                    content_size = content_string.size();
#pragma endregion         
                    // font_family
#pragma region font_family
                    std::string font_family = "Arial";
                    int font_family_size = font_family.size();

                    std::cout << "font_create, Input font_family, If Input -1, font_family use default value" << std::endl;

                    std::cin >> font_family;
                    if (font_family == "-1")
                    {
                        font_family = "Arial";
                    }
                    font_family_size = font_family.size();
#pragma endregion
                    // font_size
#pragma region font_size
                    int font_size = 32;

                    std::cout << "font_create, Input font_size, If Input -1, font_size use default value" << std::endl;

                    std::cin >> font_size;
                    if (font_size == -1)
                    {
                        font_size = 32;
                    }
#pragma endregion
                    // font_color
#pragma region font_color
                    bool flag_input_minus_one_font_color = false;

                    int font_color_r = 255;
                    int font_color_g = 255;
                    int font_color_b = 255;
                    int font_color_a = 255;

                    if (flag_input_minus_one_font_color == false)
                    {
                        std::cout << "font_create, Input font_color_r font_color_g font_color_b font_color_a, If Input -1, font_color use default value" << std::endl;
                    }

                    if (flag_input_minus_one_font_color == false)
                    {
                        std::cin >> font_color_r;
                    }
                    if (font_color_r == -1)
                    {
                        flag_input_minus_one_font_color = true;
                    }

                    if (flag_input_minus_one_font_color == false)
                    {
                        std::cin >> font_color_g;
                    }
                    if (font_color_g == -1)
                    {
                        flag_input_minus_one_font_color = true;
                    }

                    if (flag_input_minus_one_font_color == false)
                    {
                        std::cin >> font_color_b;
                    }
                    if (font_color_b == -1)
                    {
                        flag_input_minus_one_font_color = true;
                    }

                    if (flag_input_minus_one_font_color == false)
                    {
                        std::cin >> font_color_a;
                    }
                    if (font_color_a == -1)
                    {
                        flag_input_minus_one_font_color = true;
                    }

                    if (flag_input_minus_one_font_color)
                    {
                        font_color_r = 255;
                        font_color_g = 255;
                        font_color_b = 255;
                        font_color_a = 255;
                    }
#pragma endregion
                    // background_color
#pragma region background_color
                    bool flag_input_minus_one_background_color = false;

                    int background_color_r = 0;
                    int background_color_g = 0;
                    int background_color_b = 0;
                    int background_color_a = 0;

                    if (flag_input_minus_one_background_color == false)
                    {
                        std::cout << "font_create, Input background_color_r background_color_g background_color_b background_color_a, If Input -1, background_color use default value" << std::endl;
                    }

                    if (flag_input_minus_one_background_color == false)
                    {
                        std::cin >> background_color_r;
                    }
                    if (background_color_r == -1)
                    {
                        flag_input_minus_one_background_color = true;
                    }

                    if (flag_input_minus_one_background_color == false)
                    {
                        std::cin >> background_color_g;
                    }
                    if (background_color_g == -1)
                    {
                        flag_input_minus_one_background_color = true;
                    }

                    if (flag_input_minus_one_background_color == false)
                    {
                        std::cin >> background_color_b;
                    }
                    if (background_color_b == -1)
                    {
                        flag_input_minus_one_background_color = true;
                    }

                    if (flag_input_minus_one_background_color == false)
                    {
                        std::cin >> background_color_a;
                    }
                    if (background_color_a == -1)
                    {
                        flag_input_minus_one_background_color = true;
                    }

                    if (flag_input_minus_one_background_color)
                    {
                        background_color_r = 0;
                        background_color_g = 0;
                        background_color_b = 0;
                        background_color_a = 0;
                    }
#pragma endregion
                    // movement_type_horizontal
#pragma region movement_type_horizontal
                    int movement_type_horizontal = (int)e_movement_type_horizontal::none;
                    int movement_speed_horizontal = 0;
                    int movement_threshold_horizontal = INT_MIN;

                    std::cout << "font_create, Input movement_type_horizontal, If Input 0: none, 1: move to left, 2: move to right" << std::endl;
                    std::cin >> movement_type_horizontal;

                    if (movement_type_horizontal != (int)e_movement_type_horizontal::none)
                    {
                        std::cout << "font_create, Input movement_speed_horizontal, If Input negative value, speed use default value, 0" << std::endl;
                        std::cin >> movement_speed_horizontal;
                        if (movement_speed_horizontal < 0)
                        {
                            movement_speed_horizontal = 0;
                        }
                    }

                    if (movement_speed_horizontal != 0)
                    {
                        std::cout << "font_create, Input movement_threshold_horizontal, Any number in the int range, but if enter a number that is too large, the font will not be visible for too long" << std::endl;
                        std::cin >> movement_threshold_horizontal;
                    }
#pragma endregion
                    // movement_type_horizontal_background
#pragma region movement_type_horizontal_background
                    int movement_type_horizontal_background = (int)e_movement_type_horizontal::none;
                    int movement_speed_horizontal_background = 0;
                    int movement_threshold_horizontal_background = INT_MIN;

                    std::cout << "font_create, Input movement_type_horizontal_background, If Input 0: none, 1: move to left, 2: move to right" << std::endl;
                    std::cin >> movement_type_horizontal_background;

                    if (movement_type_horizontal_background != (int)e_movement_type_horizontal::none)
                    {
                        std::cout << "font_create, Input movement_speed_horizontal_background, If Input negative value, speed use default value, 0" << std::endl;
                        std::cin >> movement_speed_horizontal_background;
                        if (movement_speed_horizontal_background < 0)
                        {
                            movement_speed_horizontal_background = 0;
                        }
                    }

                    if (movement_speed_horizontal_background != 0)
                    {
                        std::cout << "font_create, Input movement_threshold_horizontal_background, Any number in the int range, but if enter a number that is too large, the font will not be visible for too long" << std::endl;
                        std::cin >> movement_threshold_horizontal_background;
                    }
#pragma endregion
                    // movement_type_vertical
#pragma region movement_type_vertical
                    int movement_type_vertical = (int)e_movement_type_vertical::none;
                    int movement_speed_vertical = 0;
                    int movement_threshold_vertical = INT_MIN;

                    std::cout << "font_create, Input movement_type_vertical, If Input 0: none, 1: move to top, 2: move to bottom" << std::endl;
                    std::cin >> movement_type_vertical;

                    if (movement_type_vertical != (int)e_movement_type_vertical::none)
                    {
                        std::cout << "font_create, Input movement_speed_vertical, If Input negative value, speed use default value, 0" << std::endl;
                        std::cin >> movement_speed_vertical;
                        if (movement_speed_vertical < 0)
                        {
                            movement_speed_vertical = 0;
                        }
                    }

                    if (movement_speed_vertical != 0)
                    {
                        std::cout << "font_create, Input movement_threshold_vertical, Any number in the int range, but if enter a number that is too large, the font will not be visible for too long" << std::endl;
                        std::cin >> movement_threshold_vertical;
                    }
#pragma endregion
                    // movement_type_vertical_background
#pragma region movement_type_vertical_background
                    int movement_type_vertical_background = (int)e_movement_type_vertical::none;
                    int movement_speed_vertical_background = 0;
                    int movement_threshold_vertical_background = INT_MIN;

                    std::cout << "font_create, Input movement_type_vertical_background, If Input 0: none, 1: move to top, 2: move to bottom" << std::endl;
                    std::cin >> movement_type_vertical_background;

                    if (movement_type_vertical_background != (int)e_movement_type_vertical::none)
                    {
                        std::cout << "font_create, Input movement_speed_vertical_background, If Input negative value, speed use default value, 0" << std::endl;
                        std::cin >> movement_speed_vertical_background;
                        if (movement_speed_vertical_background < 0)
                        {
                            movement_speed_vertical_background = 0;
                        }
                    }

                    if (movement_speed_vertical_background != 0)
                    {
                        std::cout << "font_create, Input movement_threshold_vertical_background, Any number in the int range, but if enter a number that is too large, the font will not be visible for too long" << std::endl;
                        std::cin >> movement_threshold_vertical_background;
                    }
#pragma endregion
                    // font_start_coordinate
#pragma region font_start_coordinate
                    int font_start_coordinate_left = 0;
                    int font_start_coordinate_top = 0;

                    std::cout << "font_create, Input font_start_coordinate_left" << std::endl;
                    std::cin >> font_start_coordinate_left;

                    std::cout << "font_create, Input font_start_coordinate_top" << std::endl;
                    std::cin >> font_start_coordinate_top;
#pragma endregion
                    // background_rectangle
#pragma region background_rectangle
                    int backgound_rectangle_width = 0;
                    int backgound_rectangle_height = 0;

                    std::cout << "font_create, Input backgound_rectangle_width backgound_rectangle_height, If Input 0 or negative value, will not use background_rectangle" << std::endl;
                    std::cin >> backgound_rectangle_width;
                    std::cin >> backgound_rectangle_height;

                    if (backgound_rectangle_width < 0 || backgound_rectangle_height < 0)
                    {
                        backgound_rectangle_width = 0;
                        backgound_rectangle_height = 0;
                    }
#pragma endregion
                    // font_style
#pragma region font_style
                    int font_weight = (int)e_dwrite_font_weight::DWRITE_FONT_WEIGHT_NORMAL;
                    int font_style = (int)e_dwrite_font_style::DWRITE_FONT_STYLE_NORMAL;
                    int font_stretch = (int)e_dwrite_font_stretch::DWRITE_FONT_STRETCH_NORMAL;
#pragma endregion

                    cppsocket_struct_client_send_font_create data{};

                    data.index_font = index_font;

                    data.content_string = content_string.c_str();
                    data.content_size = content_size;

                    data.font_family = font_family.c_str();
                    data.font_family_size = font_family_size;

                    data.font_size = font_size;

                    data.font_color_r = font_color_r;
                    data.font_color_g = font_color_g;
                    data.font_color_b = font_color_b;
                    data.font_color_a = font_color_a;
                    
                    data.background_color_r = background_color_r;
                    data.background_color_g = background_color_g;
                    data.background_color_b = background_color_b;
                    data.background_color_a = background_color_a;

                    data.movement_type_horizontal = movement_type_horizontal;
                    data.movement_speed_horizontal = movement_speed_horizontal;
                    data.movement_threshold_horizontal = movement_threshold_horizontal;

                    data.movement_type_horizontal_background = movement_type_horizontal_background;
                    data.movement_speed_horizontal_background = movement_speed_horizontal_background;
                    data.movement_threshold_horizontal_background = movement_threshold_horizontal_background;

                    data.movement_type_vertical = movement_type_vertical;
                    data.movement_speed_vertical = movement_speed_vertical;
                    data.movement_threshold_vertical = movement_threshold_vertical;

                    data.movement_type_vertical_background = movement_type_vertical_background;
                    data.movement_speed_vertical_background = movement_speed_vertical_background;
                    data.movement_threshold_vertical_background = movement_threshold_vertical_background;

                    data.font_start_coordinate_left = font_start_coordinate_left;
                    data.font_start_coordinate_top = font_start_coordinate_top;

                    data.backgound_rectangle_width = backgound_rectangle_width;
                    data.backgound_rectangle_height = backgound_rectangle_height;
                    
                    data.font_weight = font_weight;
                    data.font_style = font_style;
                    data.font_stretch = font_stretch;

                    cppsocket_client_send_font_create(_client, data);
                }
                break;
                case e_command_type::font_delete:
                {
                    std::cout << "font_delete, Input index_font" << std::endl;

                    uint32_t index_font;

                    std::cin >> index_font;

                    cppsocket_struct_client_send_font_delete data{};
                    data.index_font = index_font;

                    cppsocket_client_send_font_delete(_client, data);
                }
                break;
                case e_command_type::font_blink_turn_on_off:
                {
                    std::cout << "font_blink_turn_on_off, Input index_font flag_blink_turn_on_off_int" << std::endl;

                    uint32_t index_font;
                    bool flag_blink_turn_on_off;
                    int flag_blink_turn_on_off_int;

                    std::cin >> index_font;
                    std::cin >> flag_blink_turn_on_off_int;

                    if (flag_blink_turn_on_off_int == 0)
                    {
                        flag_blink_turn_on_off = false;
                    }
                    else
                    {
                        flag_blink_turn_on_off = true;
                    }

                    cppsocket_struct_client_send_font_blink_turn_on_off data{};
                    data.index_font = index_font;
                    data.flag_blink_turn_on_off = flag_blink_turn_on_off;

                    cppsocket_client_send_font_blink_turn_on_off(_client, data);
                }
                break;
                case e_command_type::font_blink_interval:
                {
                    std::cout << "font_blink_interval, Input interval_blink_in_miliseconds" << std::endl;

                    int interval_blink_in_miliseconds;

                    std::cin >> interval_blink_in_miliseconds;

                    cppsocket_struct_client_send_font_blink_interval data{};
                    data.interval_blink_in_miliseconds = interval_blink_in_miliseconds;

                    cppsocket_client_send_font_blink_interval(_client, data);
                }
                break;
                case e_command_type::font_blink_duration:
                {
                    std::cout << "font_blink_duration, Input duration_blink_in_miliseconds" << std::endl;

                    int duration_blink_in_miliseconds;

                    std::cin >> duration_blink_in_miliseconds;

                    cppsocket_struct_client_send_font_blink_duration data{};
                    data.duration_blink_in_miliseconds = duration_blink_in_miliseconds;

                    cppsocket_client_send_font_blink_duration(_client, data);
                }
                break;

                case e_command_type::cef_create:
                {
                    std::cout << "cef_create, Input index_cef left top width height url" << std::endl;

                    int index_cef;
                    int left;
                    int top;
                    int width;
                    int height;
                    std::string url;

                    std::cin >> index_cef;
                    std::cin >> left >> top >> width >> height;
                    std::cin >> url;

                    cppsocket_struct_client_send_cef_create data{};
                    data.index_cef = index_cef;
                    data.left = left;
                    data.top = top;
                    data.width = width;
                    data.height = height;
                    data.url_size = url.size();
                    data.url = url.c_str();

                    cppsocket_client_send_cef_create(_client, data);
                }
                break;
                case e_command_type::cef_delete:
                {
                    std::cout << "cef_delete, Input index_cef" << std::endl;

                    int index_cef;

                    std::cin >> index_cef;

                    cppsocket_struct_client_send_cef_delete data{};
                    data.index_cef = index_cef;

                    cppsocket_client_send_cef_delete(_client, data);
                }
                break;

                default:
                    break;
                }

                if (cmd == 99)
                {
                    break;
                }
            }
            cppsocket_client_connection_close(_client);
        }
    }
    cppsocket_network_shutdown();

    system("pause");

    return 0;
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
