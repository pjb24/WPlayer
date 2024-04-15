#pragma once

#include <stdint.h>

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#include <Windows.h>

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

// --------------------------------
