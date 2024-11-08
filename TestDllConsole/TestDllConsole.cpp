// TestDllConsole.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>

#include "CppSocketAPI.h"
#include "PacketDefine.h"
#include "ApiFunctionStructures.h"

#include "thread"

#include "tchar.h"

static void* _server;
static void* _client;

bool _use_ini_setting = false;
int _type = 1;
std::string _ip;
uint16_t _port = 0;
int _left = 0;
int _top = 0;
int _width = 0;
int _height = 0;
std::string _url;


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
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "left: " << packet->left << ", ";
        std::cout << "top: " << packet->top << ", ";
        std::cout << "width: " << packet->width << ", ";
        std::cout << "height: " << packet->height << ", ";
        std::cout << "url_size: " << packet->url_size << ", ";
        std::cout << "url: " << packet->url << std::endl;

        cppsocket_struct_server_send_play data{};
        data.scene_index = 0;
        data.result = (uint16_t)packet_result::ok;
        data.left = packet->left;
        data.top = packet->top;
        data.width = packet->width;
        data.height = packet->height;
        data.url_size = packet->url_size;
        memcpy(data.url, packet->url, packet->url_size);

        cppsocket_server_send_play(_server, connection, data);

        delete packet;
    }
    break;
    case command_type::pause:
    {
        packet_pause_from_client* packet = new packet_pause_from_client();
        memcpy(packet, data, header->size);

        std::cout << "pause" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << std::endl;

        cppsocket_struct_server_send_pause data{};
        data.scene_index = packet->scene_index;
        data.result = (uint16_t)packet_result::ok;

        cppsocket_server_send_pause(_server, connection, data);

        delete packet;
    }
    break;
    case command_type::stop:
    {
        packet_stop_from_client* packet = new packet_stop_from_client();
        memcpy(packet, data, header->size);

        std::cout << "stop" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << std::endl;

        cppsocket_struct_server_send_stop data{};
        data.scene_index = packet->scene_index;
        data.result = (uint16_t)packet_result::ok;

        cppsocket_server_send_stop(_server, connection, data);

        delete packet;
    }
    break;
    case command_type::move:
    {
        // not implemented
    }
    break;
    case command_type::jump_forward:
    {
        packet_jump_forward_from_client* packet = new packet_jump_forward_from_client();
        memcpy(packet, data, header->size);

        std::cout << "jump_forwrad" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << std::endl;

        cppsocket_struct_server_send_jump_forward data{};
        data.scene_index = packet->scene_index;
        data.result = (uint16_t)packet_result::ok;

        cppsocket_server_send_jump_forward(_server, connection, data);

        delete packet;
    }
    break;
    case command_type::jump_backwards:
    {
        packet_jump_backwards_from_client* packet = new packet_jump_backwards_from_client();
        memcpy(packet, data, header->size);

        std::cout << "jump_backwards" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << std::endl;

        cppsocket_struct_server_send_jump_backwards data{};
        data.scene_index = packet->scene_index;
        data.result = (uint16_t)packet_result::ok;

        cppsocket_server_send_jump_backwards(_server, connection, data);

        delete packet;
    }
    break;
    case command_type::play_sync_group:
    {
        packet_play_sync_group_from_client* packet = new packet_play_sync_group_from_client();
        memcpy(packet, data, header->size);

        std::cout << "play_sync_group" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "left: " << packet->left << ", ";
        std::cout << "top: " << packet->top << ", ";
        std::cout << "width: " << packet->width << ", ";
        std::cout << "height: " << packet->height << ", ";
        std::cout << "url_size: " << packet->url_size << ", ";
        std::cout << "url: " << packet->url << ", ";
        std::cout << "sync_group_index: " << packet->sync_group_index << ", ";
        std::cout << "sync_group_count: " << packet->sync_group_count << std::endl;

        cppsocket_struct_server_send_play_sync_group data{};
        data.scene_index = 0;
        data.result = (uint16_t)packet_result::ok;
        data.left = packet->left;
        data.top = packet->top;
        data.width = packet->width;
        data.height = packet->height;
        data.url_size = packet->url_size;
        memcpy(data.url, packet->url, packet->url_size);
        data.sync_group_index = packet->sync_group_index;
        data.sync_group_count = packet->sync_group_count;

        cppsocket_server_send_play_sync_group(_server, connection, data);

        delete packet;
    }
    break;
    case command_type::pause_sync_group:
    {
        packet_pause_sync_group_from_client* packet = new packet_pause_sync_group_from_client();
        memcpy(packet, data, header->size);

        std::cout << "pause_sync_group" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "sync_group_index: " << packet->sync_group_index << std::endl;

        cppsocket_struct_server_send_pause_sync_group data{};
        data.sync_group_index = packet->sync_group_index;
        data.result = (uint16_t)packet_result::ok;

        cppsocket_server_send_pause_sync_group(_server, connection, data);

        delete packet;
    }
    break;
    case command_type::stop_sync_group:
    {
        packet_stop_sync_group_from_client* packet = new packet_stop_sync_group_from_client();
        memcpy(packet, data, header->size);

        std::cout << "stop_sync_group" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "sync_group_index: " << packet->sync_group_index << std::endl;

        cppsocket_struct_server_send_stop_sync_group data{};
        data.sync_group_index = packet->sync_group_index;
        data.result = (uint16_t)packet_result::ok;

        cppsocket_server_send_stop_sync_group(_server, connection, data);

        delete packet;
    }
    break;
    case command_type::program_quit:
    {
        std::cout << "program_quit" << std::endl;
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
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "left: " << packet->left << ", ";
        std::cout << "top: " << packet->top << ", ";
        std::cout << "width: " << packet->width << ", ";
        std::cout << "height: " << packet->height << ", ";
        std::cout << "url_size: " << packet->url_size << ", ";
        std::cout << "url: " << packet->url << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::pause:
    {
        packet_pause_from_server* packet = new packet_pause_from_server();
        memcpy(packet, data, header->size);

        std::cout << "pause" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::stop:
    {
        packet_stop_from_server* packet = new packet_stop_from_server();
        memcpy(packet, data, header->size);

        std::cout << "stop" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::move:
    {
        // not implemented
    }
    break;
    case command_type::jump_forward:
    {
        packet_jump_forward_from_server* packet = new packet_jump_forward_from_server();
        memcpy(packet, data, header->size);

        std::cout << "jump_forward" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::jump_backwards:
    {
        packet_jump_backwards_from_server* packet = new packet_jump_backwards_from_server();
        memcpy(packet, data, header->size);

        std::cout << "jump_backwards" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::play_sync_group:
    {
        packet_play_sync_group_from_server* packet = new packet_play_sync_group_from_server();
        memcpy(packet, data, header->size);

        std::cout << "play_sync_group" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "left: " << packet->left << ", ";
        std::cout << "top: " << packet->top << ", ";
        std::cout << "width: " << packet->width << ", ";
        std::cout << "height: " << packet->height << ", ";
        std::cout << "sync_group_index: " << packet->sync_group_index << ", ";
        std::cout << "sync_group_count: " << packet->sync_group_count << ", ";
        std::cout << "url_size: " << packet->url_size << ", ";
        std::cout << "url: " << packet->url << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::pause_sync_group:
    {
        packet_pause_sync_group_from_server* packet = new packet_pause_sync_group_from_server();
        memcpy(packet, data, header->size);

        std::cout << "pause_sync_group" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "sync_group_index: " << packet->sync_group_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::stop_sync_group:
    {
        packet_stop_sync_group_from_server* packet = new packet_stop_sync_group_from_server();
        memcpy(packet, data, header->size);

        std::cout << "stop_sync_group" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "sync_group_index: " << packet->sync_group_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;

    case command_type::gplayer_play_url:
    {
        packet_gplayer_play_url_from_server* packet = new packet_gplayer_play_url_from_server();
        memcpy(packet, data, header->size);

        std::cout << "gplayer_play_url" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "player_sync_group_input_count: " << packet->player_sync_group_input_count << ", ";
        std::cout << "url: " << packet->url << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::gplayer_play_url_different_videos:
    {
        packet_gplayer_play_url_different_videos_from_server* packet = new packet_gplayer_play_url_different_videos_from_server();
        memcpy(packet, data, header->size);

        std::cout << "gplayer_play_url_different_videos" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "player_sync_group_input_count: " << packet->player_sync_group_input_count << ", ";
        std::cout << "url: " << packet->url << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::gplayer_play_rect:
    {
        packet_gplayer_play_rect_from_server* packet = new packet_gplayer_play_rect_from_server();
        memcpy(packet, data, header->size);

        std::cout << "gplayer_play_rect" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "player_sync_group_output_count: " << packet->player_sync_group_output_count << ", ";
        std::cout << "left: " << packet->left << ", ";
        std::cout << "top: " << packet->top << ", ";
        std::cout << "width: " << packet->width << ", ";
        std::cout << "height: " << packet->height << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::gplayer_stop:
    {
        packet_gplayer_stop_from_server* packet = new packet_gplayer_stop_from_server();
        memcpy(packet, data, header->size);

        std::cout << "gplayer_stop" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;

    case command_type::dplayer_play_url:
    {
        packet_dplayer_play_url_from_server* packet = new packet_dplayer_play_url_from_server();
        memcpy(packet, data, header->size);

        std::cout << "dplayer_play_url" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "player_sync_group_input_count: " << packet->player_sync_group_input_count << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "url: " << packet->url << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::dplayer_play_rect:
    {
        packet_dplayer_play_rect_from_server* packet = new packet_dplayer_play_rect_from_server();
        memcpy(packet, data, header->size);

        std::cout << "dplayer_play_rect" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "player_sync_group_output_count: " << packet->player_sync_group_output_count << ", ";
        std::cout << "scene_index: " << packet->scene_index << ", ";
        std::cout << "left: " << packet->left << ", ";
        std::cout << "top: " << packet->top << ", ";
        std::cout << "width: " << packet->width << ", ";
        std::cout << "height: " << packet->height << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;
    case command_type::dplayer_stop:
    {
        packet_dplayer_stop_from_server* packet = new packet_dplayer_stop_from_server();
        memcpy(packet, data, header->size);

        std::cout << "dplayer_stop" << ", ";
        std::cout << "command_type: " << (uint16_t)packet->header.cmd << ", ";
        std::cout << "packet size: " << packet->header.size << ", ";
        std::cout << "player_sync_group_index: " << packet->player_sync_group_index << ", ";
        std::cout << "result: " << (uint16_t)packet->result << std::endl;

        delete packet;
    }
    break;

    default:
        break;
    }
}

void client_frame_thread(const char * ip, uint16_t port)
{
    // Client
    void* client = cppsocket_client_create();

    cppsocket_client_set_callback_data(client, callback_ptr_client);

    if (cppsocket_client_connect(client, ip, port))
    {
        _client = client;

        while (cppsocket_client_is_connected(client))
        {
            cppsocket_client_frame(client);
        }
    }
    cppsocket_client_delete(client);
}

void client_output_messages_step_1()
{
    std::cout << "command list" << std::endl;
    std::cout << "0(play)" << std::endl;
    std::cout << "1(pause)" << std::endl;
    std::cout << "2(stop)" << std::endl;
    std::cout << "3(move) (not implemented)" << std::endl;
    std::cout << "4(jump_forward)" << std::endl;
    std::cout << "5(jump_backwards)" << std::endl;
    std::cout << "6(seek_repeat_self) (internal command type, not implemented)" << std::endl;
    std::cout << "7(play_sync_group)" << std::endl;
    std::cout << "8(pause_sync_group)" << std::endl;
    std::cout << "9(stop_sync_group)" << std::endl;
    std::cout << "10(sync_group_frame_numbering) (internal command type, not implemented)" << std::endl;
    std::cout << "11(seek_repeat_self_sync_Group) (internal command type, not implemented)" << std::endl;
    std::cout << "12(program_quit)" << std::endl;
    std::cout << std::endl;
    std::cout << "13(gplayer_play_url)" << std::endl;
    std::cout << "14(gplayer_play_rect)" << std::endl;
    std::cout << "15(gplayer_play_stop)" << std::endl;
    std::cout << "16(gplayer_play_pause) (not implemented)" << std::endl;
    std::cout << std::endl;
    std::cout << "20(dplayer_play_url)" << std::endl;
    std::cout << "21(dplayer_play_rect)" << std::endl;
    std::cout << "22(dplayer_play_stop)" << std::endl;
    std::cout << std::endl;
    std::cout << "26(gplayer_play_url_different_videos)" << std::endl;

    std::cout << std::endl;
    std::cout << "input 99 to stop program" << std::endl;
}

void config_setting()
{
    wchar_t path_w[260] = { 0, };
    GetModuleFileName(nullptr, path_w, 260);
    std::wstring str_path_w = path_w;
    str_path_w = str_path_w.substr(0, str_path_w.find_last_of(L"\\/"));
    std::wstring str_ini_path_w = str_path_w + L"\\TestDllConsole.ini";

    char path_a[260] = { 0, };
    GetModuleFileNameA(nullptr, path_a, 260);
    std::string str_path_a = path_a;
    str_path_a = str_path_a.substr(0, str_path_a.find_last_of("\\/"));
    std::string str_ini_path_a = str_path_a + "\\TestDllConsole.ini";

    char result_a[255];
    wchar_t result_w[255];
    int result_i = 0;

    GetPrivateProfileString(L"TestDllConsole", L"USE_INI_SETTING", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _use_ini_setting = result_i == 0 ? false : true;

    GetPrivateProfileString(L"TestDllConsole", L"TYPE", L"-1", result_w, 255, str_ini_path_w.c_str());
    _type = _ttoi(result_w);

    GetPrivateProfileStringA("TestDllConsole", "IP", "", result_a, 255, str_ini_path_a.c_str());
    _ip = result_a;

    GetPrivateProfileString(L"TestDllConsole", L"PORT", L"0", result_w, 255, str_ini_path_w.c_str());
    _port = _ttoi(result_w);

    GetPrivateProfileString(L"TestDllConsole", L"Left", L"0", result_w, 255, str_ini_path_w.c_str());
    _left = _ttoi(result_w);

    GetPrivateProfileString(L"TestDllConsole", L"Top", L"0", result_w, 255, str_ini_path_w.c_str());
    _top = _ttoi(result_w);

    GetPrivateProfileString(L"TestDllConsole", L"Width", L"0", result_w, 255, str_ini_path_w.c_str());
    _width = _ttoi(result_w);

    GetPrivateProfileString(L"TestDllConsole", L"Height", L"0", result_w, 255, str_ini_path_w.c_str());
    _height = _ttoi(result_w);
    
    GetPrivateProfileStringA("TestDllConsole", "URL", "", result_a, 255, str_ini_path_a.c_str());
    _url = result_a;
}

int main()
{
    config_setting();

    int input = 0;
    
    if (_type < 0 || _use_ini_setting == false)
    {
        std::cout << "Input" << std::endl;
        std::cout << "0 : server start" << std::endl;
        std::cout << "1 : client start" << std::endl;

        std::cin >> input;
    }
    else
    {
        input = _type;
    }

    if (cppsocket_network_initialize())
    {
        std::cout << "Winsock api successfully initialized." << std::endl;
        
        std::string ip;
        uint16_t port = 0;

        if (_ip.empty() || _port == 0 || _use_ini_setting == false)
        {
            std::cout << "Enter Using ip and port. e.g) 127.0.0.1 53333" << std::endl;
            std::cin >> ip >> port;
        }
        else
        {
            ip = _ip;
            port = _port;
        }

        if (input == 0)
        {
            // Server
            void* server = cppsocket_server_create();
            
            cppsocket_server_set_callback_data_connection(server, callback_data_connection_server);

            if (cppsocket_server_initialize(server, ip.c_str(), port))
            {
                _server = server;
                while (true)
                {
                    cppsocket_server_frame(_server);
                }
            }
            cppsocket_server_delete(server);
        }
        else if (input == 1)
        {
            std::thread client_thread(client_frame_thread, ip.c_str(), port);
            client_thread.detach();

            bool auto_play = false;

            if (!(_left == 0 && _top == 0 && _width == 0 && _height == 0) && _use_ini_setting == true)
            {
                auto_play = true;
            }

            while (true)
            {
                uint16_t cmd;

                if (auto_play == true)
                {
                    auto_play = false;

                    while (!_client)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    while (!cppsocket_client_is_connected(_client))
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }

                    cppsocket_struct_client_send_play data{};
                    data.left = _left;
                    data.top = _top;
                    data.width = _width;
                    data.height = _height;
                    data.url_size = _url.size();
                    data.url = _url.c_str();

                    cppsocket_client_send_play(_client, data);

                    continue;
                }

                client_output_messages_step_1();
                std::cin >> cmd;
                
                switch ((command_type)cmd)
                {
                case command_type::play:
                {
                    std::cout << "play, Input left top width height url" << std::endl;

                    int left;
                    int top;
                    int width;
                    int height;
                    std::string url;
                    std::cin >> left >> top >> width >> height >> url;

                    cppsocket_struct_client_send_play data{};
                    data.left = left;
                    data.top = top;
                    data.width = width;
                    data.height = height;
                    data.url_size = url.size();
                    data.url = url.c_str();

                    cppsocket_client_send_play(_client, data);
                }
                break;
                case command_type::pause:
                {
                    std::cout << "pause, Input scene_index" << std::endl;

                    uint32_t scene_index;
                    std::cin >> scene_index;

                    cppsocket_struct_client_send_pause data{};
                    data.scene_index = scene_index;

                    cppsocket_client_send_pause(_client, data);
                }
                break;
                case command_type::stop:
                {
                    std::cout << "stop, Input scene_index" << std::endl;

                    uint32_t scene_index;
                    std::cin >> scene_index;

                    cppsocket_struct_client_send_stop data{};
                    data.scene_index = scene_index;

                    cppsocket_client_send_stop(_client, data);
                }
                break;
                case command_type::move:
                {
                    // not implemented
                }
                break;
                case command_type::jump_forward:
                {
                    std::cout << "jump_forward, Input scene_index" << std::endl;

                    uint32_t scene_index;
                    std::cin >> scene_index;

                    cppsocket_struct_client_send_jump_forward data{};
                    data.scene_index = scene_index;

                    cppsocket_client_send_jump_forward(_client, data);
                }
                break;
                case command_type::jump_backwards:
                {
                    std::cout << "jump_backwards, Input scene_index" << std::endl;

                    uint32_t scene_index;
                    std::cin >> scene_index;

                    cppsocket_struct_client_send_jump_backwards data{};
                    data.scene_index = scene_index;

                    cppsocket_client_send_jump_backwards(_client, data);
                }
                break;
                case command_type::play_sync_group:
                {
                    std::cout << "play, Input left top width height url sync_group_index sync_group_count" << std::endl;

                    int left;
                    int top;
                    int width;
                    int height;
                    std::string url;
                    uint32_t sync_group_index;
                    uint16_t sync_group_count;

                    std::cin >> left >> top >> width >> height >> url;
                    std::cin >> sync_group_index >> sync_group_count;

                    cppsocket_struct_client_send_play_sync_group data{};
                    data.left = left;
                    data.top = top;
                    data.width = width;
                    data.height = height;
                    data.url_size = url.size();
                    data.url = url.c_str();
                    data.sync_group_index = sync_group_index;
                    data.sync_group_count = sync_group_count;

                    cppsocket_client_send_play_sync_group(_client, data);
                }
                break;
                case command_type::pause_sync_group:
                {
                    std::cout << "pause, Input sync_group_index" << std::endl;

                    uint32_t sync_group_index;
                    std::cin >> sync_group_index;

                    cppsocket_struct_client_send_pause_sync_group data{};
                    data.sync_group_index = sync_group_index;

                    cppsocket_client_send_pause_sync_group(_client, data);
                }
                break;
                case command_type::stop_sync_group:
                {
                    std::cout << "stop, Input sync_group_index" << std::endl;

                    uint32_t sync_group_index;
                    std::cin >> sync_group_index;

                    cppsocket_struct_client_send_stop_sync_group data{};
                    data.sync_group_index = sync_group_index;

                    cppsocket_client_send_stop_sync_group(_client, data);
                }
                break;
                case command_type::program_quit:
                {
                    cppsocket_client_send_program_quit(_client);
                }
                break;

                case command_type::gplayer_play_url:
                {
                    std::cout << "gplayer_play_url, Input url player_sync_group_index player_sync_group_input_count" << std::endl;

                    std::string url;
                    uint32_t player_sync_group_index;
                    uint16_t player_sync_group_input_count;

                    std::cin >> url;
                    std::cin >> player_sync_group_index >> player_sync_group_input_count;

                    cppsocket_struct_client_send_gplayer_play_url data{};
                    data.url_size = url.size();
                    data.url = url.c_str();
                    data.player_sync_group_index = player_sync_group_index;
                    data.player_sync_group_input_count = player_sync_group_input_count;

                    cppsocket_client_send_gplayer_play_url(_client, data);
                }
                break;
                case command_type::gplayer_play_url_different_videos:
                {
                    std::cout << "gplayer_play_url_different_videos, Input url player_sync_group_index player_sync_group_input_count" << std::endl;

                    std::string url;
                    uint32_t player_sync_group_index;
                    uint16_t player_sync_group_input_count;

                    std::cin >> url;
                    std::cin >> player_sync_group_index >> player_sync_group_input_count;

                    cppsocket_struct_client_send_gplayer_play_url_different_videos data{};
                    data.url_size = url.size();
                    data.url = url.c_str();
                    data.player_sync_group_index = player_sync_group_index;
                    data.player_sync_group_input_count = player_sync_group_input_count;

                    cppsocket_client_send_gplayer_play_url_different_videos(_client, data);
                }
                break;
                case command_type::gplayer_play_rect:
                {
                    std::cout << "gplayer_play_rect, Input left top width height player_sync_group_index player_sync_group_output_count" << std::endl;

                    int left;
                    int top;
                    int width;
                    int height;
                    uint32_t player_sync_group_index;
                    uint16_t player_sync_group_output_count;

                    std::cin >> left >> top >> width >> height;
                    std::cin >> player_sync_group_index >> player_sync_group_output_count;

                    cppsocket_struct_client_send_gplayer_play_rect data{};
                    data.left = left;
                    data.top = top;
                    data.width = width;
                    data.height = height;
                    data.player_sync_group_index = player_sync_group_index;
                    data.player_sync_group_output_count = player_sync_group_output_count;

                    cppsocket_client_send_gplayer_play_rect(_client, data);
                }
                break;
                case command_type::gplayer_stop:
                {
                    std::cout << "gplayer_stop, Input player_sync_group_index" << std::endl;

                    uint32_t player_sync_group_index;

                    std::cin >> player_sync_group_index;

                    cppsocket_struct_client_send_gplayer_stop data{};
                    data.player_sync_group_index = player_sync_group_index;

                    cppsocket_client_send_gplayer_stop(_client, data);
                }
                break;

                case command_type::dplayer_play_url:
                {
                    std::cout << "dplayer_play_url, Input url player_sync_group_index player_sync_group_input_count scene_index" << std::endl;
                    
                    std::string url;
                    uint32_t player_sync_group_index;
                    uint16_t player_sync_group_input_count;
                    uint32_t scene_index;

                    std::cin >> url;
                    std::cin >> player_sync_group_index >> player_sync_group_input_count >> scene_index;

                    cppsocket_struct_client_send_dplayer_play_url data{};
                    data.url_size = url.size();
                    data.url = url.c_str();
                    data.player_sync_group_index = player_sync_group_index;
                    data.player_sync_group_input_count = player_sync_group_input_count;
                    data.scene_index = scene_index;

                    cppsocket_client_send_dplayer_play_url(_client, data);
                }
                break;
                case command_type::dplayer_play_rect:
                {
                    std::cout << "dplayer_play_rect, Input left top width height player_sync_group_index player_sync_group_output_count scene_index" << std::endl;

                    int left;
                    int top;
                    int width;
                    int height;
                    uint32_t player_sync_group_index;
                    uint16_t player_sync_group_output_count;
                    uint32_t scene_index;

                    std::cin >> left >> top >> width >> height;
                    std::cin >> player_sync_group_index >> player_sync_group_output_count >> scene_index;

                    cppsocket_struct_client_send_dplayer_play_rect data{};
                    data.left = left;
                    data.top = top;
                    data.width = width;
                    data.height = height;
                    data.player_sync_group_index = player_sync_group_index;
                    data.player_sync_group_output_count = player_sync_group_output_count;
                    data.scene_index = scene_index;

                    cppsocket_client_send_dplayer_play_rect(_client, data);
                }
                break;
                case command_type::dplayer_stop:
                {
                    std::cout << "dplayer_stop, Input player_sync_group_index" << std::endl;

                    uint32_t player_sync_group_index;

                    std::cin >> player_sync_group_index;

                    cppsocket_struct_client_send_dplayer_stop data{};
                    data.player_sync_group_index = player_sync_group_index;

                    cppsocket_client_send_dplayer_stop(_client, data);
                }
                break;

                default:
                    break;
                }

                if (cmd == 99)
                {
                    break;
                }
            }
            cppsocket_client_connection_close(_client);
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
