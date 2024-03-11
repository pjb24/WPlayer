#ifndef MYSERVER_H
#define MYSERVER_H

#include "Server.h"

#include "CallbackTypes.h"

class MyServer : public Server
{
public:
    bool send_play(TcpConnection* connection, uint32_t scene_index, uint16_t result);
    bool send_pause(TcpConnection* connection, uint32_t scene_index, uint16_t result);
    bool send_stop(TcpConnection* connection, uint32_t scene_index, uint16_t result);
    bool send_move(TcpConnection* connection, uint32_t scene_index, uint16_t result);

    bool send_jump_forward(TcpConnection* connection, uint32_t scene_index, uint16_t result);
    bool send_jump_backwards(TcpConnection* connection, uint32_t scene_index, uint16_t result);

    void set_callback_data_connection(CALLBACK_DATA_CONNECTION cb) { callback_data_connection = cb; }

private:
	void OnConnect(TcpConnection& newConnection) override;
	void OnDisconnect(TcpConnection& lostConnection, std::string reason) override;
	bool ProcessPacket(std::shared_ptr<Packet> packet) override;
	bool ProcessPacket(std::shared_ptr<Packet> packet, TcpConnection* connection) override;

    CALLBACK_DATA_CONNECTION callback_data_connection = nullptr;
};

#endif // !MYSERVER_H
