#ifndef MYCLIENT_H
#define MYCLIENT_H

#include "Client.h"

#include "CallbackTypes.h"

#include "ApiFunctionStructures.h"

class MyClient : public Client
{
public:
    bool send_play(cppsocket_struct_client_send_play data);
    bool send_pause(cppsocket_struct_client_send_pause data);
    bool send_stop(cppsocket_struct_client_send_stop data);
    bool send_move(cppsocket_struct_client_send_move data);

    bool send_forward(cppsocket_struct_client_send_jump_forward data);
    bool send_backwards(cppsocket_struct_client_send_jump_backwards data);

    bool send_play_sync_group(cppsocket_struct_client_send_play_sync_group data);
    bool send_pause_sync_group(cppsocket_struct_client_send_pause_sync_group data);
    bool send_stop_sync_group(cppsocket_struct_client_send_stop_sync_group data);

    bool send_program_quit();

    void set_callback_data(CALLBACK_DATA cb) { callback_data = cb; }

    void callback_client(void* packet);

    void connection_close();

    bool send_gplayer_play_url(cppsocket_struct_client_send_gplayer_play_url data);
    bool send_gplayer_play_rect(cppsocket_struct_client_send_gplayer_play_rect data);
    bool send_gplayer_connect(cppsocket_struct_client_send_player_connect data);
    bool send_gplayer_stop(cppsocket_struct_client_send_gplayer_stop data);
    
    bool send_dplayer_play_url(cppsocket_struct_client_send_dplayer_play_url data);
    bool send_dplayer_play_rect(cppsocket_struct_client_send_dplayer_play_rect data);
    bool send_dplayer_connect(cppsocket_struct_client_send_player_connect data);
    bool send_dplayer_stop(cppsocket_struct_client_send_dplayer_stop data);

private:
	bool ProcessPacket(std::shared_ptr<Packet> packet) override;
	void OnConnect() override;
	//void OnConnectFail() override;
	//void OnDisconnect(std::string reason) override;

    CALLBACK_DATA callback_data = nullptr;
};

#endif // !MYCLIENT_H
