// PlayerCommandConsole.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#include <Windows.h>

#include <tchar.h>

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"
#pragma comment(lib, "spdlog.lib")

#include "CppSocketAPI.h"
#include "PacketDefine.h"
#include "ApiFunctionStructures.h"
#pragma comment(lib, "CppSocket.lib")

#include <map>
#include <thread>
#include <queue>
#include <mutex>

#include <TlHelp32.h>

constexpr uint64_t _sleep_time = 1;

enum class PlayerType : int
{
    gplayer,    // 0
    dplayer,    // 1
};

typedef struct st_coordinate
{
    int left;
    int top;
    int width;
    int height;
}*pst_coordinate;

typedef struct st_gplayer
{
    uint32_t player_sync_group_index = UINT32_MAX;

    uint16_t player_sync_group_input_count = UINT16_MAX;
    std::vector<std::string> vector_url;

    uint16_t player_sync_group_output_count = UINT16_MAX;
    std::vector<pst_coordinate> vector_coordinate;

    bool flag_different_videos = false;

}*pst_gplayer;

typedef struct st_dplayer
{
    uint32_t player_sync_group_index = UINT32_MAX;

    uint16_t player_sync_group_input_count = UINT16_MAX;
    std::map<uint32_t, std::string> map_url;

    uint16_t player_sync_group_output_count = UINT16_MAX;
    std::map<uint32_t, pst_coordinate> map_coordinate;

}*pst_dplayer;

bool _flag_running = true;

void* _server = nullptr;
std::thread _thread_server;
bool _flag_thread_server = true;

// first: packet_data, second: connection
// CppSocket 콜백 명령 저장 큐
std::deque<std::pair<void*, void*>> _queue_packet_processing;
std::mutex _mutex_packet_processing;
std::thread _thread_packet_processing;
bool _flag_thread_packet_processing = true;

std::string _ip;
uint16_t _port = 0;

std::map<uint32_t, pst_gplayer> _map_gplayer;
std::map<uint32_t, pst_dplayer> _map_dplayer;
std::map<uint32_t, void*> _map_player_connection;

std::string _application_name_gplayer;
std::string _application_name_dplayer;

bool _flag_set_logger = false;

int _log_level = 6;

void thread_server();

// CppSocket 명령 저장 콜백
void callback_data_connection_server(void* data, void* connection);

// CppSocket 콜백 명령 처리
void thread_packet_processing();

void process_start_player(uint32_t player_sync_group_index, PlayerType type);

void get_config();

void set_logger();

void start_process_on_user_session(uint32_t player_sync_group_index, PlayerType type);

int main()
{
#if _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG

    get_config();

    set_logger();

    _thread_packet_processing = std::thread(thread_packet_processing);
    _thread_server = std::thread(thread_server);

    while (_flag_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));
    }

    if (_thread_packet_processing.joinable())
    {
        _flag_thread_packet_processing = false;
        _thread_packet_processing.join();
    }

    if (_thread_server.joinable())
    {
        _flag_thread_server = false;
        _thread_server.join();
    }
}

void thread_server()
{
    cppsocket_network_initialize();

    _server = cppsocket_server_create();

    cppsocket_server_set_callback_data_connection(_server, callback_data_connection_server);

    if (cppsocket_server_initialize(_server, _ip.c_str(), _port))
    {
        while (_flag_thread_server)
        {
            cppsocket_server_frame(_server);
        }
    }

    cppsocket_server_delete(_server);

    cppsocket_network_shutdown();
}

