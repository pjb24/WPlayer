#pragma once

#define EXPORT extern "C" __declspec(dllexport)

#include <stdint.h>

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#include <Windows.h>

#include "ApiFunctionStructures.h"

// CallbackType
typedef void(*CALLBACK_DATA)(void*);
typedef void(*CALLBACK_DATA_CONNECTION)(void*, void*);

// --------------------------------

EXPORT bool cppsocket_network_initialize();

EXPORT void cppsocket_network_shutdown();

// --------------------------------

EXPORT void* cppsocket_server_create();

EXPORT void cppsocket_server_delete(void* server_instance);

EXPORT bool cppsocket_server_initialize(void* server_instance, const char * ip, uint16_t port);

EXPORT void cppsocket_server_frame(void* server_instance);

EXPORT void cppsocket_server_send_play(void* server_instance, void* connection, cppsocket_struct_server_send_play data);
EXPORT void cppsocket_server_send_pause(void* server_instance, void* connection, cppsocket_struct_server_send_pause data);
EXPORT void cppsocket_server_send_stop(void* server_instance, void* connection, cppsocket_struct_server_send_stop data);
EXPORT void cppsocket_server_send_move(void* server_instance, void* connection, cppsocket_struct_server_send_move data);

EXPORT void cppsocket_server_send_jump_forward(void* server_instance, void* connection, cppsocket_struct_server_send_jump_forward data);
EXPORT void cppsocket_server_send_jump_backwards(void* server_instance, void* connection, cppsocket_struct_server_send_jump_backwards data);

EXPORT void cppsocket_server_send_play_sync_group(void* server_instance, void* connection, cppsocket_struct_server_send_play_sync_group data);
EXPORT void cppsocket_server_send_pause_sync_group(void* server_instance, void* connection, cppsocket_struct_server_send_pause_sync_group data);
EXPORT void cppsocket_server_send_stop_sync_group(void* server_instance, void* connection, cppsocket_struct_server_send_stop_sync_group data);

EXPORT void cppsocket_server_set_callback_data_connection(void* server_instance, CALLBACK_DATA_CONNECTION cb);

EXPORT void cppsocket_server_send_gplayer_play_url(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_play_url data);
EXPORT void cppsocket_server_send_gplayer_play_url_different_videos(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_play_url_different_videos data);
EXPORT void cppsocket_server_send_gplayer_play_rect(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_play_rect data);
EXPORT void cppsocket_server_send_gplayer_stop(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_stop data);

EXPORT void cppsocket_server_send_gplayer_connect_data_url(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_connect_data_url data);
EXPORT void cppsocket_server_send_gplayer_connect_data_url_different_videos(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_connect_data_url_different_videos data);
EXPORT void cppsocket_server_send_gplayer_connect_data_rect(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_connect_data_rect data);

EXPORT void cppsocket_server_send_dplayer_play_url(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_play_url data);
EXPORT void cppsocket_server_send_dplayer_play_rect(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_play_rect data);
EXPORT void cppsocket_server_send_dplayer_stop(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_stop data);

EXPORT void cppsocket_server_send_dplayer_connect_data_url(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_connect_data_url data);
EXPORT void cppsocket_server_send_dplayer_connect_data_rect(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_connect_data_rect data);

EXPORT void cppsocket_server_send_font_create(void* server_instance, void* connection, cppsocket_struct_server_send_font_create data);
EXPORT void cppsocket_server_send_font_delete(void* server_instance, void* connection, cppsocket_struct_server_send_font_delete data);

// --------------------------------

EXPORT void* cppsocket_client_create();

EXPORT void cppsocket_client_delete(void* client_instance);

EXPORT bool cppsocket_client_connect(void* client_instance, const char * ip, uint16_t port);

EXPORT bool cppsocket_client_is_connected(void* client_instance);

EXPORT void cppsocket_client_frame(void* client_instance);

EXPORT void cppsocket_client_send_play(void* client_instance, cppsocket_struct_client_send_play data);
EXPORT void cppsocket_client_send_pause(void* client_instance, cppsocket_struct_client_send_pause data);
EXPORT void cppsocket_client_send_stop(void* client_instance, cppsocket_struct_client_send_stop data);
EXPORT void cppsocket_client_send_move(void* client_instance, cppsocket_struct_client_send_move data);

EXPORT void cppsocket_client_send_jump_forward(void* client_instance, cppsocket_struct_client_send_jump_forward data);
EXPORT void cppsocket_client_send_jump_backwards(void* client_instance, cppsocket_struct_client_send_jump_backwards data);

EXPORT void cppsocket_client_send_play_sync_group(void* client_instance, cppsocket_struct_client_send_play_sync_group data);
EXPORT void cppsocket_client_send_pause_sync_group(void* client_instance, cppsocket_struct_client_send_pause_sync_group data);
EXPORT void cppsocket_client_send_stop_sync_group(void* client_instance, cppsocket_struct_client_send_stop_sync_group data);

EXPORT void cppsocket_client_send_program_quit(void* client_instance);

EXPORT void cppsocket_client_set_callback_data(void* client_instance, CALLBACK_DATA cb);

EXPORT void cppsocket_client_connection_close(void* client_instance);

EXPORT void cppsocket_client_send_gplayer_connect(void* client_instance, cppsocket_struct_client_send_player_connect data);
EXPORT void cppsocket_client_send_gplayer_play_url(void* client_instance, cppsocket_struct_client_send_gplayer_play_url data);
EXPORT void cppsocket_client_send_gplayer_play_url_different_videos(void* client_instance, cppsocket_struct_client_send_gplayer_play_url_different_videos data);
EXPORT void cppsocket_client_send_gplayer_play_rect(void* client_instance, cppsocket_struct_client_send_gplayer_play_rect data);
EXPORT void cppsocket_client_send_gplayer_stop(void* client_instance, cppsocket_struct_client_send_gplayer_stop data);

EXPORT void cppsocket_client_send_dplayer_connect(void* client_instance, cppsocket_struct_client_send_player_connect data);
EXPORT void cppsocket_client_send_dplayer_play_url(void* client_instance, cppsocket_struct_client_send_dplayer_play_url data);
EXPORT void cppsocket_client_send_dplayer_play_rect(void* client_instance, cppsocket_struct_client_send_dplayer_play_rect data);
EXPORT void cppsocket_client_send_dplayer_stop(void* client_instance, cppsocket_struct_client_send_dplayer_stop data);

EXPORT void cppsocket_client_send_font_create(void* client_instance, cppsocket_struct_client_send_font_create data);
EXPORT void cppsocket_client_send_font_delete(void* client_instance, cppsocket_struct_client_send_font_delete data);

// --------------------------------
