// WindowsProject.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "GPlayer.h"

#include "StringConverter.h"

#include <map>
#include <vector>
#include <string>
#include <thread>
#include <queue>
#include <mutex>

#include "CppSocketAPI.h"
#include "PacketDefine.h"
#include "ApiFunctionStructures.h"
#pragma comment(lib, "CppSocket.lib")

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"
#pragma comment(lib, "spdlog.lib")

#if _DEBUG
#include <crtdbg.h>
#endif // _DEBUG

#pragma comment(lib, "glib-2.0.lib")
#pragma comment(lib, "gobject-2.0.lib")
#pragma comment(lib, "gstreamer-1.0.lib")
#pragma comment (lib, "gstvideo-1.0.lib")
#include <glib.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

typedef struct st_coordinate
{
    int left;
    int top;
    int width;
    int height;

}*pst_coordinate;


bool _flag_set_logger = false;


uint64_t _count_source = UINT64_MAX;
uint64_t _count_sink = UINT64_MAX;
std::map<uint64_t, std::string> _map_url;
std::map<uint64_t, pst_coordinate> _map_coordinate;
std::map<uint64_t, HWND> _map_handle;

std::map<uint64_t, GstElement*> _map_gst_sink;
std::map<uint64_t, GstElement*> _map_gst_queue;

size_t _current_track = 0;
GMainLoop* _loop = nullptr;
GstElement* _source = nullptr;
GstElement* _pipeline = nullptr;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);


std::thread _thread_glib;

void get_config();
void set_logger();

void on_pad_added(GstElement* src, GstPad* new_pad, GstElement* sink);
void play_next_track();
gboolean bus_call(GstBus* bus, GstMessage* msg, gpointer data);
void gmain_loop_thread();


constexpr uint64_t _sleep_time = 1;

bool _is_running = false;

void* _client = nullptr;
std::thread _thread_client;
bool _flag_thread_client = false;

std::deque<void*> _queue_packet_processing;
std::mutex _mutex_packet_processing;
std::thread _thread_packet_processing;
bool _flag_thread_packet_processing = false;

std::string _ip;
uint16_t _port = UINT16_MAX;
uint32_t _player_sync_group_index = UINT32_MAX;

int _log_level = 6;

std::string _logger_name;

std::string _default_image_url;

bool _flag_file_not_found = false;

void thread_client();

void callback_ptr_client(void* data);

void thread_packet_processing();

int gst_start();

void check_ready_to_playback();

void message_processing_window();

void clear_map_coordinate();

