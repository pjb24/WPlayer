#ifndef MYCLIENT_H
#define MYCLIENT_H

#include "Client.h"

#include "CallbackTypes.h"

class MyClient : public Client
{
public:
    bool send_play(RECT rect, const char* url, uint16_t url_size);
    bool send_pause(uint32_t scene_index);
    bool send_stop(uint32_t scene_index);
    bool send_move(uint32_t scene_index, RECT rect);

    void set_callback_data(CALLBACK_DATA cb) { callback_data = cb; }

    void callback_client(void* packet);

private:
	bool ProcessPacket(std::shared_ptr<Packet> packet) override;
	void OnConnect() override;
	//void OnConnectFail() override;
	//void OnDisconnect(std::string reason) override;

    CALLBACK_DATA callback_data = nullptr;
};

#endif // !MYCLIENT_H
