#include "CppSocketAPI.h"

#include "Socket.h"
#include "MyServer.h"
#include "MyClient.h"

// --------------------------------

bool cppsocket_network_initialize()
{
    return Network::Initialize();
}

void cppsocket_network_shutdown()
{
    Network::Shutdown();
}

// --------------------------------

void * cppsocket_server_create()
{
    MyServer* server = new MyServer();
    return server;
}

void cppsocket_server_delete(void * server_instance)
{
    MyServer* server = (MyServer*)server_instance;
    delete server;
}

bool cppsocket_server_initialize(void * server_instance, const char * ip, uint16_t port)
{
    MyServer* server = (MyServer*)server_instance;
    return server->Initialize(IPEndpoint(ip, port));
}

void cppsocket_server_frame(void * server_instance)
{
    MyServer* server = (MyServer*)server_instance;
    server->Frame();
}

void cppsocket_server_send_play(void * server_instance, void* connection, cppsocket_struct_server_send_play data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_play((TcpConnection*)connection, data);
}

void cppsocket_server_send_pause(void * server_instance, void* connection, cppsocket_struct_server_send_pause data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_pause((TcpConnection*)connection, data);
}

void cppsocket_server_send_stop(void * server_instance, void* connection, cppsocket_struct_server_send_stop data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_stop((TcpConnection*)connection, data);
}

void cppsocket_server_send_move(void * server_instance, void* connection, cppsocket_struct_server_send_move data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_move((TcpConnection*)connection, data);
}

void cppsocket_server_send_jump_forward(void * server_instance, void * connection, cppsocket_struct_server_send_jump_forward data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_jump_forward((TcpConnection*)connection, data);
}

void cppsocket_server_send_jump_backwards(void * server_instance, void * connection, cppsocket_struct_server_send_jump_backwards data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_jump_backwards((TcpConnection*)connection, data);
}

void cppsocket_server_send_play_sync_group(void* server_instance, void* connection, cppsocket_struct_server_send_play_sync_group data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_play_sync_group((TcpConnection*)connection, data);
}

void cppsocket_server_send_pause_sync_group(void* server_instance, void* connection, cppsocket_struct_server_send_pause_sync_group data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_pause_sync_group((TcpConnection*)connection, data);
}

void cppsocket_server_send_stop_sync_group(void* server_instance, void* connection, cppsocket_struct_server_send_stop_sync_group data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_stop_sync_group((TcpConnection*)connection, data);
}

void cppsocket_server_set_callback_data_connection(void * server_instance, CALLBACK_DATA_CONNECTION cb)
{
    MyServer* server = (MyServer*)server_instance;
    server->set_callback_data_connection(cb);
}

void cppsocket_server_send_gplayer_play_url(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_play_url data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_gplayer_play_url((TcpConnection*)connection, data);
}

void cppsocket_server_send_gplayer_play_url_different_videos(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_play_url_different_videos data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_gplayer_play_url_different_videos((TcpConnection*)connection, data);
}

void cppsocket_server_send_gplayer_play_rect(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_play_rect data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_gplayer_play_rect((TcpConnection*)connection, data);
}

void cppsocket_server_send_gplayer_connect_data_url(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_connect_data_url data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_gplayer_connect_data_url((TcpConnection*)connection, data);
}

void cppsocket_server_send_gplayer_connect_data_url_different_videos(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_connect_data_url_different_videos data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_gplayer_connect_data_url_different_videos((TcpConnection*)connection, data);
}

void cppsocket_server_send_gplayer_connect_data_rect(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_connect_data_rect data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_gplayer_connect_data_rect((TcpConnection*)connection, data);
}

void cppsocket_server_send_gplayer_stop(void* server_instance, void* connection, cppsocket_struct_server_send_gplayer_stop data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_gplayer_stop((TcpConnection*)connection, data);
}

void cppsocket_server_send_dplayer_play_url(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_play_url data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_dplayer_play_url((TcpConnection*)connection, data);
}

void cppsocket_server_send_dplayer_play_rect(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_play_rect data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_dplayer_play_rect((TcpConnection*)connection, data);
}

void cppsocket_server_send_dplayer_connect_data_url(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_connect_data_url data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_dplayer_connect_data_url((TcpConnection*)connection, data);
}

void cppsocket_server_send_dplayer_connect_data_rect(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_connect_data_rect data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_dplayer_connect_data_rect((TcpConnection*)connection, data);
}

void cppsocket_server_send_dplayer_stop(void* server_instance, void* connection, cppsocket_struct_server_send_dplayer_stop data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_dplayer_stop((TcpConnection*)connection, data);
}

void cppsocket_server_send_font_create(void* server_instance, void* connection, cppsocket_struct_server_send_font_create data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_font_create((TcpConnection*)connection, data);
}

void cppsocket_server_send_font_delete(void* server_instance, void* connection, cppsocket_struct_server_send_font_delete data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_font_delete((TcpConnection*)connection, data);
}

void cppsocket_server_send_font_blink_turn_on_off(void* server_instance, void* connection, cppsocket_struct_server_send_font_blink_turn_on_off data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_font_blink_turn_on_off((TcpConnection*)connection, data);
}