void get_config()
{
    wchar_t path_w[260] = { 0, };
    GetModuleFileName(nullptr, path_w, 260);
    std::wstring str_path_w = path_w;
    str_path_w = str_path_w.substr(0, str_path_w.find_last_of(L"\\/"));
    std::wstring str_ini_path_w = str_path_w + L"\\GPlayer.ini";

    char path_a[260] = { 0, };
    GetModuleFileNameA(nullptr, path_a, 260);
    std::string str_path_a = path_a;
    str_path_a = str_path_a.substr(0, str_path_a.find_last_of("\\/"));
    std::string str_ini_path_a = str_path_a + "\\GPlayer.ini";

    char result_a[255];
    wchar_t result_w[255];
    int result_i = 0;

    GetPrivateProfileString(L"GPlayer", L"flag_set_logger", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _flag_set_logger = result_i == 0 ? false : true;

    GetPrivateProfileString(L"GPlayer", L"log_level", L"6", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _log_level = result_i;

    GetPrivateProfileStringA("GPlayer", "default_image_url", "", result_a, 255, str_ini_path_a.c_str());
    _default_image_url = result_a;
}

void set_logger()
{
    auto max_size = 1024 * 1024;
    auto max_files = 3;

    spdlog::init_thread_pool(8192, 1);

    _logger_name.append("gplayer_logger_");
    _logger_name.append(std::to_string(_player_sync_group_index));

    std::string log_name = "spdlogs/gplayer_log_";
    log_name.append(std::to_string(_player_sync_group_index));
    log_name.append(".txt");

    //auto logger = spdlog::rotating_logger_mt<spdlog::async_factory>(_logger_name.c_str(), log_name.c_str(), max_size, max_files);
    auto logger = spdlog::rotating_logger_mt(_logger_name.c_str(), log_name.c_str(), max_size, max_files);

    spdlog::set_level(spdlog::level::level_enum(_log_level));
    spdlog::flush_every(std::chrono::seconds(3));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l], %H:%M:%S.%e, %v");
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
#if _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    std::wstring cmd(lpCmdLine);
    size_t pos = 0;

    if (cmd.empty() == false)
    {
        if ((pos = cmd.find(L" ")) != std::wstring::npos)
        {
            std::wstring unicode = cmd.substr(0, pos);
            convert_unicode_to_ansi_string(_ip, unicode.c_str(), unicode.size());
            cmd.erase(0, pos + 1);
        }
        else
        {
            std::wstring unicode = cmd;
            convert_unicode_to_ansi_string(_ip, cmd.c_str(), cmd.size());
            cmd.erase(0);
        }
    }

    if (cmd.empty() == false)
    {
        if ((pos = cmd.find(L" ")) != std::wstring::npos)
        {
            std::wstring unicode = cmd.substr(0, pos);
            _port = _wtoi(unicode.c_str());
            cmd.erase(0, pos + 1);
        }
        else
        {
            std::wstring unicode = cmd;
            _port = _wtoi(unicode.c_str());
            cmd.erase(0);
        }
    }

    if (cmd.empty() == false)
    {
        if ((pos = cmd.find(L" ")) != std::wstring::npos)
        {
            std::wstring unicode = cmd.substr(0, pos);
            _player_sync_group_index = _wtoi(unicode.c_str());
            cmd.erase(0, pos + 1);
        }
        else
        {
            std::wstring unicode = cmd;
            _player_sync_group_index = _wtoi(unicode.c_str());
            cmd.erase(0);
        }
    }

    // TODO: 여기에 코드를 입력합니다.
    get_config();

    set_logger();

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GPLAYER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    _is_running = true;

    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_main");
        str.append(", ip = ");
        str.append(_ip.c_str());

        auto logger = spdlog::get(_logger_name.c_str());
        logger->info(str.c_str());
    }

    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_main");
        str.append(", port = ");
        str.append(std::to_string(_port));

        auto logger = spdlog::get(_logger_name.c_str());
        logger->info(str.c_str());
    }

    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_main");
        str.append(", player_sync_group_index = ");
        str.append(std::to_string(_player_sync_group_index));

        auto logger = spdlog::get(_logger_name.c_str());
        logger->info(str.c_str());
    }

    _flag_thread_packet_processing = true;
    _thread_packet_processing = std::thread(thread_packet_processing);

    _flag_thread_client = true;
    _thread_client = std::thread(thread_client);

    // check_ready_to_playback();

    MSG win_msg{};

    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_main");
        str.append(", Start main loop");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    while (_is_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));

        // 기본 메시지 루프입니다:
        while (PeekMessage(&win_msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&win_msg);
            DispatchMessage(&win_msg);

            _is_running &= (win_msg.message != WM_QUIT);
        }
    }

    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_main");
        str.append(", End main loop");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    g_main_loop_quit(_loop);

    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_main");
        str.append(", End g_main_loop_quit");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    /* Free resources */
    gst_element_set_state(_pipeline, GST_STATE_NULL);
    gst_object_unref(_pipeline);

    if (_thread_glib.joinable())
    {
        _thread_glib.join();
    }

    g_main_loop_unref(_loop);

    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_main");
        str.append(", join _thread_client");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    if (_thread_client.joinable())
    {
        _flag_thread_client = false;
        _thread_client.join();
    }

    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_main");
        str.append(", join _thread_packet_processing");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    if (_thread_packet_processing.joinable())
    {
        _flag_thread_packet_processing = false;
        _thread_packet_processing.join();
    }

    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_main");
        str.append(", End thread_main");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    clear_map_coordinate();

    return (int)win_msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    //wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.style = 0;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GPLAYER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    //wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GPLAYER);
    wcex.lpszMenuName = 0;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance()
{
    HWND hWnd = nullptr;
    BOOL result = FALSE;

    for (size_t i = 0; i < _count_sink; i++)
    {
        pst_coordinate coordinate = nullptr;
        int left = 0;
        int top = 0;
        int width = 0;
        int height = 0;

        coordinate = _map_coordinate.find(i)->second;

        left = coordinate->left;
        top = coordinate->top;
        width = coordinate->width;
        height = coordinate->height;

        hWnd = CreateWindowW(szWindowClass, szTitle, WS_POPUP,
            left, top, width, height, nullptr, nullptr, hInst, nullptr);

        if (!hWnd)
        {
            if (_flag_set_logger)
            {
                std::string str;
                str.append("thread_packet_processing");
                str.append(", CreateWindowW Fail");

                auto logger = spdlog::get(_logger_name.c_str());
                logger->warn(str.c_str());
            }
            return FALSE;
        }

        if (_flag_set_logger)
        {
            std::string str;
            str.append("thread_packet_processing");
            str.append(", CreateWindowW Success");
            str.append(", left = ");
            str.append(std::to_string(left));
            str.append(", top = ");
            str.append(std::to_string(top));
            str.append(", width = ");
            str.append(std::to_string(width));
            str.append(", height = ");
            str.append(std::to_string(height));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }

        result = ShowWindow(hWnd, SW_SHOW);
        if (_flag_set_logger)
        {
            std::string str;
            str.append("thread_packet_processing");
            str.append(", ShowWindow");
            str.append(", result = ");
            str.append(std::to_string(result));

            auto logger = spdlog::get(_logger_name.c_str());

            if (result == FALSE)
            {
                str.append(", GetLastError = ");
                str.append(std::to_string(GetLastError()));
                logger->warn(str.c_str());
            }
            else
            {
                logger->debug(str.c_str());
            }
        }

        result = UpdateWindow(hWnd);
        if (_flag_set_logger)
        {
            std::string str;
            str.append("thread_packet_processing");
            str.append(", UpdateWindow");
            str.append(", result = ");
            str.append(std::to_string(result));

            auto logger = spdlog::get(_logger_name.c_str());
            if (result == FALSE)
            {
                str.append(", GetLastError = ");
                str.append(std::to_string(GetLastError()));
                logger->warn(str.c_str());
            }
            else
            {
                logger->debug(str.c_str());
            }
        }

        _map_handle.insert({ i, hWnd });
    }

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)
        {
            DestroyWindow(hWnd);
        }
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Callback function to link the dynamic pads from decodebin
void on_pad_added(GstElement* src, GstPad* new_pad, GstElement* sink)
{
    GstPad* sink_pad = gst_element_get_static_pad(sink, "sink");
    if (gst_pad_is_linked(sink_pad))
    {
        if (_flag_set_logger)
        {
            std::string str;
            str.append("on_pad_added");
            str.append(", gst_pad_is_linked return false");
            str.append(", Pad already linked. Ignoring.");

            auto logger = spdlog::get(_logger_name.c_str());
            logger->warn(str.c_str());
        }
        //g_print("Pad already linked. Ignoring.\n");
        gst_object_unref(sink_pad);
        return;
    }

    GstPadLinkReturn ret = gst_pad_link(new_pad, sink_pad);
    if (ret != GST_PAD_LINK_OK)
    {
        if (_flag_set_logger)
        {
            std::string str;
            str.append("on_pad_added");
            str.append(", gst_pad_link return not GST_PAD_LINK_OK");
            str.append(", Type is '");
            str.append(GST_PAD_NAME(new_pad));
            str.append("' but link failed.");

            auto logger = spdlog::get(_logger_name.c_str());
            logger->warn(str.c_str());
        }
        //g_printerr("Type is '%s' but link failed.\n", GST_PAD_NAME(new_pad));
    }
    else
    {
        if (_flag_set_logger)
        {
            std::string str;
            str.append("on_pad_added");
            str.append(", gst_pad_link return GST_PAD_LINK_OK");
            str.append(", Link succeeded (type '");
            str.append(GST_PAD_NAME(new_pad));
            str.append("').");

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }
        //g_print("Link succeeded (type '%s').\n", GST_PAD_NAME(new_pad));
    }
    gst_object_unref(sink_pad);
}

