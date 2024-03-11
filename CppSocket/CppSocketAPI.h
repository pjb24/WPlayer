#pragma once

#define EXPORT extern "C" __declspec(dllexport)

#include <stdint.h>

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#include <Windows.h>

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

EXPORT void cppsocket_server_send_play(void* server_instance, void* connection, uint32_t scene_index, uint16_t result);
EXPORT void cppsocket_server_send_pause(void* server_instance, void* connection, uint32_t scene_index, uint16_t result);
EXPORT void cppsocket_server_send_stop(void* server_instance, void* connection, uint32_t scene_index, uint16_t result);
EXPORT void cppsocket_server_send_move(void* server_instance, void* connection, uint32_t scene_index, uint16_t result);

EXPORT void cppsocket_server_send_jump_forward(void* server_instance, void* connection, uint32_t scene_index, uint16_t result);
EXPORT void cppsocket_server_send_jump_backwards(void* server_instance, void* connection, uint32_t scene_index, uint16_t result);

EXPORT void cppsocket_server_set_callback_data_connection(void* server_instance, CALLBACK_DATA_CONNECTION cb);

// --------------------------------

EXPORT void* cppsocket_client_create();

EXPORT void cppsocket_client_delete(void* client_instance);

EXPORT bool cppsocket_client_connect(void* client_instance, const char * ip, uint16_t port);

EXPORT bool cppsocket_client_is_connected(void* client_instance);

EXPORT void cppsocket_client_frame(void* client_instance);

EXPORT void cppsocket_client_send_play(void* client_instance, RECT rect, const char * url, uint16_t url_size);
EXPORT void cppsocket_client_send_pause(void* client_instance, uint32_t scene_index);
EXPORT void cppsocket_client_send_stop(void* client_instance, uint32_t scene_index);
EXPORT void cppsocket_client_send_move(void* client_instance, uint32_t scene_index, RECT rect);

EXPORT void cppsocket_client_send_jump_forward(void* client_instance, uint32_t scene_index);
EXPORT void cppsocket_client_send_jump_backwards(void* client_instance, uint32_t scene_index);

EXPORT void cppsocket_client_set_callback_data(void* client_instance, CALLBACK_DATA cb);

// --------------------------------
