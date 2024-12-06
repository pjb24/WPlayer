#ifndef MYSERVER_H
#define MYSERVER_H

#include "Server.h"

#include "CallbackTypes.h"

#include "ApiFunctionStructures.h"

class MyServer : public Server
{
public:
    bool send_play(TcpConnection* connection, cppsocket_struct_server_send_play data);
    bool send_pause(TcpConnection* connection, cppsocket_struct_server_send_pause data);
    bool send_stop(TcpConnection* connection, cppsocket_struct_server_send_stop data);
    bool send_move(TcpConnection* connection, cppsocket_struct_server_send_move data);

    bool send_jump_forward(TcpConnection* connection, cppsocket_struct_server_send_jump_forward data);
    bool send_jump_backwards(TcpConnection* connection, cppsocket_struct_server_send_jump_backwards data);

    bool send_play_sync_group(TcpConnection* connection, cppsocket_struct_server_send_play_sync_group data);
    bool send_pause_sync_group(TcpConnection* connection, cppsocket_struct_server_send_pause_sync_group data);
    bool send_stop_sync_group(TcpConnection* connection, cppsocket_struct_server_send_stop_sync_group data);

    void set_callback_data_connection(CALLBACK_DATA_CONNECTION cb) { callback_data_connection = cb; }


    bool send_gplayer_play_url(TcpConnection* connection, cppsocket_struct_server_send_gplayer_play_url data);
    bool send_gplayer_play_url_different_videos(TcpConnection* connection, cppsocket_struct_server_send_gplayer_play_url_different_videos data);
    bool send_gplayer_play_rect(TcpConnection* connection, cppsocket_struct_server_send_gplayer_play_rect data);

    bool send_gplayer_connect_data_url(TcpConnection* connection, cppsocket_struct_server_send_gplayer_connect_data_url data);
    bool send_gplayer_connect_data_url_different_videos(TcpConnection* connection, cppsocket_struct_server_send_gplayer_connect_data_url_different_videos data);
    bool send_gplayer_connect_data_rect(TcpConnection* connection, cppsocket_struct_server_send_gplayer_connect_data_rect data);

    bool send_gplayer_stop(TcpConnection* connection, cppsocket_struct_server_send_gplayer_stop data);
    

    bool send_dplayer_play_url(TcpConnection* connection, cppsocket_struct_server_send_dplayer_play_url data);
    bool send_dplayer_play_rect(TcpConnection* connection, cppsocket_struct_server_send_dplayer_play_rect data);

    bool send_dplayer_connect_data_url(TcpConnection* connection, cppsocket_struct_server_send_dplayer_connect_data_url data);
    bool send_dplayer_connect_data_rect(TcpConnection* connection, cppsocket_struct_server_send_dplayer_connect_data_rect data);

    bool send_dplayer_stop(TcpConnection* connection, cppsocket_struct_server_send_dplayer_stop data);
    
    bool send_font_create(TcpConnection* connection, cppsocket_struct_server_send_font_create data);
    bool send_font_delete(TcpConnection* connection, cppsocket_struct_server_send_font_delete data);

    bool send_font_blink_turn_on_off(TcpConnection* connection, cppsocket_struct_server_send_font_blink_turn_on_off data);
    bool send_font_blink_interval(TcpConnection* connection, cppsocket_struct_server_send_font_blink_interval data);
    bool send_font_blink_duration(TcpConnection* connection, cppsocket_struct_server_send_font_blink_duration data);

private:
	void OnConnect(TcpConnection* newConnection) override;
	void OnDisconnect(TcpConnection* lostConnection, std::string reason) override;
	bool ProcessPacket(std::shared_ptr<Packet> packet) override;
	bool ProcessPacket(std::shared_ptr<Packet> packet, TcpConnection* connection) override;

    CALLBACK_DATA_CONNECTION callback_data_connection = nullptr;
};

#endif // !MYSERVER_H
