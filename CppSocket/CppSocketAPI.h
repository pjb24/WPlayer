#pragma once

#define EXPORT extern "C" __declspec(dllexport)

#include <stdint.h>

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
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
// --------------------------------