void play_next_track()
{
    if (_current_track >= _map_url.size())
    {
        //// 반복재생 X
        //g_main_loop_quit(_loop);
        //return;

        // 반복재생 O
        _current_track = 0;
    }

    std::string current_uri = _map_url.find(_current_track)->second;
    g_object_set(G_OBJECT(_source), "location", current_uri.c_str(), NULL);

    gst_element_set_state(_pipeline, GST_STATE_PLAYING);
    _current_track++;
}

gboolean bus_call(GstBus* bus, GstMessage* msg, gpointer data)
{
    switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_ERROR:
    {
        GError* err;
        gchar* debug_info;

        gst_message_parse_error(msg, &err, &debug_info);

        if (_flag_set_logger)
        {
            std::string str;
            str.append("bus_call");
            str.append(", MessageType = GST_MESSAGE_ERROR");
            str.append(", Error received from element ");
            str.append(GST_OBJECT_NAME(msg->src));
            str.append(": ");
            str.append(err->message);
            str.append(", Error Code : ");
            str.append(std::to_string(err->code));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->warn(str.c_str());
        }
        if (_flag_set_logger)
        {
            std::string str;
            str.append("bus_call");
            str.append(", MessageType = GST_MESSAGE_ERROR");
            str.append(", Debugging information: ");
            str.append(debug_info ? debug_info : "none");

            auto logger = spdlog::get(_logger_name.c_str());
            logger->warn(str.c_str());
        }

        // file not found
        if (err->code == 3)
        {
            if (_flag_file_not_found == false)
            {
                _flag_file_not_found = true;

                _count_source = 1;
                _map_url.clear();
                _map_url.insert({ 0, _default_image_url });

                gst_element_set_state(_pipeline, GST_STATE_NULL);
                play_next_track();
            }
            else
            {
                //g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                //g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);

                g_main_loop_quit(_loop);
            }
        }
    }
    break;
    case GST_MESSAGE_EOS:
    {
        if (_flag_set_logger)
        {
            std::string str;
            str.append("bus_call");
            str.append(", MessageType = GST_MESSAGE_EOS");
            str.append(", End-Of-Stream reached.");

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }
        //g_print("End-Of-Stream reached.\n");

        if (_flag_file_not_found == false)
        {
            gst_element_set_state(_pipeline, GST_STATE_NULL);
            play_next_track();
        }
    }
    break;
    default:
    {
        if (_flag_set_logger)
        {
            std::string str;
            str.append("bus_call");
            str.append(", MessageType = default");
            str.append(", Unexpected message received.");
            str.append(", ");
            str.append(GST_MESSAGE_TYPE_NAME(msg));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->trace(str.c_str());
        }
        //g_printerr("Unexpected message received.\n");
    }
    break;
    }

    return TRUE;
}

