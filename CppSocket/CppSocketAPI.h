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

/// <summary>
/// 다른 CppSocket의 API를 사용하기 전에 호출해야 함.
/// </summary>
/// <returns></returns>
EXPORT bool cppsocket_network_initialize();

/// <summary>
/// CppSocket의 사용을 끝내기 전에 호출해야 함.
/// 만약 다른 라이브러리에서 winsock를 사용한다면 오류를 발생시킬 수 있음.
/// </summary>
EXPORT void cppsocket_network_shutdown();

// --------------------------------

/// <summary>
/// MyServer 클래스 인스턴스를 생성함.
/// 생성된 인스턴스는 다른 Server API를 사용할 때 파라미터로 제공해야 함.
/// </summary>
/// <returns> MyServer 클래스 인스턴스를 void*로 반환함. </returns>
EXPORT void* cppsocket_server_create();

/// <summary>
/// MyServer 클래스 인스턴스를 delete 함.
/// </summary>
/// <param name="server_instance"> </param>
EXPORT void cppsocket_server_delete(void* server_instance);

/// <summary>
/// Server의 Listening을 시작함.
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="ip"> Listen할 ip </param>
/// <param name="port"> Listen할 port </param>
/// <returns> true: 성공, false: 실패 </returns>
EXPORT bool cppsocket_server_initialize(void* server_instance, const char * ip, uint16_t port);

/// <summary>
/// Server가 필요에 따라 통신 패킷을 send하고 receive하는 1번의 루프를 수행함.
/// 이 라이브러리를 사용하여 Server 역할을 수행하는 프로그램은 반복적으로 이 함수를 호출해줘야 함.
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
EXPORT void cppsocket_server_frame(void* server_instance);

/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_play(void* server_instance, void* connection, cppsocket_struct_server_send_play data);
/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_pause(void* server_instance, void* connection, cppsocket_struct_server_send_pause data);
/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_stop(void* server_instance, void* connection, cppsocket_struct_server_send_stop data);
/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_move(void* server_instance, void* connection, cppsocket_struct_server_send_move data);

/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_jump_forward(void* server_instance, void* connection, cppsocket_struct_server_send_jump_forward data);
/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_jump_backwards(void* server_instance, void* connection, cppsocket_struct_server_send_jump_backwards data);

/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_play_sync_group(void* server_instance, void* connection, cppsocket_struct_server_send_play_sync_group data);
/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_pause_sync_group(void* server_instance, void* connection, cppsocket_struct_server_send_pause_sync_group data);
/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="server_instance"></param>
/// <param name="connection"></param>
/// <param name="data"></param>
EXPORT void cppsocket_server_send_stop_sync_group(void* server_instance, void* connection, cppsocket_struct_server_send_stop_sync_group data);

/// <summary>
/// 패킷을 수신했을 때 이 라이브러리를 사용하여 Server 역할을 수행하는 프로그램으로 데이터를 전달할 때 사용할 Callback 함수를 설정함.
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="cb"> 상위 프로그램으로 데이터를 전달할 때 사용할 Callback 함수 </param>
EXPORT void cppsocket_server_set_callback_data_connection(void* server_instance, CALLBACK_DATA_CONNECTION cb);

/// <summary>
/// gplayer_play_url 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_gplayer_play_url(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_play_url data);
/// <summary>
/// gplayer_play_url_different_videos 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_gplayer_play_url_different_videos(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_play_url_different_videos data);
/// <summary>
/// gplayer_play_rect 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_gplayer_play_rect(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_play_rect data);
/// <summary>
/// gplayer_stop 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_gplayer_stop(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_stop data);

/// <summary>
/// gplayer_connect 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_gplayer_connect_data_url(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_connect_data_url data);
/// <summary>
/// gplayer_connect 명령에 대응하는 Server의 response
/// 영상 입력이 gplayer_play_url_different_videos로 들어왔었을 때 사용됨.
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_gplayer_connect_data_url_different_videos(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_connect_data_url_different_videos data);
/// <summary>
/// gplayer_connect 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_gplayer_connect_data_rect(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_connect_data_rect data);

/// <summary>
/// dplayer_play_url 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_dplayer_play_url(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_play_url data);
/// <summary>
/// dplayer_play_rect 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_dplayer_play_rect(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_play_rect data);
/// <summary>
/// dplayer_stop 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_dplayer_stop(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_stop data);

/// <summary>
/// dplayer_connect 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_dplayer_connect_data_url(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_connect_data_url data);
/// <summary>
/// dplayer_connect 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_dplayer_connect_data_rect(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_connect_data_rect data);

/// <summary>
/// font_create 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_font_create(void* server_instance, void* connection, cppsocket_struct_server_send_font_create data);
/// <summary>
/// font_delete 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_font_delete(void* server_instance, void* connection, cppsocket_struct_server_send_font_delete data);

/// <summary>
/// font_blink_turn_on_off 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_font_blink_turn_on_off(void* server_instance, void* connection, cppsocket_struct_server_send_font_blink_turn_on_off data);
/// <summary>
/// font_blink_interval 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_font_blink_interval(void* server_instance, void* connection, cppsocket_struct_server_send_font_blink_interval data);
/// <summary>
/// font_blink_duration 명령에 대응하는 Server의 response
/// </summary>
/// <param name="server_instance"> MyServer 클래스 인스턴스 </param>
/// <param name="connection"> 명령을 보낸 Client의 connection </param>
/// <param name="data"> Client로 보낼 데이터 </param>
EXPORT void cppsocket_server_send_font_blink_duration(void* server_instance, void* connection, cppsocket_struct_server_send_font_blink_duration data);

