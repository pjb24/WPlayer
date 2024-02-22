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

void cppsocket_server_send_play(void * server_instance, void* connection, uint32_t scene_index, uint16_t result)
{
    MyServer* server = (MyServer*)server_instance;
    server->send_play((TcpConnection*)connection, scene_index, result);
}

void cppsocket_server_send_pause(void * server_instance, void* connection, uint32_t scene_index, uint16_t result)
{
    MyServer* server = (MyServer*)server_instance;

}

void cppsocket_server_send_stop(void * server_instance, void* connection, uint32_t scene_index, uint16_t result)
{
    MyServer* server = (MyServer*)server_instance;

}

void cppsocket_server_send_move(void * server_instance, void* connection, uint32_t scene_index, uint16_t result)
{
    MyServer* server = (MyServer*)server_instance;

}

void cppsocket_server_set_callback_data_connection(void * server_instance, CALLBACK_DATA_CONNECTION cb)
{
    MyServer* server = (MyServer*)server_instance;
    server->set_callback_data_connection(cb);
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

void cppsocket_client_send_play(void * client_instance, RECT rect, const char * url, uint16_t url_size)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_play(rect, url, url_size);
}

void cppsocket_client_send_pause(void * client_instance, uint32_t scene_index)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_pause(scene_index);
}

void cppsocket_client_send_stop(void * client_instance, uint32_t scene_index)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_stop(scene_index);
}

void cppsocket_client_send_move(void * client_instance, uint32_t scene_index, RECT rect)
{
    MyClient* client = (MyClient*)client_instance;
    client->send_move(scene_index, rect);
}

void cppsocket_client_set_callback_data(void * client_instance, CALLBACK_DATA cb)
{
    MyClient* client = (MyClient*)client_instance;
    client->set_callback_data(cb);
}

// --------------------------------