void gmain_loop_thread()
{
    if (_flag_set_logger)
    {
        std::string str;
        str.append("gmain_loop_thread");
        str.append(", Start g_main_loop_run");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    g_main_loop_run(_loop);

    if (_flag_set_logger)
    {
        std::string str;
        str.append("gmain_loop_thread");
        str.append(", End g_main_loop_run");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }
}

void thread_client()
{
    cppsocket_network_initialize();

    void* client = cppsocket_client_create();

    cppsocket_client_set_callback_data(client, callback_ptr_client);

    if (cppsocket_client_connect(client, _ip.c_str(), _port))
    {
        _client = client;

        if (cppsocket_client_is_connected(client))
        {
            cppsocket_struct_client_send_player_connect data{};
            data.player_sync_group_index = _player_sync_group_index;

            cppsocket_client_send_gplayer_connect(client, data);
        }

        while (_flag_thread_client)
        {
            if (cppsocket_client_is_connected(client))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));

                cppsocket_client_frame(client);
            }
        }
    }
    cppsocket_client_delete(client);

    cppsocket_client_connection_close(client);
    cppsocket_network_shutdown();
}

void callback_ptr_client(void* data)
{
    packet_header* header = (packet_header*)data;

    void* packet = new char[header->size];
    memcpy(packet, data, header->size);

    {
        std::lock_guard<std::mutex> lk(_mutex_packet_processing);
        _queue_packet_processing.push_back((packet));
    }
}