void cppsocket_server_send_font_blink_interval(void* server_instance, void* connection, cppsocket_struct_server_send_font_blink_interval data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_font_blink_interval((TcpConnection*)connection, data);
}

void cppsocket_server_send_font_blink_duration(void* server_instance, void* connection, cppsocket_struct_server_send_font_blink_duration data)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_font_blink_duration((TcpConnection*)connection, data);
}

// --------------------------------

void * cppsocket_client_create()
{
    MyClient* client = new MyClient();
    return client;
}

void cppsocket_client_delete(void * client_instance)
{
    MyClient* client = (MyClient*)client_instance;
    delete client;
}

bool cppsocket_client_connect(void * client_instance, const char * ip, uint16_t port)
{
    MyClient* client = (MyClient*)client_instance;
    return client->Connect(IPEndpoint(ip, port));
}

bool cppsocket_client_is_connected(void * client_instance)
{
    MyClient* client = (MyClient*)client_instance;
    return client->IsConnected();
}

void cppsocket_client_frame(void * client_instance)
{
    MyClient* client = (MyClient*)client_instance;
    client->Frame();
}

void cppsocket_client_send_play(void * client_instance, cppsocket_struct_client_send_play data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_play(data);
}

void cppsocket_client_send_pause(void * client_instance, cppsocket_struct_client_send_pause data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_pause(data);
}

void cppsocket_client_send_stop(void * client_instance, cppsocket_struct_client_send_stop data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_stop(data);
}

void cppsocket_client_send_move(void * client_instance, cppsocket_struct_client_send_move data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_move(data);
}

void cppsocket_client_send_jump_forward(void * client_instance, cppsocket_struct_client_send_jump_forward data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_forward(data);
}

void cppsocket_client_send_jump_backwards(void * client_instance, cppsocket_struct_client_send_jump_backwards data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_backwards(data);
}

void cppsocket_client_send_play_sync_group(void* client_instance, cppsocket_struct_client_send_play_sync_group data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_play_sync_group(data);
}

void cppsocket_client_send_pause_sync_group(void* client_instance, cppsocket_struct_client_send_pause_sync_group data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_pause_sync_group(data);
}

void cppsocket_client_send_stop_sync_group(void* client_instance, cppsocket_struct_client_send_stop_sync_group data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_stop_sync_group(data);
}

void cppsocket_client_send_program_quit(void* client_instance)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_program_quit();
}

void cppsocket_client_set_callback_data(void * client_instance, CALLBACK_DATA cb)
{
    MyClient* client = (MyClient*)client_instance;
    client->set_callback_data(cb);
}

void cppsocket_client_connection_close(void * client_instance)
{
    MyClient* client = (MyClient*)client_instance;
    client->connection_close();
}

void cppsocket_client_send_gplayer_play_url(void* client_instance, cppsocket_struct_client_send_gplayer_play_url data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_gplayer_play_url(data);
}

void cppsocket_client_send_gplayer_play_url_different_videos(void* client_instance, cppsocket_struct_client_send_gplayer_play_url_different_videos data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_gplayer_play_url_different_videos(data);
}

void cppsocket_client_send_gplayer_play_rect(void* client_instance, cppsocket_struct_client_send_gplayer_play_rect data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_gplayer_play_rect(data);
}

void cppsocket_client_send_gplayer_connect(void* client_instance, cppsocket_struct_client_send_player_connect data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_gplayer_connect(data);
}

void cppsocket_client_send_gplayer_stop(void* client_instance, cppsocket_struct_client_send_gplayer_stop data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_gplayer_stop(data);
}

void cppsocket_client_send_dplayer_play_url(void* client_instance, cppsocket_struct_client_send_dplayer_play_url data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_dplayer_play_url(data);
}

void cppsocket_client_send_dplayer_play_rect(void* client_instance, cppsocket_struct_client_send_dplayer_play_rect data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_dplayer_play_rect(data);
}

void cppsocket_client_send_dplayer_connect(void* client_instance, cppsocket_struct_client_send_player_connect data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_dplayer_connect(data);
}

void cppsocket_client_send_dplayer_stop(void* client_instance, cppsocket_struct_client_send_dplayer_stop data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_dplayer_stop(data);
}

void cppsocket_client_send_font_create(void* client_instance, cppsocket_struct_client_send_font_create data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_font_create(data);
}

void cppsocket_client_send_font_delete(void* client_instance, cppsocket_struct_client_send_font_delete data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_font_delete(data);
}

void cppsocket_client_send_font_blink_turn_on_off(void* client_instance, cppsocket_struct_client_send_font_blink_turn_on_off data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_font_blink_turn_on_off(data);
}

void cppsocket_client_send_font_blink_interval(void* client_instance, cppsocket_struct_client_send_font_blink_interval data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_font_blink_interval(data);
}

void cppsocket_client_send_font_blink_duration(void* client_instance, cppsocket_struct_client_send_font_blink_duration data)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_font_blink_duration(data);
}

// --------------------------------
