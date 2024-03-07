// TestDllConsole.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>

#include "CppSocketAPI.h"
#include "PacketDefine.h"

static void* _server;

void callback_data_connection_server(void* data, void* connection)
{
    packet_header* header = (packet_header*)data;

    switch (header->cmd)
    {
    case command_type::play:
    {
        packet_play_from_client* packet = new packet_play_from_client();
        memcpy(packet, data, header->size);

        std::cout << "play" << ", ";
        std::cout << (uint16_t)packet->header.cmd << ", ";
        std::cout << packet->header.size << ", ";
        std::cout << packet->rect.left << ", ";
        std::cout << packet->rect.top << ", ";
        std::cout << packet->rect.right << ", ";
        std::cout << packet->rect.bottom << ", ";
        std::cout << packet->url_size << ", ";
        std::cout << packet->url << std::endl;

        cppsocket_server_send_play(_server, connection, 0, (uint16_t)packet_result::ok);

        delete packet;
    }
    break;
    case command_type::pause:
    {
        packet_pause_from_client* packet = new packet_pause_from_client();
        memcpy(packet, data, header->size);

        std::cout << "pause" << ", ";
        std::cout << (uint16_t)packet->header.cmd << ", ";
        std::cout << packet->header.size << ", ";
        std::cout << packet->scene_index << std::endl;

        cppsocket_server_send_pause(_server, connection, 0, (uint16_t)packet_result::ok);

        delete packet;
    }
    break;
    default:
        break;
    }
}

void callback_ptr_client(void* data)
{
    packet_header* header = (packet_header*)data;

    switch (header->cmd)
    {
    case command_type::play:
    {
        packet_play_from_server* packet = new packet_play_from_server();
        memcpy(packet, data, header->size);

        std::cout << "play" << ", ";
        std::cout << (uint16_t)packet->header.cmd << ", ";
        std::cout << packet->header.size << ", ";
        std::cout << packet->scene_index << ", ";
        std::cout << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::pause:
    {
        packet_pause_from_server* packet = new packet_pause_from_server();
        memcpy(packet, data, header->size);

        std::cout << "pause" << ", ";
        std::cout << (uint16_t)packet->header.cmd << ", ";
        std::cout << packet->header.size << ", ";
        std::cout << packet->scene_index << ", ";
        std::cout << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::stop:
    {
        packet_stop_from_server* packet = new packet_stop_from_server();
        memcpy(packet, data, header->size);

        std::cout << "stop" << ", ";
        std::cout << (uint16_t)packet->header.cmd << ", ";
        std::cout << packet->header.size << ", ";
        std::cout << packet->scene_index << ", ";
        std::cout << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    default:
        break;
    }
}

int main()
{
    int input = 0;
    printf("Input \n0: server start \n1: client play \n2: client pause \n3: client stop \n4: client play2\n 5: client play3\n");
    scanf_s("%d", &input);

    if (cppsocket_network_initialize())
    {
        std::cout << "Winsock api successfully initialized." << std::endl;

        if (input == 0)
        {
            // Server
            void* server = cppsocket_server_create();
            
            cppsocket_server_set_callback_data_connection(server, callback_data_connection_server);

            if (cppsocket_server_initialize(server, "127.0.0.1", 53333))
            {
                _server = server;
                while (true)
                {
                    cppsocket_server_frame(server);
                }
            }
            cppsocket_server_delete(server);
        }
        else if (input == 1)
        {
            RECT rect{ 0, 0, 7680, 2160 };
            std::string url = "C:\\Wizbrain\\media\\media1.mp4";

            // Client
            void* client = cppsocket_client_create();

            cppsocket_client_set_callback_data(client, callback_ptr_client);

            if (cppsocket_client_connect(client, "127.0.0.1", 53333))
            {
                cppsocket_client_send_play(client, rect, url.c_str(), url.size());

                while (cppsocket_client_is_connected(client))
                {
                    cppsocket_client_frame(client);
                }
            }
            cppsocket_client_delete(client);
        }
        else if (input == 2)
        {
            // Client
            void* client = cppsocket_client_create();

            cppsocket_client_set_callback_data(client, callback_ptr_client);

            if (cppsocket_client_connect(client, "127.0.0.1", 53333))
            {
                cppsocket_client_send_pause(client, 0);

                while (cppsocket_client_is_connected(client))
                {
                    cppsocket_client_frame(client);
                }
            }
            cppsocket_client_delete(client);
        }
        else if (input == 3)
        {
            // Client
            void* client = cppsocket_client_create();

            cppsocket_client_set_callback_data(client, callback_ptr_client);

            if (cppsocket_client_connect(client, "127.0.0.1", 53333))
            {
                cppsocket_client_send_stop(client, 0);

                while (cppsocket_client_is_connected(client))
                {
                    cppsocket_client_frame(client);
                }
            }
            cppsocket_client_delete(client);
        }
        else if (input == 4)
        {
            RECT rect{ 0, 0, 7680, 2160 };
            std::string url = "C:\\Wizbrain\\media\\media_4k_1.mp4";
            void* client = cppsocket_client_create();
            cppsocket_client_set_callback_data(client, callback_ptr_client);
            if (cppsocket_client_connect(client, "127.0.0.1", 53333))
            {
                cppsocket_client_send_play(client, rect, url.c_str(), url.size());
                while (cppsocket_client_is_connected(client))
                {
                    cppsocket_client_frame(client);
                }
            }
            cppsocket_client_delete(client);
        }
        else if (input == 5)
        {
            RECT rect{ 0, 0, 7680, 2160 };
            std::string url = "C:\\Wizbrain\\media\\output.mp4";

            // Client
            void* client = cppsocket_client_create();

            cppsocket_client_set_callback_data(client, callback_ptr_client);

            if (cppsocket_client_connect(client, "127.0.0.1", 53333))
            {
                cppsocket_client_send_play(client, rect, url.c_str(), url.size());

                while (cppsocket_client_is_connected(client))
                {
                    cppsocket_client_frame(client);
                }
            }
            cppsocket_client_delete(client);
        }
    }
    cppsocket_network_shutdown();

    system("pause");

    return 0;
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