void thread_packet_processing()
{
    while (_flag_thread_packet_processing)
    {
        message_processing_window();

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

        void* data;
        {
            std::lock_guard<std::mutex> lk(_mutex_packet_processing);
            data = _queue_packet_processing.front();
            _queue_packet_processing.pop_front();
        }

        packet_header* header = (packet_header*)data;

        switch (header->cmd)
        {
        case command_type::gplayer_connect_data_url:
        {
            packet_gplayer_connect_data_url_from_server* packet = new packet_gplayer_connect_data_url_from_server();
            memcpy(packet, data, header->size);

            if (_flag_set_logger)
            {
                std::string str;
                str.append("thread_packet_processing");
                str.append(", gplayer_connect_data_url");
                str.append(", url = ");
                str.append(packet->url);

                auto logger = spdlog::get(_logger_name.c_str());
                logger->debug(str.c_str());
            }

            if (_player_sync_group_index == packet->player_sync_group_index)
            {
                if (_count_source == UINT64_MAX)
                {
                    _count_source = packet->player_sync_group_input_count;
                }

                _map_url.insert({ _map_url.size(), packet->url });
            }

            delete packet;

            check_ready_to_playback();
        }
        break;
        case command_type::gplayer_connect_data_rect:
        {
            packet_gplayer_connect_data_rect_from_server* packet = new packet_gplayer_connect_data_rect_from_server();
            memcpy(packet, data, header->size);

            if (_flag_set_logger)
            {
                std::string str;
                str.append("thread_packet_processing");
                str.append(", gplayer_connect_data_rect");
                str.append(", left = ");
                str.append(std::to_string(packet->left));
                str.append(", top = ");
                str.append(std::to_string(packet->top));
                str.append(", width = ");
                str.append(std::to_string(packet->width));
                str.append(", height = ");
                str.append(std::to_string(packet->height));

                auto logger = spdlog::get(_logger_name.c_str());
                logger->debug(str.c_str());
            }

            if (_player_sync_group_index == packet->player_sync_group_index)
            {
                if (_count_sink == UINT64_MAX)
                {
                    _count_sink = packet->player_sync_group_output_count;
                }

                pst_coordinate coordinate = new st_coordinate();
                coordinate->left = packet->left;
                coordinate->top = packet->top;
                coordinate->width = packet->width;
                coordinate->height = packet->height;

                _map_coordinate.insert({ _map_coordinate.size(), coordinate });
            }

            delete packet;

            check_ready_to_playback();
        }
        break;

        case command_type::gplayer_stop:
        {
            packet_gplayer_stop_from_server* packet = new packet_gplayer_stop_from_server();
            memcpy(packet, data, header->size);

            if (_flag_set_logger)
            {
                std::string str;
                str.append("thread_packet_processing");
                str.append(", gplayer_stop");

                auto logger = spdlog::get(_logger_name.c_str());
                logger->info(str.c_str());
            }

            delete packet;

            _is_running = false;

            if (_flag_set_logger)
            {
                std::string str;
                str.append("thread_packet_processing");
                str.append(", gplayer_stop");
                str.append(", _is_running = ");
                str.append(std::to_string(_is_running));

                auto logger = spdlog::get(_logger_name.c_str());
                logger->info(str.c_str());
            }
        }
        break;

        default:
            break;
        }

        delete data;
    }
}