void callback_data_connection_server(void* data, void* connection)
{
    packet_header* header = (packet_header*)data;

    void* packet = new char[header->size];
    memcpy(packet, data, header->size);

    {
        std::lock_guard<std::mutex> lk(_mutex_packet_processing);
        _queue_packet_processing.push_back(std::pair<void*, void*>(packet, connection));
    }
}
void thread_packet_processing()
{
    while (_flag_thread_packet_processing)
    {
        bool flag_packet_processing_command_is_empty = false;
        {
            std::lock_guard<std::mutex> lk(_mutex_packet_processing);
            flag_packet_processing_command_is_empty = _queue_packet_processing.empty();
        }

        if (flag_packet_processing_command_is_empty)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));
            continue;
        }

        // first: packet_data, second: connection
        std::pair<void*, void*> data_pair;
        {
            std::lock_guard<std::mutex> lk(_mutex_packet_processing);
            data_pair = _queue_packet_processing.front();
            _queue_packet_processing.pop_front();
        }

        packet_header* header = (packet_header*)data_pair.first;

        switch (header->cmd)
        {
            // 여기에 코드 추가 작성
            // Write Code Here!

        case command_type::gplayer_play_url:
        {
            packet_gplayer_play_url_from_client* packet = (packet_gplayer_play_url_from_client*)data_pair.first;

            pst_gplayer gplayer = nullptr;

            auto it_gplayer = _map_gplayer.find(packet->player_sync_group_index);
            if (it_gplayer != _map_gplayer.end())
            {
                gplayer = it_gplayer->second;
            }
            else
            {
                gplayer = new st_gplayer();
                gplayer->flag_different_videos = false;
                _map_gplayer.insert({ packet->player_sync_group_index, gplayer });
            }

            if (gplayer->player_sync_group_index == UINT32_MAX)
            {
                gplayer->player_sync_group_index = packet->player_sync_group_index;
            }

            if (gplayer->vector_url.size() == 0)
            {
                gplayer->player_sync_group_input_count = packet->player_sync_group_input_count;
            }

            std::string url;
            url.assign(packet->url);

            gplayer->vector_url.push_back(url);

            {
                cppsocket_struct_server_send_gplayer_play_url data{};
                data.result = (uint16_t)packet_result::ok;
                data.player_sync_group_index = packet->player_sync_group_index;
                data.player_sync_group_input_count = packet->player_sync_group_input_count;
                data.url_size = packet->url_size;
                memcpy(data.url, packet->url, packet->url_size);

                cppsocket_server_send_gplayer_play_url(_server, data_pair.second, data);
            }

            if (gplayer->vector_url.size() == gplayer->player_sync_group_input_count
                && gplayer->vector_coordinate.size() == gplayer->player_sync_group_output_count)
            {
                process_start_player(gplayer->player_sync_group_index, PlayerType::gplayer);
            }
        }
        break;
        case command_type::gplayer_play_url_different_videos:
        {
            packet_gplayer_play_url_different_videos_from_client* packet = (packet_gplayer_play_url_different_videos_from_client*)data_pair.first;

            pst_gplayer gplayer = nullptr;

            auto it_gplayer = _map_gplayer.find(packet->player_sync_group_index);
            if (it_gplayer != _map_gplayer.end())
            {
                gplayer = it_gplayer->second;
            }
            else
            {
                gplayer = new st_gplayer();
                gplayer->flag_different_videos = true;
                _map_gplayer.insert({ packet->player_sync_group_index, gplayer });
            }

            if (gplayer->player_sync_group_index == UINT32_MAX)
            {
                gplayer->player_sync_group_index = packet->player_sync_group_index;
            }

            if (gplayer->vector_url.size() == 0)
            {
                gplayer->player_sync_group_input_count = packet->player_sync_group_input_count;
            }

            std::string url;
            url.assign(packet->url);

            gplayer->vector_url.push_back(url);

            {
                cppsocket_struct_server_send_gplayer_play_url_different_videos data{};
                data.result = (uint16_t)packet_result::ok;
                data.player_sync_group_index = packet->player_sync_group_index;
                data.player_sync_group_input_count = packet->player_sync_group_input_count;
                data.url_size = packet->url_size;
                memcpy(data.url, packet->url, packet->url_size);

                cppsocket_server_send_gplayer_play_url_different_videos(_server, data_pair.second, data);
            }

            if (gplayer->vector_url.size() == gplayer->player_sync_group_input_count
                && gplayer->vector_coordinate.size() == gplayer->player_sync_group_output_count)
            {
                process_start_player(gplayer->player_sync_group_index, PlayerType::gplayer);
            }
        }
        break;
        case command_type::gplayer_play_rect:
        {
            packet_gplayer_play_rect_from_client* packet = (packet_gplayer_play_rect_from_client*)data_pair.first;

            pst_gplayer gplayer = nullptr;

            auto it_gplayer = _map_gplayer.find(packet->player_sync_group_index);
            if (it_gplayer != _map_gplayer.end())
            {
                gplayer = it_gplayer->second;
            }
            else
            {
                gplayer = new st_gplayer();
                _map_gplayer.insert({ packet->player_sync_group_index, gplayer });
            }

            if (gplayer->player_sync_group_index == UINT32_MAX)
            {
                gplayer->player_sync_group_index = packet->player_sync_group_index;
            }

            if (gplayer->vector_coordinate.size() == 0)
            {
                gplayer->player_sync_group_output_count = packet->player_sync_group_output_count;
            }

            pst_coordinate coordinate = new st_coordinate();
            coordinate->left = packet->left;
            coordinate->top = packet->top;
            coordinate->width = packet->width;
            coordinate->height = packet->height;

            gplayer->vector_coordinate.push_back(coordinate);

            {
                cppsocket_struct_server_send_gplayer_play_rect data{};
                data.result = (uint16_t)packet_result::ok;
                data.player_sync_group_index = packet->player_sync_group_index;
                data.player_sync_group_output_count = packet->player_sync_group_output_count;
                data.left = packet->left;
                data.top = packet->top;
                data.width = packet->width;
                data.height = packet->height;

                cppsocket_server_send_gplayer_play_rect(_server, data_pair.second, data);
            }

            if (gplayer->vector_url.size() == gplayer->player_sync_group_input_count
                && gplayer->vector_coordinate.size() == gplayer->player_sync_group_output_count)
            {
                process_start_player(gplayer->player_sync_group_index, PlayerType::gplayer);
            }
        }
        break;
        case command_type::gplayer_connect:
        {
            packet_player_connect_from_client* packet = (packet_player_connect_from_client*)data_pair.first;

            auto it_player_connection = _map_player_connection.find(packet->player_sync_group_index);
            if (it_player_connection == _map_player_connection.end())
            {
                _map_player_connection.insert({ packet->player_sync_group_index, data_pair.second });
            }

            auto it_gplayer = _map_gplayer.find(packet->player_sync_group_index);
            if (it_gplayer != _map_gplayer.end())
            {
                pst_gplayer gplayer = it_gplayer->second;

                if (gplayer->flag_different_videos)
                {
                    for (size_t i = 0; i < gplayer->vector_url.size(); i++)
                    {
                        cppsocket_struct_server_send_gplayer_connect_data_url_different_videos data{};
                        data.result = (uint16_t)packet_result::ok;
                        data.player_sync_group_index = gplayer->player_sync_group_index;
                        data.player_sync_group_input_count = gplayer->player_sync_group_input_count;
                        data.url_size = gplayer->vector_url.at(i).size();
                        memcpy(data.url, gplayer->vector_url.at(i).c_str(), data.url_size);

                        cppsocket_server_send_gplayer_connect_data_url_different_videos(_server, data_pair.second, data);
                    }
                }
                else
                {
                    for (size_t i = 0; i < gplayer->vector_url.size(); i++)
                    {
                        cppsocket_struct_server_send_gplayer_connect_data_url data{};
                        data.result = (uint16_t)packet_result::ok;
                        data.player_sync_group_index = gplayer->player_sync_group_index;
                        data.player_sync_group_input_count = gplayer->player_sync_group_input_count;
                        data.url_size = gplayer->vector_url.at(i).size();
                        memcpy(data.url, gplayer->vector_url.at(i).c_str(), data.url_size);

                        cppsocket_server_send_gplayer_connect_data_url(_server, data_pair.second, data);
                    }
                }

                for (size_t i = 0; i < gplayer->vector_coordinate.size(); i++)
                {
                    cppsocket_struct_server_send_gplayer_connect_data_rect data{};
                    data.result = (uint16_t)packet_result::ok;
                    data.player_sync_group_index = gplayer->player_sync_group_index;
                    data.player_sync_group_output_count = gplayer->player_sync_group_output_count;
                    data.left = gplayer->vector_coordinate.at(i)->left;
                    data.top = gplayer->vector_coordinate.at(i)->top;
                    data.width = gplayer->vector_coordinate.at(i)->width;
                    data.height = gplayer->vector_coordinate.at(i)->height;

                    cppsocket_server_send_gplayer_connect_data_rect(_server, data_pair.second, data);
                }
            }
        }
        break;
        case command_type::gplayer_stop:
        {
            bool err = false;

            packet_gplayer_stop_from_client* packet = (packet_gplayer_stop_from_client*)data_pair.first;

            pst_gplayer gplayer = nullptr;
            void* player_connection = nullptr;

            auto it_gplayer = _map_gplayer.find(packet->player_sync_group_index);
            if (it_gplayer == _map_gplayer.end())
            {
                err = true;
            }
            else
            {
                gplayer = it_gplayer->second;
            }

            auto it_player_connection = _map_player_connection.find(packet->player_sync_group_index);
            if (it_player_connection == _map_player_connection.end())
            {
                err = true;
            }
            else
            {
                player_connection = it_player_connection->second;
            }

            if (err)
            {
                cppsocket_struct_server_send_gplayer_stop data{};
                data.result = (uint16_t)packet_result::fail;
                data.player_sync_group_index = packet->player_sync_group_index;

                // to_console
                cppsocket_server_send_gplayer_stop(_server, data_pair.second, data);
            }
            else
            {
                cppsocket_struct_server_send_gplayer_stop data{};
                data.result = (uint16_t)packet_result::ok;
                data.player_sync_group_index = packet->player_sync_group_index;

                // to_console
                cppsocket_server_send_gplayer_stop(_server, data_pair.second, data);
                // to_gplayer
                cppsocket_server_send_gplayer_stop(_server, player_connection, data);

                for (auto it_url = gplayer->vector_url.begin(); it_url != gplayer->vector_url.end();)
                {
                    it_url = gplayer->vector_url.erase(it_url);
                }

                for (auto it_coordinate = gplayer->vector_coordinate.begin(); it_coordinate != gplayer->vector_coordinate.end();)
                {
                    pst_coordinate coordinate = *it_coordinate;
                    delete coordinate;

                    it_coordinate = gplayer->vector_coordinate.erase(it_coordinate);
                }

                delete gplayer;
                _map_gplayer.erase(it_gplayer);

                _map_player_connection.erase(it_player_connection);
            }
        }
        break;

        case command_type::dplayer_play_url:
        {
            packet_dplayer_play_url_from_client* packet = (packet_dplayer_play_url_from_client*)data_pair.first;

            pst_dplayer dplayer = nullptr;

            auto it_dplayer = _map_dplayer.find(packet->player_sync_group_index);
            if (it_dplayer != _map_dplayer.end())
            {
                dplayer = it_dplayer->second;
            }
            else
            {
                dplayer = new st_dplayer();
                _map_dplayer.insert({ packet->player_sync_group_index, dplayer });
            }

            if (dplayer->player_sync_group_index == UINT32_MAX)
            {
                dplayer->player_sync_group_index = packet->player_sync_group_index;
            }

            if (dplayer->map_url.size() == 0)
            {
                dplayer->player_sync_group_input_count = packet->player_sync_group_input_count;
            }

            std::string url;
            url.assign(packet->url);

            dplayer->map_url.insert({ packet->scene_index, url });

            {
                cppsocket_struct_server_send_dplayer_play_url data{};
                data.result = (uint16_t)packet_result::ok;
                data.player_sync_group_index = packet->player_sync_group_index;
                data.player_sync_group_input_count = packet->player_sync_group_input_count;
                data.url_size = packet->url_size;
                memcpy(data.url, packet->url, packet->url_size);

                cppsocket_server_send_dplayer_play_url(_server, data_pair.second, data);
            }

            if (dplayer->map_url.size() == dplayer->player_sync_group_input_count
                && dplayer->map_coordinate.size() == dplayer->player_sync_group_output_count)
            {
                process_start_player(dplayer->player_sync_group_index, PlayerType::dplayer);
            }
        }
        break;
        case command_type::dplayer_play_rect:
        {
            packet_dplayer_play_rect_from_client* packet = (packet_dplayer_play_rect_from_client*)data_pair.first;

            pst_dplayer dplayer = nullptr;

            auto it_dplayer = _map_dplayer.find(packet->player_sync_group_index);
            if (it_dplayer != _map_dplayer.end())
            {
                dplayer = it_dplayer->second;
            }
            else
            {
                dplayer = new st_dplayer();
                _map_dplayer.insert({ packet->player_sync_group_index, dplayer });
            }

            if (dplayer->player_sync_group_index == UINT32_MAX)
            {
                dplayer->player_sync_group_index = packet->player_sync_group_index;
            }

            if (dplayer->map_coordinate.size() == 0)
            {
                dplayer->player_sync_group_output_count = packet->player_sync_group_output_count;
            }

            pst_coordinate coordinate = new st_coordinate();
            coordinate->left = packet->left;
            coordinate->top = packet->top;
            coordinate->width = packet->width;
            coordinate->height = packet->height;

            dplayer->map_coordinate.insert({ packet->scene_index, coordinate });

            {
                cppsocket_struct_server_send_dplayer_play_rect data{};
                data.result = (uint16_t)packet_result::ok;
                data.player_sync_group_index = packet->player_sync_group_index;
                data.player_sync_group_output_count = packet->player_sync_group_output_count;
                data.left = packet->left;
                data.top = packet->top;
                data.width = packet->width;
                data.height = packet->height;

                cppsocket_server_send_dplayer_play_rect(_server, data_pair.second, data);
            }

            if (dplayer->map_url.size() == dplayer->player_sync_group_input_count
                && dplayer->map_coordinate.size() == dplayer->player_sync_group_output_count)
            {
                process_start_player(dplayer->player_sync_group_index, PlayerType::dplayer);
            }
        }
        break;
        case command_type::dplayer_connect:
        {
            packet_player_connect_from_client* packet = (packet_player_connect_from_client*)data_pair.first;

            auto it_player_connection = _map_player_connection.find(packet->player_sync_group_index);
            if (it_player_connection == _map_player_connection.end())
            {
                _map_player_connection.insert({ packet->player_sync_group_index, data_pair.second });
            }

            auto it_dplayer = _map_dplayer.find(packet->player_sync_group_index);
            if (it_dplayer != _map_dplayer.end())
            {
                pst_dplayer dplayer = it_dplayer->second;

                for (auto it_url = dplayer->map_url.begin(); it_url != dplayer->map_url.end(); it_url++)
                {
                    cppsocket_struct_server_send_dplayer_connect_data_url data{};
                    data.result = (uint16_t)packet_result::ok;
                    data.player_sync_group_index = dplayer->player_sync_group_index;
                    data.player_sync_group_input_count = dplayer->player_sync_group_input_count;
                    data.url_size = it_url->second.size();
                    memcpy(data.url, it_url->second.c_str(), data.url_size);
                    data.scene_index = it_url->first;

                    cppsocket_server_send_dplayer_connect_data_url(_server, data_pair.second, data);
                }

                for (auto it_coordinate = dplayer->map_coordinate.begin(); it_coordinate != dplayer->map_coordinate.end(); it_coordinate++)
                {
                    pst_coordinate data_coordinate = it_coordinate->second;

                    cppsocket_struct_server_send_dplayer_connect_data_rect data{};
                    data.result = (uint16_t)packet_result::ok;
                    data.player_sync_group_index = dplayer->player_sync_group_index;
                    data.player_sync_group_output_count = dplayer->player_sync_group_output_count;
                    data.left = data_coordinate->left;
                    data.top = data_coordinate->top;
                    data.width = data_coordinate->width;
                    data.height = data_coordinate->height;
                    data.scene_index = it_coordinate->first;

                    cppsocket_server_send_dplayer_connect_data_rect(_server, data_pair.second, data);
                }
            }
        }
        break;
        case command_type::dplayer_stop:
        {
            bool err = false;

            packet_dplayer_stop_from_client* packet = (packet_dplayer_stop_from_client*)data_pair.first;

            pst_dplayer dplayer = nullptr;
            void* player_connection = nullptr;

            auto it_dplayer = _map_dplayer.find(packet->player_sync_group_index);
            if (it_dplayer == _map_dplayer.end())
            {
                err = true;
            }
            else
            {
                dplayer = it_dplayer->second;
            }

            auto it_player_connection = _map_player_connection.find(packet->player_sync_group_index);
            if (it_player_connection == _map_player_connection.end())
            {
                err = true;
            }
            else
            {
                player_connection = it_player_connection->second;
            }

            if (err)
            {
                cppsocket_struct_server_send_dplayer_stop data{};
                data.result = (uint16_t)packet_result::fail;
                data.player_sync_group_index = packet->player_sync_group_index;

                // to_console
                cppsocket_server_send_dplayer_stop(_server, data_pair.second, data);
            }
            else
            {
                cppsocket_struct_server_send_dplayer_stop data{};
                data.result = (uint16_t)packet_result::ok;
                data.player_sync_group_index = packet->player_sync_group_index;

                // to_console
                cppsocket_server_send_dplayer_stop(_server, data_pair.second, data);
                // to_gplayer
                cppsocket_server_send_dplayer_stop(_server, player_connection, data);

                for (auto it_url = dplayer->map_url.begin(); it_url != dplayer->map_url.end();)
                {
                    it_url = dplayer->map_url.erase(it_url);
                }

                for (auto it_coordinate = dplayer->map_coordinate.begin(); it_coordinate != dplayer->map_coordinate.end();)
                {
                    pst_coordinate coordinate = it_coordinate->second;
                    delete coordinate;

                    it_coordinate = dplayer->map_coordinate.erase(it_coordinate);
                }

                delete dplayer;
                _map_dplayer.erase(it_dplayer);

                _map_player_connection.erase(it_player_connection);
            }
        }
        break;

        case command_type::program_quit:
        {
            _flag_running = false;
        }
        break;
        default:
            break;
        }

        delete data_pair.first;
    }
}