// --------------------------------

/// <summary>
/// MyClient 클래스 인스턴스를 생성함.
/// 생성된 인스턴스는 다른 Client API를 사용할 때 파라미터로 제공해야 함.
/// </summary>
/// <returns> MyClient 클래스 인스턴스를 void*로 반환함. </returns>
EXPORT void* cppsocket_client_create();

/// <summary>
/// MyClient 인스턴스를 delete 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
EXPORT void cppsocket_client_delete(void* client_instance);

/// <summary>
/// Server로 Connect 하는 함수
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="ip"> Connect할 IP </param>
/// <param name="port"> Connect할 Port </param>
/// <returns> true: 성공, false: 실패 </returns>
EXPORT bool cppsocket_client_connect(void* client_instance, const char * ip, uint16_t port);

/// <summary>
/// Server와 Connect 되어있는지 확인하는 함수
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <returns> true: Connect 상태임, false: Connect 상태가 아님 </returns>
EXPORT bool cppsocket_client_is_connected(void* client_instance);

/// <summary>
/// Client가 필요에 따라 통신 패킷을 send하고 receive하는 1번의 루프를 수행함.
/// 이 라이브러리를 사용하여 Client 역할을 수행하는 프로그램은 반복적으로 이 함수를 호출해줘야 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
EXPORT void cppsocket_client_frame(void* client_instance);

/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_play(void* client_instance, cppsocket_struct_client_send_play data);
/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_pause(void* client_instance, cppsocket_struct_client_send_pause data);
/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_stop(void* client_instance, cppsocket_struct_client_send_stop data);
/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_move(void* client_instance, cppsocket_struct_client_send_move data);

/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_jump_forward(void* client_instance, cppsocket_struct_client_send_jump_forward data);
/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_jump_backwards(void* client_instance, cppsocket_struct_client_send_jump_backwards data);

/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_play_sync_group(void* client_instance, cppsocket_struct_client_send_play_sync_group data);
/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_pause_sync_group(void* client_instance, cppsocket_struct_client_send_pause_sync_group data);
/// <summary>
/// 사용하지 않음.
/// </summary>
/// <param name="client_instance"></param>
/// <param name="data"></param>
EXPORT void cppsocket_client_send_stop_sync_group(void* client_instance, cppsocket_struct_client_send_stop_sync_group data);

/// <summary>
/// Server의 종료를 요청하는 함수
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
EXPORT void cppsocket_client_send_program_quit(void* client_instance);

/// <summary>
/// 패킷을 수신했을 때 이 라이브러리를 사용하여 Client 역할을 수행하는 프로그램으로 데이터를 전달할 때 사용할 Callback 함수를 설정함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="cb"> 상위 프로그램으로 데이터를 전달할 때 사용할 Callback 함수 </param>
EXPORT void cppsocket_client_set_callback_data(void* client_instance, CALLBACK_DATA cb);

/// <summary>
/// Server와의 연결을 종료함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
EXPORT void cppsocket_client_connection_close(void* client_instance);

/// <summary>
/// gplayer_connect 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_gplayer_connect(void* client_instance, cppsocket_struct_client_send_player_connect data);
/// <summary>
/// gplayer_play_url 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_gplayer_play_url(void* client_instance, cppsocket_struct_client_send_gplayer_play_url data);
/// <summary>
/// gplayer_play_url_different_videos 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_gplayer_play_url_different_videos(void* client_instance, cppsocket_struct_client_send_gplayer_play_url_different_videos data);
/// <summary>
/// gplayer_play_rect 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_gplayer_play_rect(void* client_instance, cppsocket_struct_client_send_gplayer_play_rect data);
/// <summary>
/// gplayer_stop 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_gplayer_stop(void* client_instance, cppsocket_struct_client_send_gplayer_stop data);

/// <summary>
/// dplayer_connect 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_dplayer_connect(void* client_instance, cppsocket_struct_client_send_player_connect data);
/// <summary>
/// dplayer_play_url 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_dplayer_play_url(void* client_instance, cppsocket_struct_client_send_dplayer_play_url data);
/// <summary>
/// dplayer_play_rect 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_dplayer_play_rect(void* client_instance, cppsocket_struct_client_send_dplayer_play_rect data);
/// <summary>
/// dplayer_stop 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_dplayer_stop(void* client_instance, cppsocket_struct_client_send_dplayer_stop data);

/// <summary>
/// font_create 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_font_create(void* client_instance, cppsocket_struct_client_send_font_create data);
/// <summary>
/// font_delete 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_font_delete(void* client_instance, cppsocket_struct_client_send_font_delete data);

/// <summary>
/// font_blink_turn_on_off 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_font_blink_turn_on_off(void* client_instance, cppsocket_struct_client_send_font_blink_turn_on_off data);
/// <summary>
/// font_blink_interval 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_font_blink_interval(void* client_instance, cppsocket_struct_client_send_font_blink_interval data);
/// <summary>
/// font_blink_duration 명령을 Server로 request 함.
/// </summary>
/// <param name="client_instance"> MyClient 클래스 인스턴스 </param>
/// <param name="data"> Server로 보낼 데이터 </param>
EXPORT void cppsocket_client_send_font_blink_duration(void* client_instance, cppsocket_struct_client_send_font_blink_duration data);

// --------------------------------