int gst_start()
{
    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_packet_processing");
        str.append(", gst_start");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance())
    {
        return FALSE;
    }

    /* Initialize GStreamer */
    gst_init(NULL, NULL);
    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_packet_processing");
        str.append(", gst_start");
        str.append(", gst_init()");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    _loop = g_main_loop_new(NULL, FALSE);
    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_packet_processing");
        str.append(", gst_start");
        str.append(", g_main_loop_new()");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    /* Create GStreamer elements */
    _pipeline = gst_pipeline_new("video-player");
    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_packet_processing");
        str.append(", gst_start");
        str.append(", gst_pipeline_new()");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    // 항상 사용함
    _source = gst_element_factory_make("filesrc", "file-source");
    GstElement* decoder = gst_element_factory_make("decodebin", "decoder");
    GstElement* videoconvert = gst_element_factory_make("videoconvert", "video-converter");

    // 항상 사용함
    // 동기화할 개수 만큼 사용함
    for (size_t i = 0; i < _count_sink; i++)
    {
        std::string sink_name = "video-output";
        sink_name.append(std::to_string(i));

        GstElement* sink = gst_element_factory_make("d3dvideosink", sink_name.c_str());

        _map_gst_sink.insert({ i, sink });
    }

    // 동기화 재생할 때만 사용함
    // 동기화할 개수 만큼 사용함
    GstElement* tee = gst_element_factory_make("tee", "tee");

    for (size_t i = 0; i < _count_sink; i++)
    {
        std::string queue_name = "queue";
        queue_name.append(std::to_string(i));

        GstElement* queue = gst_element_factory_make("queue", queue_name.c_str());

        _map_gst_queue.insert({ i, queue });
    }


    if (!_pipeline || !_source || !decoder || !videoconvert || !tee)
    {
        if (_flag_set_logger)
        {
            std::string str;
            str.append("gst_start");
            str.append(", Not all elements could be created.");

            auto logger = spdlog::get(_logger_name.c_str());
            logger->warn(str.c_str());
        }
        //g_printerr("Not all elements could be created.\n");
        return -1;
    }

    for (size_t i = 0; i < _count_sink; i++)
    {
        if (!_map_gst_sink.find(i)->second || !_map_gst_queue.find(i)->second)
        {
            if (_flag_set_logger)
            {
                std::string str;
                str.append("gst_start");
                str.append(", Not all elements could be created.");

                auto logger = spdlog::get(_logger_name.c_str());
                logger->warn(str.c_str());
            }
            //g_printerr("Not all elements could be created.\n");
            return -1;
        }
    }
    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_packet_processing");
        str.append(", gst_start");
        str.append(", elements created");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    // 영상 종횡비와 window 종횡비가 다를때 영상의 빈곳에 검은색 배경이 생기는 것을 방지함.
    for (size_t i = 0; i < _count_sink; i++)
    {
        g_object_set(G_OBJECT(_map_gst_sink.find(i)->second), "force-aspect-ratio", false, NULL);
    }

    /* Build the pipeline */
    gst_bin_add_many(GST_BIN(_pipeline), _source, decoder, videoconvert, tee, NULL);

    for (size_t i = 0; i < _count_sink; i++)
    {
        gst_bin_add(GST_BIN(_pipeline), _map_gst_sink.find(i)->second);
        gst_bin_add(GST_BIN(_pipeline), _map_gst_queue.find(i)->second);
    }


    if (!gst_element_link(_source, decoder))
    {
        if (_flag_set_logger)
        {
            std::string str;
            str.append("gst_start");
            str.append(", gst_element_link return false");
            str.append(", Source and decoder could not be linked.");

            auto logger = spdlog::get(_logger_name.c_str());
            logger->warn(str.c_str());
        }
        //g_printerr("Source and decoder could not be linked.\n");
        gst_object_unref(_pipeline);
        return -1;
    }

    // Connect the decoder's pad-added signal to the handler function
    // 동적 패드 연결을 위한 시그널을 이용한 콜백 설정
    g_signal_connect(decoder, "pad-added", G_CALLBACK(on_pad_added), videoconvert);

    //// Link the remaining elements

    if (!gst_element_link(videoconvert, tee))
    {
        if (_flag_set_logger)
        {
            std::string str;
            str.append("gst_start");
            str.append(", gst_element_link return false");
            str.append(", Converter and tee could not be linked.");

            auto logger = spdlog::get(_logger_name.c_str());
            logger->warn(str.c_str());
        }
        g_printerr("Converter and tee could not be linked.\n");
        gst_object_unref(_pipeline);
        return -1;
    }

    for (size_t i = 0; i < _count_sink; i++)
    {
        if (!gst_element_link(_map_gst_queue.find(i)->second, _map_gst_sink.find(i)->second))
        {
            if (_flag_set_logger)
            {
                std::string str;
                str.append("gst_start");
                str.append(", gst_element_link return false");
                str.append(", Queue");
                str.append(std::to_string(i));
                str.append(" and sink");
                str.append(std::to_string(i));
                str.append(" could not be linked.");

                auto logger = spdlog::get(_logger_name.c_str());
                logger->warn(str.c_str());
            }

            //g_printerr("Queue and sink could not be linked.\n");
            gst_object_unref(_pipeline);
            return -1;
        }
    }

    for (size_t i = 0; i < _count_sink; i++)
    {
        if (!gst_element_link_pads(tee, "src_%u", _map_gst_queue.find(i)->second, "sink"))
        {
            if (_flag_set_logger)
            {
                std::string str;
                str.append("gst_start");
                str.append(", gst_element_link_pads return false");
                str.append(", Tee and queue");
                str.append(std::to_string(i));
                str.append(" could not be linked.");

                auto logger = spdlog::get(_logger_name.c_str());
                logger->warn(str.c_str());
            }

            //g_printerr("Tee and queue could not be linked.\n");
            gst_object_unref(_pipeline);
            return -1;
        }
    }

    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_packet_processing");
        str.append(", gst_start");
        str.append(", elements linked");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    for (size_t i = 0; i < _count_sink; i++)
    {
        HWND hWnd = _map_handle.find(i)->second;

        GstElement* sink = _map_gst_sink.find(i)->second;

        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(sink), (guintptr)hWnd);
    }

    /* Wait until error or EOS */
    GstBus* bus = gst_element_get_bus(_pipeline);
    gst_bus_add_watch(bus, bus_call, NULL);
    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_packet_processing");
        str.append(", gst_start");
        str.append(", gst_bus_add_watch");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }
    gst_object_unref(bus);


    play_next_track();


    _thread_glib = std::thread(gmain_loop_thread);

    return true;
}

