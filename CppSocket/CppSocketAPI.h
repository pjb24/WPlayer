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

/// <summary>
/// �ٸ� CppSocket�� API�� ����ϱ� ���� ȣ���ؾ� ��.
/// </summary>
/// <returns></returns>
EXPORT bool cppsocket_network_initialize();

/// <summary>
/// CppSocket�� ����� ������ ���� ȣ���ؾ� ��.
/// ���� �ٸ� ���̺귯������ winsock�� ����Ѵٸ� ������ �߻���ų �� ����.
/// </summary>
EXPORT void cppsocket_network_shutdown();

// --------------------------------

/// <summary>
/// MyServer Ŭ���� �ν��Ͻ��� ������.
/// ������ �ν��Ͻ��� �ٸ� Server API�� ����� �� �Ķ���ͷ� �����ؾ� ��.
/// </summary>
/// <returns> MyServer Ŭ���� �ν��Ͻ��� void*�� ��ȯ��. </returns>
EXPORT void* cppsocket_server_create();

/// <summary>
/// MyServer Ŭ���� �ν��Ͻ��� delete ��.
/// </summary>
/// <param name="server_instance"> </param>
EXPORT void cppsocket_server_delete(void* server_instance);

/// <summary>
/// Server�� Listening�� ������.
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="ip"> Listen�� ip </param>
/// <param name="port"> Listen�� port </param>
/// <returns> true: ����, false: ���� </returns>
EXPORT bool cppsocket_server_initialize(void* server_instance, const char * ip, uint16_t port);

/// <summary>
/// Server�� �ʿ信 ���� ��� ��Ŷ�� send�ϰ� receive�ϴ� 1���� ������ ������.
/// �� ���̺귯���� ����Ͽ� Server ������ �����ϴ� ���α׷��� �ݺ������� �� �Լ��� ȣ������� ��.
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
EXPORT void cppsocket_server_frame(void* server_instance);

/// <summary>
/// ������� ����.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_play(void* server_instance, void* connection, cppsocket_struct_server_send_play data);
/// <summary>
/// ������� ����.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_pause(void* server_instance, void* connection, cppsocket_struct_server_send_pause data);
/// <summary>
/// ������� ����.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_stop(void* server_instance, void* connection, cppsocket_struct_server_send_stop data);
/// <summary>
/// ������� ����.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_move(void* server_instance, void* connection, cppsocket_struct_server_send_move data);

/// <summary>
/// ������� ����.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_jump_forward(void* server_instance, void* connection, cppsocket_struct_server_send_jump_forward data);
/// <summary>
/// ������� ����.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_jump_backwards(void* server_instance, void* connection, cppsocket_struct_server_send_jump_backwards data);

/// <summary>
/// ������� ����.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_play_sync_group(void* server_instance, void* connection, cppsocket_struct_server_send_play_sync_group data);
/// <summary>
/// ������� ����.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_pause_sync_group(void* server_instance, void* connection, cppsocket_struct_server_send_pause_sync_group data);
/// <summary>
/// ������� ����.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_stop_sync_group(void* server_instance, void* connection, cppsocket_struct_server_send_stop_sync_group data);

/// <summary>
/// ��Ŷ�� �������� �� �� ���̺귯���� ����Ͽ� Server ������ �����ϴ� ���α׷����� �����͸� ������ �� ����� Callback �Լ��� ������.
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="cb"> ���� ���α׷����� �����͸� ������ �� ����� Callback �Լ� </param>
EXPORT void cppsocket_server_set_callback_data_connection(void* server_instance, CALLBACK_DATA_CONNECTION cb);

/// <summary>
/// gplayer_play_url ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_gplayer_play_url(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_play_url data);
/// <summary>
/// gplayer_play_url_different_videos ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_gplayer_play_url_different_videos(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_play_url_different_videos data);
/// <summary>
/// gplayer_play_rect ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_gplayer_play_rect(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_play_rect data);
/// <summary>
/// gplayer_stop ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_gplayer_stop(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_stop data);