void process_start_player(uint32_t player_sync_group_index, PlayerType type)
{
    STARTUPINFOA startup_info;
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    PROCESS_INFORMATION process_info;
    ZeroMemory(&process_info, sizeof(process_info));

    std::string cmd;

    if (type == PlayerType::dplayer)
    {
        cmd.append(_application_name_dplayer);
    }
    else
    {
        cmd.append(_application_name_gplayer);
    }
    std::string str = " " + _ip + " " + std::to_string(_port) + " " + std::to_string(player_sync_group_index);
    cmd.append(str);

    BOOL result = FALSE;
    result = CreateProcessA(
        NULL,
        const_cast<LPSTR>(cmd.c_str()),
        NULL, NULL,
        FALSE,
        0,
        NULL, NULL,
        &startup_info,
        &process_info
    );

    if (result == TRUE)
    {
        CloseHandle(process_info.hProcess);
        CloseHandle(process_info.hThread);
    }
}

void get_config()
{
    wchar_t path_w[260] = { 0, };
    GetModuleFileName(nullptr, path_w, 260);
    std::wstring str_path_w = path_w;
    str_path_w = str_path_w.substr(0, str_path_w.find_last_of(L"\\/"));
    std::wstring str_ini_path_w = str_path_w + L"\\PlayerCommandConsole.ini";

    char path_a[260] = { 0, };
    GetModuleFileNameA(nullptr, path_a, 260);
    std::string str_path_a = path_a;
    str_path_a = str_path_a.substr(0, str_path_a.find_last_of("\\/"));
    std::string str_ini_path_a = str_path_a + "\\PlayerCommandConsole.ini";

    char result_a[255];
    wchar_t result_w[255];
    int result_i = 0;

    GetPrivateProfileString(L"PlayerCommandConsole", L"flag_set_logger", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _flag_set_logger = result_i == 0 ? false : true;

    GetPrivateProfileString(L"PlayerCommandConsole", L"log_level", L"6", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _log_level = result_i;

    GetPrivateProfileStringA("PlayerCommandConsole", "application_name_gplayer", "", result_a, 255, str_ini_path_a.c_str());
    _application_name_gplayer = result_a;

    GetPrivateProfileStringA("PlayerCommandConsole", "application_name_dplayer", "", result_a, 255, str_ini_path_a.c_str());
    _application_name_dplayer = result_a;

    GetPrivateProfileStringA("PlayerCommandConsole", "ip", "", result_a, 255, str_ini_path_a.c_str());
    _ip = result_a;

    GetPrivateProfileString(L"PlayerCommandConsole", L"port", L"0", result_w, 255, str_ini_path_w.c_str());
    _port = _ttoi(result_w);
}

void set_logger()
{
    auto max_size = 1024 * 1024;
    auto max_files = 3;

    spdlog::init_thread_pool(8192, 1);
    auto logger = spdlog::rotating_logger_mt<spdlog::async_factory>("player_command_console_logger", "spdlog_logs/player_command_console_log.txt", max_size, max_files);

    spdlog::set_level(spdlog::level::level_enum(_log_level));
    spdlog::flush_every(std::chrono::seconds(3));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l], %H:%M:%S.%e, %v");
}

void start_process_on_user_session(uint32_t player_sync_group_index, PlayerType type)
{
    std::string cmd;
    if (type == PlayerType::dplayer)
    {
        cmd.append(_application_name_dplayer);
    }
    else
    {
        cmd.append(_application_name_gplayer);
    }
    std::string str = " " + _ip + " " + std::to_string(_port) + " " + std::to_string(player_sync_group_index);
    cmd.append(str);

    if (_flag_set_logger)
    {
        std::string str;
        str.append("start_process_on_user_session");
        str.append(", cmd = ");
        str.append(cmd);

        auto logger = spdlog::get("player_command_console_logger");
        logger->info(str.c_str());
    }

    uint32_t winlogon_pid = 0;
    HANDLE handle_process_token = nullptr;
    HANDLE handle_user_token_duplicate = nullptr;

    // obtain the currently active session id; every logged on user in the system has a unique session id
    // 현재 활성 세션 ID를 얻습니다. 시스템에 로그온한 모든 사용자는 고유한 세션 ID를 갖습니다.
    uint32_t session_id = WTSGetActiveConsoleSessionId();
    if (_flag_set_logger)
    {
        std::string str;
        str.append("start_process_on_user_session");
        str.append(", WTSGetActiveConsoleSessionId");
        str.append(", session_id = ");
        str.append(std::to_string(session_id));

        auto logger = spdlog::get("player_command_console_logger");
        logger->debug(str.c_str());
    }

    // obtain the process id of the winlogon process that is running within the currently active session
    // 현재 활성 세션 내에서 실행 중인 winlogon 프로세스의 프로세스 ID를 얻습니다.
    PROCESSENTRY32 pe32{};
    pe32.dwSize = sizeof(pe32);

    // Take a snapshot of all processes in the system.
    // 시스템의 모든 프로세스에 대한 스냅샷을 찍습니다.
    HANDLE handle_process_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (handle_process_snap != INVALID_HANDLE_VALUE)
    {
        // Retrieve information about the first process, and exit if unsuccessful
        // 첫 번째 프로세스에 대한 정보를 검색하고 실패하면 종료합니다.
        BOOL flag_first = Process32First(handle_process_snap, &pe32);
        if (flag_first != FALSE)
        {
            do
            {
                if (0 == _wcsicmp(L"winlogon.exe", pe32.szExeFile))
                {
                    // obtain the process id of the winlogon process that is running within the currently active session
                    // 현재 활성 세션 내에서 실행 중인 winlogon 프로세스의 프로세스 ID를 얻습니다.
                    DWORD temp_session_id;
                    ProcessIdToSessionId(pe32.th32ProcessID, &temp_session_id);
                    if (_flag_set_logger)
                    {
                        std::string str;
                        str.append("start_process_on_user_session");
                        str.append(", ProcessIdToSessionId");
                        str.append(", temp_session_id = ");
                        str.append(std::to_string(temp_session_id));

                        auto logger = spdlog::get("player_command_console_logger");
                        logger->debug(str.c_str());
                    }

                    if (temp_session_id == session_id)
                    {
                        winlogon_pid = pe32.th32ProcessID;

                        if (_flag_set_logger)
                        {
                            std::string str;
                            str.append("start_process_on_user_session");
                            str.append(", Process32First");
                            str.append(", winlogon_pid = ");
                            str.append(std::to_string(winlogon_pid));

                            auto logger = spdlog::get("player_command_console_logger");
                            logger->debug(str.c_str());
                        }

                        break;
                    }
                }
            } while (Process32Next(handle_process_snap, &pe32));
        }

        CloseHandle(handle_process_snap);
    }

    // obtain a handle to the winlogon process
    // winlogon 프로세스에 대한 핸들 얻기
    HANDLE handle_process = nullptr;
    handle_process = OpenProcess(MAXIMUM_ALLOWED, false, winlogon_pid);
    if (_flag_set_logger)
    {
        std::string str;
        str.append("start_process_on_user_session");
        str.append(", OpenProcess");
        str.append(", handle_process = ");
        str.append(std::to_string((uint64_t)handle_process));

        auto logger = spdlog::get("player_command_console_logger");
        logger->debug(str.c_str());
    }

    // obtain a handle to the access token of the winlogon process
    // winlogon 프로세스의 액세스 토큰에 대한 핸들을 얻습니다.
    if (!OpenProcessToken(handle_process, TOKEN_DUPLICATE, &handle_process_token))
    {
        CloseHandle(handle_process);
    }
    if (_flag_set_logger)
    {
        std::string str;
        str.append("start_process_on_user_session");
        str.append(", OpenProcessToken");
        str.append(", handle_process_token = ");
        str.append(std::to_string((uint64_t)handle_process_token));

        auto logger = spdlog::get("player_command_console_logger");
        logger->debug(str.c_str());
    }

    // copy the access token of the winlogon process; the newly created token will be a primary token
    // winlogon 프로세스의 액세스 토큰을 복사합니다. 새로 생성된 토큰은 기본 토큰이 됩니다.
    if (!DuplicateTokenEx(handle_process_token, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &handle_user_token_duplicate))
    {
        CloseHandle(handle_process);
        CloseHandle(handle_process_token);
    }
    if (_flag_set_logger)
    {
        std::string str;
        str.append("start_process_on_user_session");
        str.append(", DuplicateTokenEx");
        str.append(", handle_user_token_duplicate = ");
        str.append(std::to_string((uint64_t)handle_user_token_duplicate));

        auto logger = spdlog::get("player_command_console_logger");
        logger->debug(str.c_str());
    }

    STARTUPINFOA startup_info;
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    PROCESS_INFORMATION process_info;
    ZeroMemory(&process_info, sizeof(process_info));

    // create a new process in the current user's logon session
    // 현재 사용자의 로그온 세션에서 새 프로세스를 만듭니다.
    bool result = CreateProcessAsUserA(
        handle_user_token_duplicate,
        NULL,
        const_cast<LPSTR>(cmd.c_str()),
        NULL,
        NULL,
        FALSE,
        DETACHED_PROCESS,
        NULL,
        NULL,
        &startup_info,
        &process_info
    );
    if (_flag_set_logger)
    {
        std::string str;
        str.append("start_process_on_user_session");
        str.append(", CreateProcessAsUserA");
        str.append(", result = ");
        str.append(std::to_string(result));

        auto logger = spdlog::get("player_command_console_logger");
        logger->debug(str.c_str());
    }

    if (result == TRUE)
    {
        CloseHandle(process_info.hProcess);
        CloseHandle(process_info.hThread);
    }
}