void check_ready_to_playback()
{
    if (_flag_set_logger)
    {
        std::string str;
        str.append("thread_packet_processing");
        str.append(", check_ready_to_playback");
        str.append(", _count_source = ");
        str.append(std::to_string(_count_source));
        str.append(", _count_sink = ");
        str.append(std::to_string(_count_sink));
        str.append(", _map_url.size() = ");
        str.append(std::to_string(_map_url.size()));
        str.append(", _map_coordinate.size() = ");
        str.append(std::to_string(_map_coordinate.size()));

        auto logger = spdlog::get(_logger_name.c_str());
        logger->info(str.c_str());
    }

    if (!(_count_source != UINT64_MAX && _count_sink != UINT64_MAX))
    {
        return;
    }

    if (_map_url.size() != _count_source)
    {
        return;
    }

    if (_map_coordinate.size() != _count_sink)
    {
        return;
    }

    gst_start();
}

void message_processing_window()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        _is_running &= (msg.message != WM_QUIT);
    }
}

void clear_map_coordinate()
{
    for (auto it_coordinate = _map_coordinate.begin(); it_coordinate != _map_coordinate.end();)
    {
        pst_coordinate coordinate = it_coordinate->second;

        delete coordinate;

        it_coordinate = _map_coordinate.erase(it_coordinate);
    }
}