/// <summary>
/// gplayer_connect ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_gplayer_connect_data_url(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_connect_data_url data);
/// <summary>
/// gplayer_connect ��ɿ� �����ϴ� Server�� response
/// ���� �Է��� gplayer_play_url_different_videos�� ���Ծ��� �� ����.
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_gplayer_connect_data_url_different_videos(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_connect_data_url_different_videos data);
/// <summary>
/// gplayer_connect ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_gplayer_connect_data_rect(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_connect_data_rect data);

/// <summary>
/// dplayer_play_url ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_dplayer_play_url(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_play_url data);
/// <summary>
/// dplayer_play_rect ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_dplayer_play_rect(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_play_rect data);
/// <summary>
/// dplayer_stop ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_dplayer_stop(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_stop data);

/// <summary>
/// dplayer_connect ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_dplayer_connect_data_url(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_connect_data_url data);
/// <summary>
/// dplayer_connect ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_dplayer_connect_data_rect(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_connect_data_rect data);

/// <summary>
/// font_create ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_font_create(void* server_instance, void* connection, cppsocket_struct_server_send_font_create data);
/// <summary>
/// font_delete ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_font_delete(void* server_instance, void* connection, cppsocket_struct_server_send_font_delete data);

/// <summary>
/// font_blink_turn_on_off ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_font_blink_turn_on_off(void* server_instance, void* connection, cppsocket_struct_server_send_font_blink_turn_on_off data);
/// <summary>
/// font_blink_interval ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_font_blink_interval(void* server_instance, void* connection, cppsocket_struct_server_send_font_blink_interval data);
/// <summary>
/// font_blink_duration ��ɿ� �����ϴ� Server�� response
/// </summary>
/// <param name="server_instance"> MyServer Ŭ���� �ν��Ͻ� </param>
/// <param name="connection"> ����� ���� Client�� connection </param>
/// <param name="data"> Client�� ���� ������ </param>
EXPORT void cppsocket_server_send_font_blink_duration(void* server_instance, void* connection, cppsocket_struct_server_send_font_blink_duration data);

// --------------------------------

/// <summary>
/// MyClient Ŭ���� �ν��Ͻ��� ������.
/// ������ �ν��Ͻ��� �ٸ� Client API�� ����� �� �Ķ���ͷ� �����ؾ� ��.
/// </summary>
/// <returns> MyClient Ŭ���� �ν��Ͻ��� void*�� ��ȯ��. </returns>
EXPORT void* cppsocket_client_create();

/// <summary>
/// MyClient �ν��Ͻ��� delete ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
EXPORT void cppsocket_client_delete(void* client_instance);

/// <summary>
/// Server�� Connect �ϴ� �Լ�
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="ip"> Connect�� IP </param>
/// <param name="port"> Connect�� Port </param>
/// <returns> true: ����, false: ���� </returns>
EXPORT bool cppsocket_client_connect(void* client_instance, const char * ip, uint16_t port);

/// <summary>
/// Server�� Connect �Ǿ��ִ��� Ȯ���ϴ� �Լ�
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <returns> true: Connect ������, false: Connect ���°� �ƴ� </returns>
EXPORT bool cppsocket_client_is_connected(void* client_instance);

/// <summary>
/// Client�� �ʿ信 ���� ��� ��Ŷ�� send�ϰ� receive�ϴ� 1���� ������ ������.
/// �� ���̺귯���� ����Ͽ� Client ������ �����ϴ� ���α׷��� �ݺ������� �� �Լ��� ȣ������� ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
EXPORT void cppsocket_client_frame(void* client_instance);

/// <summary>
/// ������� ����.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_play(void* client_instance, cppsocket_struct_client_send_play data);
/// <summary>
/// ������� ����.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_pause(void* client_instance, cppsocket_struct_client_send_pause data);
/// <summary>
/// ������� ����.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_stop(void* client_instance, cppsocket_struct_client_send_stop data);
/// <summary>
/// ������� ����.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_move(void* client_instance, cppsocket_struct_client_send_move data);

/// <summary>
/// ������� ����.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_jump_forward(void* client_instance, cppsocket_struct_client_send_jump_forward data);
/// <summary>
/// ������� ����.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_jump_backwards(void* client_instance, cppsocket_struct_client_send_jump_backwards data);

/// <summary>
/// ������� ����.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_play_sync_group(void* client_instance, cppsocket_struct_client_send_play_sync_group data);
/// <summary>
/// ������� ����.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_pause_sync_group(void* client_instance, cppsocket_struct_client_send_pause_sync_group data);
/// <summary>
/// ������� ����.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_stop_sync_group(void* client_instance, cppsocket_struct_client_send_stop_sync_group data);

/// <summary>
/// Server�� ���Ḧ ��û�ϴ� �Լ�
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
EXPORT void cppsocket_client_send_program_quit(void* client_instance);

/// <summary>
/// ��Ŷ�� �������� �� �� ���̺귯���� ����Ͽ� Client ������ �����ϴ� ���α׷����� �����͸� ������ �� ����� Callback �Լ��� ������.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="cb"> ���� ���α׷����� �����͸� ������ �� ����� Callback �Լ� </param>
EXPORT void cppsocket_client_set_callback_data(void* client_instance, CALLBACK_DATA cb);

/// <summary>
/// Server���� ������ ������.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
EXPORT void cppsocket_client_connection_close(void* client_instance);

/// <summary>
/// gplayer_connect ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_gplayer_connect(void* client_instance, cppsocket_struct_client_send_player_connect data);
/// <summary>
/// gplayer_play_url ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_gplayer_play_url(void* client_instance, cppsocket_struct_client_send_gplayer_play_url data);
/// <summary>
/// gplayer_play_url_different_videos ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_gplayer_play_url_different_videos(void* client_instance, cppsocket_struct_client_send_gplayer_play_url_different_videos data);
/// <summary>
/// gplayer_play_rect ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_gplayer_play_rect(void* client_instance, cppsocket_struct_client_send_gplayer_play_rect data);
/// <summary>
/// gplayer_stop ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_gplayer_stop(void* client_instance, cppsocket_struct_client_send_gplayer_stop data);

/// <summary>
/// dplayer_connect ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_dplayer_connect(void* client_instance, cppsocket_struct_client_send_player_connect data);
/// <summary>
/// dplayer_play_url ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_dplayer_play_url(void* client_instance, cppsocket_struct_client_send_dplayer_play_url data);
/// <summary>
/// dplayer_play_rect ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_dplayer_play_rect(void* client_instance, cppsocket_struct_client_send_dplayer_play_rect data);
/// <summary>
/// dplayer_stop ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_dplayer_stop(void* client_instance, cppsocket_struct_client_send_dplayer_stop data);

/// <summary>
/// font_create ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_font_create(void* client_instance, cppsocket_struct_client_send_font_create data);
/// <summary>
/// font_delete ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_font_delete(void* client_instance, cppsocket_struct_client_send_font_delete data);

/// <summary>
/// font_blink_turn_on_off ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_font_blink_turn_on_off(void* client_instance, cppsocket_struct_client_send_font_blink_turn_on_off data);
/// <summary>
/// font_blink_interval ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_font_blink_interval(void* client_instance, cppsocket_struct_client_send_font_blink_interval data);
/// <summary>
/// font_blink_duration ����� Server�� request ��.
/// </summary>
/// <param name="client_instance"> MyClient Ŭ���� �ν��Ͻ� </param>
/// <param name="data"> Server�� ���� ������ </param>
EXPORT void cppsocket_client_send_font_blink_duration(void* client_instance, cppsocket_struct_client_send_font_blink_duration data);

// --------------------------------
