// WPlayer.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "WPlayer.h"

// --------------------------------

#include "CommonHeaders.h"
#include "GraphicsHeaders.h"

#include <limits>
#include <queue>

#if _DEBUG
#include <crtdbg.h>
#endif // _DEBUG

#if _DEBUG
#include "dxgidebug.h"
#endif // _DEBUG

// --------------------------------

#include "CppSocketAPI.h"
#include "PacketDefine.h"
#include "ApiFunctionStructures.h"
#pragma comment(lib, "CppSocket.lib")

// --------------------------------

#include "CppFFmpegWrapperAPI.h"
#include "CppFFmpegWrapperCallbackStruct.h"
#pragma comment(lib, "CppFFmpegWrapper.lib")

// --------------------------------

#include "nvapi.h"
#pragma comment(lib, "nvapi64.lib")

// --------------------------------

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

bool _is_running = true;
std::wstring _asset_path;
constexpr uint16_t _sleep_time_main_loop = 1;
constexpr uint32_t frame_buffer_count = 3;

constexpr uint32_t rtv_descriptor_count = 4096;
constexpr uint32_t srv_descriptor_count = 4096;

std::string _ip;
uint16_t _port;
bool _use_nvapi = false;
bool _block_swap_group_present = false;
// 텍스처 준비 개수
uint64_t _prepare_texture_count = 0;

// 윈도우 생성 개수
uint64_t _create_window_count = 0;
std::map<uint64_t, int> _map_window_rect_left;
std::map<uint64_t, int> _map_window_rect_top;
std::map<uint64_t, int> _map_window_rect_width;
std::map<uint64_t, int> _map_window_rect_height;

// scene 생성 개수
uint64_t _prepare_scene_count = 0;
std::map<uint64_t, std::string> _map_prepare_scene_url;
std::map<uint64_t, int> _map_prepare_scene_rect_left;
std::map<uint64_t, int> _map_prepare_scene_rect_top;
std::map<uint64_t, int> _map_prepare_scene_rect_width;
std::map<uint64_t, int> _map_prepare_scene_rect_height;

uint64_t _scene_texture_width = 0;
uint64_t _scene_texture_height = 0;

// --------------------------------

std::deque<std::pair<void*, void*>> _queue_packet_processing;
constexpr uint32_t _sleep_time_packet_processing = 10;
std::thread _thread_packet_processing;
bool _flag_packet_processing = true;
std::mutex _mutex_packet_processing;

std::thread _thread_tcp_server;
bool _flag_tcp_server = true;
void* _tcp_server = nullptr;

void thread_packet_processing();
void thread_tcp_server();
void callback_data_connection_server(void* data, void* connection);

// --------------------------------

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 uv;
};

typedef struct st_adapter
{
    IDXGIAdapter1* adapter = nullptr;
    DXGI_ADAPTER_DESC1 adapter_desc{};

    uint64_t adapter_index = UINT64_MAX;

}*pst_adapter;

HANDLE _event_adapter_to_window = nullptr;

std::mutex* _mutex_window_to_adapter = nullptr;
std::condition_variable* _condition_variable_window_to_adapter = nullptr;
bool _flag_window_to_adapter = false;

typedef struct st_output
{
    IDXGIOutput* output = nullptr;
    DXGI_OUTPUT_DESC output_desc{};

    uint64_t output_index = UINT64_MAX;
    uint64_t device_index = UINT64_MAX;

}*pst_output;

typedef struct st_window
{
    HWND handle = nullptr;
    RECT rect{};

    uint64_t window_index = UINT64_MAX;
    uint64_t output_index = UINT64_MAX;
    uint64_t device_index = UINT64_MAX;

    bool flag_thread_window = true;
    HANDLE event_window_to_adapter = nullptr;

    std::mutex* mutex_adapter_to_window = nullptr;
    std::condition_variable* condition_variable_adapter_to_window = nullptr;
    bool flag_adapter_to_window = false;

}*pst_window;

typedef struct st_device
{
    ID3D12Device* device = nullptr;

    uint64_t adapter_index = UINT64_MAX;
    uint64_t device_index = UINT64_MAX;

}*pst_device;

typedef struct st_command_queue
{
    ID3D12CommandQueue* command_queue = nullptr;

    uint64_t device_index = UINT64_MAX;
    uint64_t command_queue_index = UINT64_MAX;

}*pst_command_queue;

typedef struct st_swap_chain
{
    IDXGISwapChain1* swap_chain = nullptr;

    uint64_t swap_chain_index = UINT64_MAX;
    uint64_t device_index = UINT64_MAX;
    uint64_t window_index = UINT64_MAX;

}*pst_swap_chain;

typedef struct st_rtv_heap
{
    ID3D12DescriptorHeap* rtv_heap = nullptr;
    uint32_t rtv_descriptor_size = UINT32_MAX;

    uint64_t device_index = UINT64_MAX;
    uint64_t rtv_heap_index = UINT64_MAX;

}*pst_rtv_heap;

typedef struct st_rtv
{
    std::vector<ID3D12Resource*> vector_rtv;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> vector_rtv_handle;

    uint64_t device_index = UINT64_MAX;
    uint64_t swap_chain_index = UINT64_MAX;
    uint64_t rtv_index = UINT64_MAX;

}*pst_rtv;

typedef struct st_srv_heap
{
    ID3D12DescriptorHeap* srv_heap = nullptr;
    uint32_t srv_descriptor_size = UINT32_MAX;

    uint64_t device_index = UINT64_MAX;
    uint64_t srv_heap_index = UINT64_MAX;

}*pst_srv_heap;

typedef struct st_command_allocator
{
    std::vector<ID3D12CommandAllocator*> vector_command_allocator;

    uint64_t device_index = UINT64_MAX;
    uint64_t command_allocator_index = UINT64_MAX;

}*pst_command_allocator;

typedef struct st_root_signature
{
    ID3D12RootSignature* root_sig = nullptr;

    uint64_t device_index = UINT64_MAX;
    uint64_t root_sig_index = UINT64_MAX;

}*pst_root_signature;

typedef struct st_pipeline_state_object
{
    ID3D12PipelineState* pso = nullptr;

    uint64_t device_index = UINT64_MAX;
    uint64_t pso_index = UINT64_MAX;

}st_pso, * pst_pso;

typedef struct st_command_list
{
    ID3D12GraphicsCommandList* command_list = nullptr;

    uint64_t device_index = UINT64_MAX;
    uint64_t command_list_index = UINT64_MAX;

}*pst_command_list;

typedef struct st_fence
{
    ID3D12Fence* fence = nullptr;
    HANDLE fence_event = nullptr;

    uint64_t fence_value = 0;

    uint64_t device_index = UINT64_MAX;
    uint64_t fence_index = UINT64_MAX;

}*pst_fence;

typedef struct st_viewport
{
    D3D12_VIEWPORT viewport{};
    D3D12_RECT scissor_rect{};

    uint64_t device_index = UINT64_MAX;
    uint64_t window_index = UINT64_MAX;
    uint64_t viewport_index = UINT64_MAX;

}*pst_viewport;

typedef struct st_scene
{
    void* instance = nullptr;
    std::vector<AVFrame*> vector_frame;

    uint64_t scene_index = UINT64_MAX;
    uint64_t device_index = UINT64_MAX;
    uint64_t output_index = UINT64_MAX;

}*pst_scene;

typedef struct st_vertex_buffer
{
    ID3D12Resource* vertex_buffer = nullptr;

    uint64_t vertex_buffer_index = UINT64_MAX;

}*pst_vertex_buffer;

typedef struct st_vertex_buffer_512
{
    std::vector<ID3D12Resource*> vector_vertex_buffer;

    uint64_t vertex_buffer_index = UINT64_MAX;

}*pst_vertex_buffer_512;

typedef struct st_vertex_upload_buffer
{
    ID3D12Resource* vertex_upload_buffer = nullptr;

    uint64_t vertex_upload_buffer_index = UINT64_MAX;

}*pst_vertex_upload_buffer;

typedef struct st_vertex_upload_buffer_512
{
    std::vector<ID3D12Resource*> vector_vertex_upload_buffer;

    uint64_t vertex_upload_buffer_index = UINT64_MAX;

}*pst_vertex_upload_buffer_512;

typedef struct st_vertex_buffer_view
{
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;

    uint64_t vertex_buffer_view_index = UINT64_MAX;

}*pst_vertex_buffer_view;

typedef struct st_vertex_buffer_view_512
{
    std::vector<D3D12_VERTEX_BUFFER_VIEW> vector_vertex_buffer_view;

    uint64_t vertex_buffer_view_index = UINT64_MAX;

}*pst_vertex_buffer_view_512;

typedef struct st_index_buffer
{
    ID3D12Resource* index_buffer = nullptr;

    uint64_t index_buffer_index = UINT64_MAX;

}*pst_index_buffer;

typedef struct st_index_upload_buffer
{
    ID3D12Resource* index_upload_buffer = nullptr;

    uint64_t index_upload_buffer_index = UINT64_MAX;

}*pst_index_upload_buffer;

typedef struct st_index_buffer_view
{
    D3D12_INDEX_BUFFER_VIEW index_buffer_view;

    uint64_t index_buffer_view_index = UINT64_MAX;

}*pst_index_buffer_view;

typedef struct st_texture
{
    std::vector<ID3D12Resource*> vector_texture;

    uint64_t device_index = UINT64_MAX;

}*pst_texture;

typedef struct st_srv_handle
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> vector_handle_cpu;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> vector_handle_gpu;

    uint64_t device_index = UINT64_MAX;

}*pst_srv_handle;

IDXGIFactory2* _factory = nullptr;

std::map<uint64_t, pst_adapter> _map_adapter;
std::map<uint64_t, pst_output> _map_output;
std::map<uint64_t, pst_window> _map_window;
std::map<uint64_t, pst_device> _map_device;
std::map<uint64_t, pst_command_queue> _map_command_queue;
std::map<uint64_t, pst_swap_chain> _map_swap_chain;
std::map<uint64_t, pst_rtv_heap> _map_rtv_heap;
std::map<uint64_t, pst_rtv> _map_rtv;
std::map<uint64_t, pst_srv_heap> _map_srv_heap;
std::map<uint64_t, pst_command_allocator> _map_command_allocator;
std::map<uint64_t, pst_root_signature> _map_root_signature;
std::map<uint64_t, pst_pso> _map_pso;
std::map<uint64_t, pst_command_list> _map_command_list;
std::map<uint64_t, pst_fence> _map_fence;
std::map<uint64_t, pst_viewport> _map_viewport;
std::map<uint64_t, pst_scene> _map_scene;
std::map<uint64_t, pst_vertex_buffer> _map_vertex_buffer;
std::map<uint64_t, pst_vertex_upload_buffer> _map_vertex_upload_buffer;
std::map<uint64_t, pst_vertex_buffer_view> _map_vertex_buffer_view;
std::map<uint64_t, pst_vertex_buffer_512> _map_vertex_buffer_512;
std::map<uint64_t, pst_vertex_upload_buffer_512> _map_vertex_upload_buffer_512;
std::map<uint64_t, pst_vertex_buffer_view_512> _map_vertex_buffer_view_512;
std::map<uint64_t, pst_index_buffer> _map_index_buffer;
std::map<uint64_t, pst_index_upload_buffer> _map_index_upload_buffer;
std::map<uint64_t, pst_index_buffer_view> _map_index_buffer_view;
std::map<uint64_t, pst_texture> _map_texture_y;
std::map<uint64_t, pst_texture> _map_texture_u;
std::map<uint64_t, pst_texture> _map_texture_v;
std::map<uint64_t, pst_texture> _map_upload_texture_y;
std::map<uint64_t, pst_texture> _map_upload_texture_u;
std::map<uint64_t, pst_texture> _map_upload_texture_v;
std::map<uint64_t, pst_srv_handle> _map_srv_handle_y;
std::map<uint64_t, pst_srv_handle> _map_srv_handle_u;
std::map<uint64_t, pst_srv_handle> _map_srv_handle_v;

std::deque<uint64_t> _free_scene_queue;
uint64_t _next_scene_index = 0;


void create_factory();
void enum_adapters();
void enum_outputs();
void create_window(WCHAR* window_class, WCHAR* title, HINSTANCE instance, RECT rect, void* data, HWND& handle);
void create_windows();
void delete_adapter_has_none_window();
void create_devices();
void create_command_queues();
void create_swap_chains();
void create_rtv_heaps();
void create_rtvs();
void create_srv_heaps();
void create_command_allocators();
void create_root_signatures();
void create_psos();
void create_command_lists();
void create_fences();
void create_viewports();
void create_scenes();
void create_scene(char* url, RECT rect);
void callback_ffmpeg_wrapper_ptr(void* param);
void create_vertex_buffer_512(pst_device data_device);
void create_index_buffer(pst_device data_device);
void create_textures(uint64_t width, uint64_t height);
void prepare_frames();
void prepare_texture(pst_device data_device);


void delete_factory();
void delete_adapters();
void delete_outputs();
void delete_windows();
void delete_devices();
void delete_command_queues();
void delete_swap_chains();
void delete_rtv_heaps();
void delete_rtvs();
void delete_srv_heaps();
void delete_command_allocators();
void delete_root_signatures();
void delete_psos();
void delete_command_lists();
void delete_fences();
void delete_viewports();
void delete_scenes();
void delete_index_buffers();
void delete_vertex_buffers_512();
void delete_textures();
void delete_frames();



void initialize_swap_lock(ID3D12Device* device, IDXGISwapChain1* swapchain);
void initialize_swap_locks();
void delete_swap_lock(ID3D12Device* device, IDXGISwapChain1* swap_chain);
void delete_swap_locks();

// --------------------------------

enum class WaitType : uint32_t
{
    none = 0,
    adapter_to_window = 1,
    window_to_adapter = 2,
};

enum class ObjectType : uint32_t
{
    none = 0,
    event_object = 1,
    condition_variable_object = 2,
    mutex_object = 3,
    flag_object = 4,
};

typedef struct st_input_object
{
    WaitType wait_type = WaitType::none;
    ObjectType object_type = ObjectType::none;

    void* object = nullptr;

}*pst_input_object;

std::thread _thread_vector_input;
bool _flag_vector_input = true;
std::vector<st_input_object> _vector_input_object;
std::mutex _mutex_input_object;

void thread_vector_input();
void vector_input(st_input_object object);

std::vector<HANDLE> _vector_event_adapter_to_window;
std::mutex _mutex_vector_event_adapter_to_window;
std::vector<std::condition_variable*> _vector_condition_variable_adapter_to_window;
std::mutex _mutex_vector_condition_variable_adapter_to_window;
std::vector<std::mutex*> _vector_mutex_adapter_to_window;
std::mutex _mutex_vector_mutex_adapter_to_window;
std::vector<bool*> _vector_flag_adapter_to_window;
std::mutex _mutex_vector_flag_adapter_to_window;

std::vector<HANDLE> _vector_event_window_to_adapter;
std::mutex _mutex_vector_event_window_to_adapter;
std::vector<std::condition_variable*> _vector_condition_variable_window_to_adapter;
std::mutex _mutex_vector_condition_variable_window_to_adapter;
std::vector<std::mutex*> _vector_mutex_window_to_adapter;
std::mutex _mutex_vector_mutex_window_to_adapter;
std::vector<bool*> _vector_flag_window_to_adapter;
std::mutex _mutex_vector_flag_window_to_adapter;


std::thread _thread_wait_for_multiple_objects_adapter_to_window;
bool _flag_wait_for_multiple_objects_adapter_to_window = true;

std::thread _thread_wait_for_multiple_objects_window_to_adapter;
bool _flag_wait_for_multiple_objects_window_to_adapter = true;

void thread_wait_for_multiple_objects(WaitType wait_type, bool* flag_thread);

std::map<uint64_t, std::thread*> _map_thread_window;
std::thread _thread_adapter;
bool _flag_thread_adapter = true;

void thread_window(pst_window data_window);
void thread_adapter();

// --------------------------------

bool _nvapi_initialized = false;
NvAPI_Status _nvapi_status = NVAPI_OK;
NvU32 _swap_group = 1;
NvU32 _swap_barrier = 1;

void get_asset_path(wchar_t* path, u32 path_size);
std::wstring get_asset_full_path(LPCWSTR asset_name);
void config_setting();

#if _DEBUG
void d3d_memory_check();
#endif

// --------------------------------

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
    while (_flag_packet_processing)
    {
        //message_processing_window();

        bool tcp_processing_command_is_empty = false;
        {
            std::lock_guard<std::mutex> lk(_mutex_packet_processing);
            tcp_processing_command_is_empty = _queue_packet_processing.empty();
        }

        if (tcp_processing_command_is_empty)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_packet_processing));
            continue;
        }

        // first : packet_data, second : connection
        std::pair<void*, void*> data_pair;
        {
            std::lock_guard<std::mutex> lk(_mutex_packet_processing);
            data_pair = _queue_packet_processing.front();
            _queue_packet_processing.pop_front();
        }

        packet_header* header = (packet_header*)data_pair.first;

        switch (header->cmd)
        {
        case command_type::program_quit:
        {
            _is_running = false;
        }
        break;
        default:
            break;
        }

        delete data_pair.first;
    }
}

void thread_tcp_server()
{
    cppsocket_network_initialize();

    _tcp_server = cppsocket_server_create();

    cppsocket_server_set_callback_data_connection(_tcp_server, callback_data_connection_server);

    if (cppsocket_server_initialize(_tcp_server, _ip.c_str(), _port))
    {
        while (_flag_tcp_server)
        {
            cppsocket_server_frame(_tcp_server);
        }
    }

    cppsocket_server_delete(_tcp_server);

    cppsocket_network_shutdown();
}


void get_asset_path(wchar_t* path, u32 path_size)
{
    DWORD size = GetModuleFileName(nullptr, path, path_size);

    wchar_t* last_slash = wcsrchr(path, L'\\');
    if (last_slash)
    {
        *(last_slash + 1) = L'\0';
    }
}

std::wstring get_asset_full_path(LPCWSTR asset_name)
{
    return _asset_path + asset_name;
}

void config_setting()
{
    wchar_t path_w[260] = { 0, };
    GetModuleFileName(nullptr, path_w, 260);
    std::wstring str_path_w = path_w;
    str_path_w = str_path_w.substr(0, str_path_w.find_last_of(L"\\/"));
    std::wstring str_ini_path_w = str_path_w + L"\\WPlayer_Test_IDC512.ini";

    char path_a[260] = { 0, };
    GetModuleFileNameA(nullptr, path_a, 260);
    std::string str_path_a = path_a;
    str_path_a = str_path_a.substr(0, str_path_a.find_last_of("\\/"));
    std::string str_ini_path_a = str_path_a + "\\WPlayer_Test_IDC512.ini";

    char result_a[255];
    wchar_t result_w[255];
    int result_i = 0;

    GetPrivateProfileStringA("WPlayer", "IP", "", result_a, 255, str_ini_path_a.c_str());
    _ip = result_a;

    GetPrivateProfileString(L"WPlayer", L"PORT", L"0", result_w, 255, str_ini_path_w.c_str());
    _port = _ttoi(result_w);

    GetPrivateProfileString(L"WPlayer", L"use_nvapi", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _use_nvapi = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"block_swap_group_present", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _block_swap_group_present = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"prepare_texture_count", L"30", result_w, 255, str_ini_path_w.c_str());
    _prepare_texture_count = _ttoi(result_w);

    GetPrivateProfileString(L"WPlayer", L"create_window_count", L"0", result_w, 255, str_ini_path_w.c_str());
    _create_window_count = _ttoi(result_w);

    for (size_t i = 0; i < _create_window_count; i++)
    {
        int left;
        int top;
        int width;
        int height;

        std::wstring option_string_left = L"window_rect_left";
        std::wstring option_string_top = L"window_rect_top";
        std::wstring option_string_width = L"window_rect_width";
        std::wstring option_string_height = L"window_rect_height";
        
        option_string_left.append(std::to_wstring(i));
        option_string_top.append(std::to_wstring(i));
        option_string_width.append(std::to_wstring(i));
        option_string_height.append(std::to_wstring(i));

        GetPrivateProfileString(L"WPlayer", option_string_left.c_str(), L"0", result_w, 255, str_ini_path_w.c_str());
        left = _ttoi(result_w);
        GetPrivateProfileString(L"WPlayer", option_string_top.c_str(), L"0", result_w, 255, str_ini_path_w.c_str());
        top = _ttoi(result_w);
        GetPrivateProfileString(L"WPlayer", option_string_width.c_str(), L"0", result_w, 255, str_ini_path_w.c_str());
        width = _ttoi(result_w);
        GetPrivateProfileString(L"WPlayer", option_string_height.c_str(), L"0", result_w, 255, str_ini_path_w.c_str());
        height = _ttoi(result_w);

        _map_window_rect_left.insert({ i, left });
        _map_window_rect_top.insert({ i, top });
        _map_window_rect_width.insert({ i, width });
        _map_window_rect_height.insert({ i, height });
    }

    GetPrivateProfileString(L"WPlayer", L"prepare_scene_count", L"0", result_w, 255, str_ini_path_w.c_str());
    _prepare_scene_count = _ttoi(result_w);

    for (size_t i = 0; i < _prepare_scene_count; i++)
    {
        std::string option_string_url = "prepare_scene_url";

        option_string_url.append(std::to_string(i));

        GetPrivateProfileStringA("WPlayer", option_string_url.c_str(), "", result_a, 255, str_ini_path_a.c_str());
        _map_prepare_scene_url.insert({ i, result_a });

        int left;
        int top;
        int width;
        int height;

        std::wstring option_string_left = L"prepare_scene_rect_left";
        std::wstring option_string_top = L"prepare_scene_rect_top";
        std::wstring option_string_width = L"prepare_scene_rect_width";
        std::wstring option_string_height = L"prepare_scene_rect_height";

        option_string_left.append(std::to_wstring(i));
        option_string_top.append(std::to_wstring(i));
        option_string_width.append(std::to_wstring(i));
        option_string_height.append(std::to_wstring(i));

        GetPrivateProfileString(L"WPlayer", option_string_left.c_str(), L"0", result_w, 255, str_ini_path_w.c_str());
        left = _ttoi(result_w);
        GetPrivateProfileString(L"WPlayer", option_string_top.c_str(), L"0", result_w, 255, str_ini_path_w.c_str());
        top = _ttoi(result_w);
        GetPrivateProfileString(L"WPlayer", option_string_width.c_str(), L"0", result_w, 255, str_ini_path_w.c_str());
        width = _ttoi(result_w);
        GetPrivateProfileString(L"WPlayer", option_string_height.c_str(), L"0", result_w, 255, str_ini_path_w.c_str());
        height = _ttoi(result_w);

        _map_prepare_scene_rect_left.insert({ i, left });
        _map_prepare_scene_rect_top.insert({ i, top });
        _map_prepare_scene_rect_width.insert({ i, width });
        _map_prepare_scene_rect_height.insert({ i, height });
    }

    GetPrivateProfileString(L"WPlayer", L"scene_texture_width", L"0", result_w, 255, str_ini_path_w.c_str());
    _scene_texture_width = _ttoi(result_w);

    GetPrivateProfileString(L"WPlayer", L"scene_texture_height", L"0", result_w, 255, str_ini_path_w.c_str());
    _scene_texture_height = _ttoi(result_w);
}


#if _DEBUG
void d3d_memory_check()
{
    HMODULE dxgidebugdll = GetModuleHandleW(L"dxgidebug.dll");
    decltype(&DXGIGetDebugInterface) GetDebugInterface = reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(dxgidebugdll, "DXGIGetDebugInterface"));

    if (GetDebugInterface == nullptr)
    {
        return;
    }

    IDXGIDebug* debug;

    GetDebugInterface(IID_PPV_ARGS(&debug));

    OutputDebugString(L"---------------- Report Live Objects ----------------\n");
    debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_DETAIL);
    OutputDebugString(L"---------------- Report Live Objects ----------------\n");

    debug->Release();
}
#endif // _DEBUG

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = 0;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WPLAYER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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


void create_factory()
{
    HRESULT hr = S_OK;

    uint32_t dxgi_factory_flags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug1> debug_controller;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller))))
        {
            debug_controller->EnableDebugLayer();
#if 1
            debug_controller->SetEnableGPUBasedValidation(true);
#endif // 1

            // Enable additional debug layers.
            dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&_factory));
}

void enum_adapters()
{
    if (!_factory)
    {
        return;
    }

    HRESULT hr = S_OK;

    IDXGIAdapter1* adapter = nullptr;
    DXGI_ADAPTER_DESC1 adapter_desc{};

    for (size_t i = 0; ; i++)
    {
        hr = _factory->EnumAdapters1(i, &adapter);
        if (hr != S_OK)
        {
            break;
        }

        adapter->GetDesc1(&adapter_desc);

        if (adapter_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            adapter->Release();
            continue;
        }

        hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr);
        if (hr == S_FALSE)
        {
            pst_adapter data_adapter = new st_adapter();
            data_adapter->adapter = adapter;
            data_adapter->adapter_desc = adapter_desc;
            data_adapter->adapter_index = i;
            _map_adapter.insert({ i, data_adapter });
        }
        else
        {
            adapter->Release();
        }
    }

    _event_adapter_to_window = CreateEvent(NULL, FALSE, FALSE, NULL);
    _condition_variable_window_to_adapter = new std::condition_variable();
    _mutex_window_to_adapter = new std::mutex();

    st_input_object event_adapter_to_window{};
    event_adapter_to_window.object = _event_adapter_to_window;
    event_adapter_to_window.object_type = ObjectType::event_object;
    event_adapter_to_window.wait_type = WaitType::adapter_to_window;

    vector_input(event_adapter_to_window);


    st_input_object condition_variable_window_to_adapter{};
    condition_variable_window_to_adapter.object = _condition_variable_window_to_adapter;
    condition_variable_window_to_adapter.object_type = ObjectType::condition_variable_object;
    condition_variable_window_to_adapter.wait_type = WaitType::window_to_adapter;

    st_input_object mutex_window_to_adapter{};
    mutex_window_to_adapter.object = _mutex_window_to_adapter;
    mutex_window_to_adapter.object_type = ObjectType::mutex_object;
    mutex_window_to_adapter.wait_type = WaitType::window_to_adapter;

    st_input_object flag_window_to_adapter{};
    flag_window_to_adapter.object = &_flag_window_to_adapter;
    flag_window_to_adapter.object_type = ObjectType::flag_object;
    flag_window_to_adapter.wait_type = WaitType::window_to_adapter;

    vector_input(condition_variable_window_to_adapter);
    vector_input(mutex_window_to_adapter);
    vector_input(flag_window_to_adapter);
}

void enum_outputs()
{
    HRESULT hr = S_OK;

    IDXGIOutput* output = nullptr;

    uint64_t output_index = 0;

    for (auto it_adapter = _map_adapter.begin(); it_adapter != _map_adapter.end();)
    {
        pst_adapter data_adapter = it_adapter->second;

        uint64_t output_count = 0;

        for (size_t i = 0; ; i++)
        {
            hr = data_adapter->adapter->EnumOutputs(i, &output);
            if (hr != S_OK)
            {
                break;
            }

            pst_output data_output = new st_output();

            data_output->output = output;
            output->GetDesc(&data_output->output_desc);
            data_output->device_index = data_adapter->adapter_index;
            data_output->output_index = output_index;

            output_index++;

            _map_output.insert({ data_output->output_index, data_output });

            output_count++;
        }

        if (output_count == 0)
        {
            data_adapter->adapter->Release();
            data_adapter->adapter = nullptr;

            delete data_adapter;
            data_adapter = nullptr;

            it_adapter = _map_adapter.erase(it_adapter);
        }
        else
        {
            it_adapter++;
        }
    }
}

void create_window(WCHAR* window_class, WCHAR* title, HINSTANCE instance, RECT rect, void* data, HWND& handle)
{
    AdjustWindowRect(&rect, WS_POPUP, FALSE);

    int left = rect.left;
    int top = rect.top;
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    handle = CreateWindow(window_class, title, WS_POPUP,
        left, top, width, height,
        NULL, NULL, instance, data);

    ShowWindow(handle, SW_SHOW);
    UpdateWindow(handle);
}

void create_windows()
{
    for (size_t i = 0; i < _create_window_count; i++)
    {
        HWND handle = nullptr;

        RECT rect{ 0, 0, 0, 0 };

        rect.left = _map_window_rect_left.find(i)->second;
        rect.top = _map_window_rect_top.find(i)->second;
        rect.right = _map_window_rect_left.find(i)->second + _map_window_rect_width.find(i)->second;
        rect.bottom = _map_window_rect_top.find(i)->second + _map_window_rect_height.find(i)->second;

        create_window(szWindowClass, szTitle, hInst, rect, nullptr, handle);

        if (handle != nullptr)
        {
            pst_window data_window = new st_window;
            data_window->handle = handle;

            data_window->event_window_to_adapter = CreateEvent(NULL, FALSE, FALSE, NULL);

            data_window->condition_variable_adapter_to_window = new std::condition_variable();
            data_window->mutex_adapter_to_window = new std::mutex();

            st_input_object event_window_to_adapter{};
            event_window_to_adapter.object = data_window->event_window_to_adapter;
            event_window_to_adapter.object_type = ObjectType::event_object;
            event_window_to_adapter.wait_type = WaitType::window_to_adapter;

            vector_input(event_window_to_adapter);


            st_input_object condition_variable_adapter_to_window{};
            condition_variable_adapter_to_window.object = data_window->condition_variable_adapter_to_window;
            condition_variable_adapter_to_window.object_type = ObjectType::condition_variable_object;
            condition_variable_adapter_to_window.wait_type = WaitType::adapter_to_window;

            st_input_object mutex_adapter_to_window{};
            mutex_adapter_to_window.object = data_window->mutex_adapter_to_window;
            mutex_adapter_to_window.object_type = ObjectType::mutex_object;
            mutex_adapter_to_window.wait_type = WaitType::adapter_to_window;

            st_input_object flag_adapter_to_window{};
            flag_adapter_to_window.object = &data_window->flag_adapter_to_window;
            flag_adapter_to_window.object_type = ObjectType::flag_object;
            flag_adapter_to_window.wait_type = WaitType::adapter_to_window;

            vector_input(condition_variable_adapter_to_window);
            vector_input(mutex_adapter_to_window);
            vector_input(flag_adapter_to_window);

            for (auto it_output = _map_output.begin(); it_output != _map_output.end(); it_output++)
            {
                pst_output data_output = it_output->second;

                if (!(data_output->output_desc.DesktopCoordinates.right > rect.left &&
                    data_output->output_desc.DesktopCoordinates.bottom > rect.top &&
                    data_output->output_desc.DesktopCoordinates.left <= rect.left &&
                    data_output->output_desc.DesktopCoordinates.top <= rect.top))
                {
                    continue;
                }

                GetWindowRect(data_window->handle, &data_window->rect);

                data_window->device_index = data_output->device_index;
                data_window->output_index = data_output->output_index;
                data_window->window_index = i;
                _map_window.insert({ data_window->window_index, data_window });

                break;
            }
        }
    }
}

void delete_adapter_has_none_window()
{
    for (auto it_adapter = _map_adapter.begin(); it_adapter != _map_adapter.end();)
    {
        pst_adapter data_adapter = it_adapter->second;

        int counter = 0;

        for (auto it_window = _map_window.begin(); it_window != _map_window.end(); it_window++)
        {
            pst_window data_window = it_window->second;

            if (data_window->device_index == data_adapter->adapter_index)
            {
                counter++;
                break;
            }
        }

        if (counter == 0)
        {
            data_adapter->adapter->Release();
            data_adapter->adapter = nullptr;

            delete data_adapter;
            data_adapter = nullptr;

            it_adapter = _map_adapter.erase(it_adapter);
        }
        else
        {
            it_adapter++;
        }
    }
}

void create_devices()
{
    HRESULT hr = S_OK;

    for (auto it_adapter = _map_adapter.begin(); it_adapter != _map_adapter.end(); it_adapter++)
    {
        pst_adapter data_adapter = it_adapter->second;

        ID3D12Device* device = nullptr;

        hr = D3D12CreateDevice(data_adapter->adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));

        if (hr == S_OK)
        {
            pst_device data_device = new st_device();

            data_device->device = device;
            data_device->adapter_index = data_adapter->adapter_index;
            data_device->device_index = data_adapter->adapter_index;

            NAME_D3D12_OBJECT_INDEXED(data_device->device, data_device->device_index, L"ID3D12Device");

            _map_device.insert({ data_device->device_index, data_device });
        }
    }
}

void create_command_queues()
{
    HRESULT hr = S_OK;

    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        ID3D12CommandQueue* command_queue = nullptr;

        D3D12_COMMAND_QUEUE_DESC queue_desc{};
        queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

        hr = data_device->device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue));

        if (hr == S_OK)
        {
            pst_command_queue data_command_queue = new st_command_queue();

            data_command_queue->command_queue = command_queue;

            data_command_queue->device_index = data_device->device_index;
            data_command_queue->command_queue_index = data_device->device_index;

            NAME_D3D12_OBJECT_INDEXED(data_command_queue->command_queue, data_command_queue->command_queue_index, L"ID3D12CommandQueue");

            _map_command_queue.insert({ data_command_queue->command_queue_index, data_command_queue });
        }
    }
}

void create_swap_chains()
{
    HRESULT hr = S_OK;

    for (auto it_window = _map_window.begin(); it_window != _map_window.end(); it_window++)
    {
        pst_window data_window = it_window->second;

        auto it_output = _map_output.find(data_window->output_index);
        if (it_output == _map_output.end())
        {
            continue;
        }
        pst_output data_output = it_output->second;

        auto it_command_queue = _map_command_queue.find(data_window->device_index);
        if (it_command_queue == _map_command_queue.end())
        {
            continue;
        }
        pst_command_queue data_command_queue = it_command_queue->second;

        IDXGISwapChain1* swap_chain = nullptr;

        DXGI_SWAP_CHAIN_DESC1 swap_chain_desc{};
        swap_chain_desc.Width = data_window->rect.right - data_window->rect.left;
        swap_chain_desc.Height = data_window->rect.bottom - data_window->rect.top;
        swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swap_chain_desc.Stereo = FALSE;
        swap_chain_desc.SampleDesc.Count = 1;
        swap_chain_desc.SampleDesc.Quality = 0;
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.BufferCount = frame_buffer_count;
        swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swap_chain_desc.Flags = 0;

        hr = _factory->CreateSwapChainForHwnd(
            data_command_queue->command_queue,
            data_window->handle,
            &swap_chain_desc,
            nullptr,
            nullptr,
            &swap_chain);
        if (hr == S_OK)
        {
            pst_swap_chain data_swap_chain = new st_swap_chain();

            data_swap_chain->swap_chain = swap_chain;
            data_swap_chain->window_index = data_window->window_index;
            data_swap_chain->device_index = data_window->device_index;
            data_swap_chain->swap_chain_index = data_window->window_index;

            _map_swap_chain.insert({ data_swap_chain->swap_chain_index, data_swap_chain });
        }
    }
}

void create_rtv_heaps()
{
    HRESULT hr = S_OK;

    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        ID3D12DescriptorHeap* rtv_heap = nullptr;

        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{};
        rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_heap_desc.NumDescriptors = rtv_descriptor_count;
        rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        hr = data_device->device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap));

        pst_rtv_heap data_rtv_heap = new st_rtv_heap();

        data_rtv_heap->rtv_heap = rtv_heap;
        data_rtv_heap->rtv_descriptor_size = data_device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        data_rtv_heap->device_index = data_device->device_index;
        data_rtv_heap->rtv_heap_index = data_device->device_index;

        NAME_D3D12_OBJECT_INDEXED(data_rtv_heap->rtv_heap, data_rtv_heap->device_index, L"ID3D12DescriptorHeap_rtv");

        _map_rtv_heap.insert({ data_rtv_heap->rtv_heap_index, data_rtv_heap });
    }
}

void create_rtvs()
{
    for (auto it_swap_chain = _map_swap_chain.begin(); it_swap_chain != _map_swap_chain.end(); it_swap_chain++)
    {
        pst_swap_chain data_swap_chain = it_swap_chain->second;

        auto it_rtv_heap = _map_rtv_heap.find(data_swap_chain->device_index);
        pst_rtv_heap data_rtv_heap = it_rtv_heap->second;

        pst_rtv data_rtv = new st_rtv();

        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = data_rtv_heap->rtv_heap->GetCPUDescriptorHandleForHeapStart();

        auto it_device = _map_device.find(data_swap_chain->device_index);
        pst_device data_device = it_device->second;

        for (size_t i = 0; i < frame_buffer_count; i++)
        {
            ID3D12Resource* rtv = nullptr;

            data_swap_chain->swap_chain->GetBuffer(i, IID_PPV_ARGS(&rtv));
            data_device->device->CreateRenderTargetView(rtv, nullptr, cpu_handle);

            data_rtv->vector_rtv_handle.push_back(cpu_handle);

            cpu_handle.ptr = cpu_handle.ptr + data_rtv_heap->rtv_descriptor_size;

            NAME_D3D12_OBJECT_INDEXED_2(rtv, data_swap_chain->device_index, i, L"ID3D12Resource_rtv");

            data_rtv->vector_rtv.push_back(rtv);
        }

        data_rtv->device_index = data_swap_chain->device_index;
        data_rtv->swap_chain_index = data_swap_chain->swap_chain_index;
        data_rtv->rtv_index = data_swap_chain->device_index;

        _map_rtv.insert({ data_rtv->rtv_index, data_rtv });
    }
}

void create_srv_heaps()
{
    HRESULT hr = S_OK;

    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        ID3D12DescriptorHeap* srv_heap = nullptr;

        D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc{};
        srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srv_heap_desc.NumDescriptors = srv_descriptor_count;
        srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        hr = data_device->device->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&srv_heap));

        pst_srv_heap data_srv_heap = new st_srv_heap();

        data_srv_heap->srv_heap = srv_heap;
        data_srv_heap->srv_descriptor_size = data_device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        data_srv_heap->device_index = data_device->device_index;
        data_srv_heap->srv_heap_index = data_device->device_index;

        NAME_D3D12_OBJECT_INDEXED(data_srv_heap->srv_heap, data_srv_heap->device_index, L"ID3D12DescriptorHeap_srv");

        _map_srv_heap.insert({ data_srv_heap->srv_heap_index , data_srv_heap });
    }
}

void create_command_allocators()
{
    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        pst_command_allocator data_command_allocator = new st_command_allocator();

        for (size_t i = 0; i < frame_buffer_count; i++)
        {
            ID3D12CommandAllocator* command_allocator = nullptr;

            data_device->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator));

            NAME_D3D12_OBJECT_INDEXED_2(command_allocator, data_device->device_index, i, L"ID3D12CommandAllocator");

            data_command_allocator->vector_command_allocator.push_back(command_allocator);
        }

        data_command_allocator->device_index = data_device->device_index;
        data_command_allocator->command_allocator_index = data_device->device_index;

        _map_command_allocator.insert({ data_command_allocator->command_allocator_index, data_command_allocator });
    }
}

void create_root_signatures()
{
    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        CD3DX12_DESCRIPTOR_RANGE1 ranges[3]{};
        CD3DX12_ROOT_PARAMETER1 root_parameters[3]{};

        // srv 1
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
        root_parameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        // srv 2
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
        root_parameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

        // srv 3
        ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
        root_parameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_sig_desc{};
        root_sig_desc.Init_1_1(_countof(root_parameters), root_parameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ID3D12RootSignature* root_sig = nullptr;

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        D3D12SerializeVersionedRootSignature(&root_sig_desc, &signature, &error);
        data_device->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_sig));

        NAME_D3D12_OBJECT_INDEXED(root_sig, data_device->device_index, L"ID3D12RootSignature");

        pst_root_signature data_root_sig = new st_root_signature();

        data_root_sig->root_sig = root_sig;
        data_root_sig->device_index = data_device->device_index;
        data_root_sig->root_sig_index = data_device->device_index;

        _map_root_signature.insert({ data_root_sig->root_sig_index, data_root_sig });
    }
}

void create_psos()
{
    HRESULT hr = S_OK;

    ComPtr<ID3DBlob> vertex_shader;
    ComPtr<ID3DBlob> pixel_shader;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compile_flags = 0;
#endif

    hr = D3DCompileFromFile(get_asset_full_path(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compile_flags, 0, &vertex_shader, nullptr);
    hr = D3DCompileFromFile(get_asset_full_path(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compile_flags, 0, &pixel_shader, nullptr);

    D3D12_INPUT_ELEMENT_DESC input_element_descs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        auto it_root_sig = _map_root_signature.find(data_device->device_index);
        pst_root_signature data_root_sig = it_root_sig->second;

        ID3D12PipelineState* pso = nullptr;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
        pso_desc.pRootSignature = data_root_sig->root_sig;
        pso_desc.VS = D3D12_SHADER_BYTECODE{ vertex_shader.Get()->GetBufferPointer(), vertex_shader.Get()->GetBufferSize() };
        pso_desc.PS = D3D12_SHADER_BYTECODE{ pixel_shader.Get()->GetBufferPointer(), pixel_shader.Get()->GetBufferSize() };
        pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        pso_desc.SampleMask = UINT_MAX;
        pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        pso_desc.DepthStencilState.DepthEnable = FALSE;
        pso_desc.DepthStencilState.StencilEnable = FALSE;
        pso_desc.InputLayout = { input_element_descs, _countof(input_element_descs) };
        pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso_desc.NumRenderTargets = 1;
        pso_desc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
        pso_desc.SampleDesc.Count = 1;
        pso_desc.NodeMask = 0;
        hr = data_device->device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pso));

        NAME_D3D12_OBJECT_INDEXED(pso, data_device->device_index, L"ID3D12PipelineState");

        pst_pso data_pso = new st_pso();

        data_pso->pso = pso;
        data_pso->device_index = data_device->device_index;
        data_pso->pso_index = data_device->device_index;

        _map_pso.insert({ data_pso->pso_index, data_pso });
    }
}

void create_command_lists()
{
    HRESULT hr = S_OK;

    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        auto it_command_allocator = _map_command_allocator.find(data_device->device_index);
        pst_command_allocator data_command_allocator = it_command_allocator->second;

        auto it_pso = _map_pso.find(data_device->device_index);
        pst_pso data_pso = it_pso->second;

        ID3D12GraphicsCommandList* command_list = nullptr;

        hr = data_device->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, data_command_allocator->vector_command_allocator[0], data_pso->pso, IID_PPV_ARGS(&command_list));
        command_list->Close();

        NAME_D3D12_OBJECT_INDEXED(command_list, data_device->device_index, L"ID3D12GraphicsCommandList");

        pst_command_list data_command_list = new st_command_list();

        data_command_list->command_list = command_list;
        data_command_list->device_index = data_device->device_index;
        data_command_list->command_list_index = data_device->device_index;

        _map_command_list.insert({ data_command_list->command_list_index, data_command_list });
    }
}

void create_fences()
{
    HRESULT hr = S_OK;

    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        ID3D12Fence* fence = nullptr;

        hr = data_device->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

        NAME_D3D12_OBJECT_INDEXED(fence, data_device->device_index, L"ID3D12Fence");

        HANDLE fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);

        pst_fence data_fence = new st_fence();

        data_fence->fence = fence;
        data_fence->fence_event = fence_event;
        data_fence->device_index = data_device->device_index;
        data_fence->fence_index = data_device->device_index;

        _map_fence.insert({ data_fence->fence_index, data_fence });
    }
}

void create_viewports()
{
    for (auto it_window = _map_window.begin(); it_window != _map_window.end(); it_window++)
    {
        pst_window data_window = it_window->second;

        D3D12_VIEWPORT viewport{};
        D3D12_RECT scissor_rect{};

        viewport = { 0.0f, 0.0f, (float)data_window->rect.right - data_window->rect.left, (float)data_window->rect.bottom - data_window->rect.top };
        scissor_rect = { 0, 0, data_window->rect.right - data_window->rect.left, data_window->rect.bottom - data_window->rect.top };

        pst_viewport data_viewport = new st_viewport();

        data_viewport->viewport = viewport;
        data_viewport->scissor_rect = scissor_rect;

        data_viewport->device_index = data_window->device_index;
        data_viewport->window_index = data_window->window_index;
        data_viewport->viewport_index = data_window->device_index;

        _map_viewport.insert({ data_viewport->viewport_index, data_viewport });
    }
}

void create_scenes()
{
    for (size_t i = 0; i < _prepare_scene_count; i++)
    {
        RECT rect{ 0, 0, 0, 0 };

        rect.left = _map_prepare_scene_rect_left.find(i)->second;
        rect.top = _map_prepare_scene_rect_top.find(i)->second;
        rect.right = _map_prepare_scene_rect_left.find(i)->second + _map_prepare_scene_rect_width.find(i)->second;
        rect.bottom = _map_prepare_scene_rect_top.find(i)->second + _map_prepare_scene_rect_height.find(i)->second;

        create_scene((char*)_map_prepare_scene_url.find(i)->second.c_str(), rect);
    }
}

void create_scene(char* url, RECT rect)
{
    void* instance = cpp_ffmpeg_wrapper_create();
    cpp_ffmpeg_wrapper_initialize(instance, callback_ffmpeg_wrapper_ptr);
    cpp_ffmpeg_wrapper_set_file_path(instance, url);
    if (cpp_ffmpeg_wrapper_open_file(instance) != 0)
    {
        cpp_ffmpeg_wrapper_shutdown(instance);
        cpp_ffmpeg_wrapper_delete(instance);

        return;
    }

    size_t scene_index = MAXSIZE_T;
    if (_free_scene_queue.empty())
    {
        scene_index = _next_scene_index;
        _next_scene_index++;
    }
    else
    {
        scene_index = _free_scene_queue.front();
        _free_scene_queue.pop_front();
    }

    cpp_ffmpeg_wrapper_set_scene_index(instance, scene_index);
    cpp_ffmpeg_wrapper_set_rect(instance, rect);
    cpp_ffmpeg_wrapper_play_start(instance, nullptr);

    uint64_t device_index = UINT64_MAX;
    uint64_t output_index = UINT64_MAX;

    for (auto it_output = _map_output.begin(); it_output != _map_output.end(); it_output++)
    {
        pst_output data_output = it_output->second;

        if (data_output->output_desc.DesktopCoordinates.left <= rect.left &&
            data_output->output_desc.DesktopCoordinates.top <= rect.top &&
            data_output->output_desc.DesktopCoordinates.right > rect.left &&
            data_output->output_desc.DesktopCoordinates.bottom > rect.top)
        {
            device_index = data_output->device_index;
            output_index = data_output->output_index;
            break;
        }
    }

    pst_scene data_scene = new st_scene();
    data_scene->instance = instance;
    data_scene->scene_index = scene_index;
    data_scene->device_index = device_index;
    data_scene->output_index = output_index;

    _map_scene.insert({ scene_index, data_scene });
}

void callback_ffmpeg_wrapper_ptr(void* param)
{

}

void create_vertex_buffer_512(pst_device data_device)
{
    HRESULT hr = S_OK;

    pst_vertex_buffer_512 data_vertex_buffer = new st_vertex_buffer_512();
    pst_vertex_upload_buffer_512 data_vertex_upload_buffer = new st_vertex_upload_buffer_512();
    pst_vertex_buffer_view_512 data_vertex_buffer_view = new st_vertex_buffer_view_512();

    data_vertex_buffer->vertex_buffer_index = data_device->device_index;
    data_vertex_upload_buffer->vertex_upload_buffer_index = data_device->device_index;
    data_vertex_buffer_view->vertex_buffer_view_index = data_device->device_index;

    for (size_t i = 0; i < 2; i++)
    {
        ID3D12Resource* vertex_buffer = nullptr;
        ID3D12Resource* vertex_upload_buffer = nullptr;
        D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view{};

        Vertex vertices[] =
        {
            { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }
        };

        if (i == 0)
        {
            vertices[0] = { { -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } };
            vertices[1] = { { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } };
            vertices[2] = { { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } };
            vertices[3] = { { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } };
        }
        else
        {
            vertices[0] = { { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } };
            vertices[1] = { { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } };
            vertices[2] = { { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } };
            vertices[3] = { { 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } };
        }

        const uint32_t vertex_buffer_size = sizeof(vertices);
        CD3DX12_HEAP_PROPERTIES vertex_buffer_properties(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_HEAP_PROPERTIES vertex_upload_buffer_properties(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC vertex_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size);

        hr = data_device->device->CreateCommittedResource(
            &vertex_buffer_properties,
            D3D12_HEAP_FLAG_NONE,
            &vertex_buffer_desc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&vertex_buffer)
        );
        NAME_D3D12_OBJECT_INDEXED_2(vertex_buffer, data_device->device_index, i, L"ID3D12Resource_vertex_buffer");

        hr = data_device->device->CreateCommittedResource(
            &vertex_upload_buffer_properties,
            D3D12_HEAP_FLAG_NONE,
            &vertex_buffer_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertex_upload_buffer)
        );
        NAME_D3D12_OBJECT_INDEXED_2(vertex_upload_buffer, data_device->device_index, i, L"ID3D12Resource_vertex_upload_buffer");

        D3D12_SUBRESOURCE_DATA vertex_data{};
        vertex_data.pData = vertices;
        vertex_data.RowPitch = sizeof(Vertex);
        vertex_data.SlicePitch = sizeof(vertices);

        auto it_command_list = _map_command_list.find(data_device->device_index);
        pst_command_list data_command_list = it_command_list->second;

        CD3DX12_RESOURCE_BARRIER transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertex_buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
        data_command_list->command_list->ResourceBarrier(1, &transition_barrier);
        UpdateSubresources(data_command_list->command_list, vertex_buffer, vertex_upload_buffer, 0, 0, 1, &vertex_data);
        transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertex_buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        data_command_list->command_list->ResourceBarrier(1, &transition_barrier);

        vertex_buffer_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
        vertex_buffer_view.StrideInBytes = sizeof(Vertex);
        vertex_buffer_view.SizeInBytes = vertex_buffer_size;

        data_vertex_buffer->vector_vertex_buffer.push_back(vertex_buffer);
        data_vertex_upload_buffer->vector_vertex_upload_buffer.push_back(vertex_upload_buffer);
        data_vertex_buffer_view->vector_vertex_buffer_view.push_back(vertex_buffer_view);
    }

    _map_vertex_buffer_512.insert({ data_vertex_buffer->vertex_buffer_index, data_vertex_buffer });
    _map_vertex_upload_buffer_512.insert({ data_vertex_upload_buffer->vertex_upload_buffer_index, data_vertex_upload_buffer });
    _map_vertex_buffer_view_512.insert({ data_vertex_buffer_view->vertex_buffer_view_index, data_vertex_buffer_view });
}

void create_index_buffer(pst_device data_device)
{
    HRESULT hr = S_OK;

    ID3D12Resource* index_buffer = nullptr;
    ID3D12Resource* index_upload_buffer = nullptr;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view{};

    uint32_t indices[] =
    {
        0, 1, 2,
        1, 3, 2
    };

    const uint32_t index_buffer_size = sizeof(indices);

    CD3DX12_HEAP_PROPERTIES index_buffer_properties(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_HEAP_PROPERTIES index_upload_buffer_properties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC index_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(index_buffer_size);

    D3D12_SUBRESOURCE_DATA index_data{};
    index_data.pData = indices;
    index_data.RowPitch = sizeof(uint32_t) * 3;
    index_data.SlicePitch = sizeof(indices);

    hr = data_device->device->CreateCommittedResource(
        &index_buffer_properties,
        D3D12_HEAP_FLAG_NONE,
        &index_buffer_desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&index_buffer)
    );
    NAME_D3D12_OBJECT_INDEXED(index_buffer, data_device->device_index, L"ID3D12Resource_index_buffer");

    hr = data_device->device->CreateCommittedResource(
        &index_upload_buffer_properties,
        D3D12_HEAP_FLAG_NONE,
        &index_buffer_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&index_upload_buffer)
    );
    NAME_D3D12_OBJECT_INDEXED(index_upload_buffer, data_device->device_index, L"ID3D12Resource_index_upload_buffer");

    auto it_command_list = _map_command_list.find(data_device->device_index);
    pst_command_list data_command_list = it_command_list->second;

    CD3DX12_RESOURCE_BARRIER transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(index_buffer, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    data_command_list->command_list->ResourceBarrier(1, &transition_barrier);
    UpdateSubresources(data_command_list->command_list, index_buffer, index_upload_buffer, 0, 0, 1, &index_data);
    transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(index_buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    data_command_list->command_list->ResourceBarrier(1, &transition_barrier);

    index_buffer_view.BufferLocation = index_buffer->GetGPUVirtualAddress();
    index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
    index_buffer_view.SizeInBytes = index_buffer_size;

    pst_index_buffer data_index_buffer = new st_index_buffer();
    data_index_buffer->index_buffer = index_buffer;
    data_index_buffer->index_buffer_index = data_device->device_index;
    _map_index_buffer.insert({ data_index_buffer->index_buffer_index, data_index_buffer });

    pst_index_upload_buffer data_index_upload_buffer = new st_index_upload_buffer();
    data_index_upload_buffer->index_upload_buffer = index_upload_buffer;
    data_index_upload_buffer->index_upload_buffer_index = data_device->device_index;
    _map_index_upload_buffer.insert({ data_index_upload_buffer->index_upload_buffer_index, data_index_upload_buffer });

    pst_index_buffer_view data_index_buffer_view = new st_index_buffer_view();
    data_index_buffer_view->index_buffer_view = index_buffer_view;
    data_index_buffer_view->index_buffer_view_index = data_device->device_index;
    _map_index_buffer_view.insert({ data_index_buffer_view->index_buffer_view_index, data_index_buffer_view });
}

void create_textures(uint64_t width, uint64_t height)
{
    HRESULT hr = S_OK;

    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        std::vector<ID3D12Resource*> vector_texture_y;
        std::vector<ID3D12Resource*> vector_texture_u;
        std::vector<ID3D12Resource*> vector_texture_v;
        std::vector<ID3D12Resource*> vector_upload_texture_y;
        std::vector<ID3D12Resource*> vector_upload_texture_u;
        std::vector<ID3D12Resource*> vector_upload_texture_v;
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> vector_srv_handle_cpu_y;
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> vector_srv_handle_cpu_u;
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> vector_srv_handle_cpu_v;
        std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> vector_srv_handle_gpu_y;
        std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> vector_srv_handle_gpu_u;
        std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> vector_srv_handle_gpu_v;

        pst_device data_device = it_device->second;

        auto it_srv_heap = _map_srv_heap.find(data_device->device_index);
        pst_srv_heap data_srv_heap = it_srv_heap->second;
        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu(data_srv_heap->srv_heap->GetCPUDescriptorHandleForHeapStart());
        D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu(data_srv_heap->srv_heap->GetGPUDescriptorHandleForHeapStart());

        const uint32_t srv_descriptor_size = data_device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        uint64_t texture_count = 0;

        for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
        {
            pst_scene data_scene = it_scene->second;

            if (data_scene->device_index == data_device->device_index)
            {
                texture_count++;
            }
        }

        for (size_t i = 0; i < _prepare_texture_count * texture_count; i++)
        {
            D3D12_RESOURCE_DESC texture_desc{};
            texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            texture_desc.Alignment = 0;
            texture_desc.Width = width;
            texture_desc.Height = height;
            texture_desc.DepthOrArraySize = 1;
            texture_desc.MipLevels = 1;
            texture_desc.Format = DXGI_FORMAT_R8_UNORM;
            texture_desc.SampleDesc.Count = 1;
            texture_desc.SampleDesc.Quality = 0;
            texture_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            texture_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

            CD3DX12_HEAP_PROPERTIES texture_properties(D3D12_HEAP_TYPE_DEFAULT);


            ID3D12Resource* texture_y = nullptr;
            hr = data_device->device->CreateCommittedResource(
                &texture_properties,
                D3D12_HEAP_FLAG_NONE,
                &texture_desc,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                nullptr,
                IID_PPV_ARGS(&texture_y)
            );
            vector_texture_y.push_back(texture_y);
            NAME_D3D12_OBJECT_INDEXED_2(texture_y, data_device->device_index, i, L"ID3D12Resource_texture_y");


            texture_desc.Width /= 2;
            texture_desc.Height /= 2;

            ID3D12Resource* texture_u = nullptr;
            hr = data_device->device->CreateCommittedResource(
                &texture_properties,
                D3D12_HEAP_FLAG_NONE,
                &texture_desc,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                nullptr,
                IID_PPV_ARGS(&texture_u)
            );
            vector_texture_u.push_back(texture_u);
            NAME_D3D12_OBJECT_INDEXED_2(texture_u, data_device->device_index, i, L"ID3D12Resource_texture_u");

            ID3D12Resource* texture_v = nullptr;
            hr = data_device->device->CreateCommittedResource(
                &texture_properties,
                D3D12_HEAP_FLAG_NONE,
                &texture_desc,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                nullptr,
                IID_PPV_ARGS(&texture_v)
            );
            vector_texture_v.push_back(texture_v);
            NAME_D3D12_OBJECT_INDEXED_2(texture_v, data_device->device_index, i, L"ID3D12Resource_texture_v");


            uint64_t upload_buffer_size = 0;
            texture_desc.Format = DXGI_FORMAT_R8_TYPELESS;
            texture_desc.Width = width;
            texture_desc.Height = height;
            data_device->device->GetCopyableFootprints(&texture_desc, 0, 1, 0, nullptr, nullptr, nullptr, &upload_buffer_size);

            D3D12_RESOURCE_DESC upload_desc{};
            upload_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            upload_desc.Alignment = 0;
            upload_desc.Width = upload_buffer_size;
            upload_desc.Height = 1;
            upload_desc.DepthOrArraySize = 1;
            upload_desc.MipLevels = 1;
            upload_desc.Format = DXGI_FORMAT_UNKNOWN;
            upload_desc.SampleDesc.Count = 1;
            upload_desc.SampleDesc.Quality = 0;
            upload_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            upload_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

            CD3DX12_HEAP_PROPERTIES texture_upload_heap_properties(D3D12_HEAP_TYPE_UPLOAD);


            ID3D12Resource* upload_texture_y = nullptr;
            hr = data_device->device->CreateCommittedResource(
                &texture_upload_heap_properties,
                D3D12_HEAP_FLAG_NONE,
                &upload_desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&upload_texture_y)
            );
            vector_upload_texture_y.push_back(upload_texture_y);
            NAME_D3D12_OBJECT_INDEXED_2(upload_texture_y, data_device->device_index, i, L"ID3D12Resource_upload_texture_y");


            texture_desc.Width /= 2;
            texture_desc.Height /= 2;
            data_device->device->GetCopyableFootprints(&texture_desc, 0, 1, 0, nullptr, nullptr, nullptr, &upload_buffer_size);
            upload_desc.Width = upload_buffer_size;

            ID3D12Resource* upload_texture_u = nullptr;
            hr = data_device->device->CreateCommittedResource(
                &texture_upload_heap_properties,
                D3D12_HEAP_FLAG_NONE,
                &upload_desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&upload_texture_u)
            );
            vector_upload_texture_u.push_back(upload_texture_u);
            NAME_D3D12_OBJECT_INDEXED_2(upload_texture_u, data_device->device_index, i, L"ID3D12Resource_upload_texture_u");

            ID3D12Resource* upload_texture_v = nullptr;
            hr = data_device->device->CreateCommittedResource(
                &texture_upload_heap_properties,
                D3D12_HEAP_FLAG_NONE,
                &upload_desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&upload_texture_v)
            );
            vector_upload_texture_v.push_back(upload_texture_v);
            NAME_D3D12_OBJECT_INDEXED_2(upload_texture_v, data_device->device_index, i, L"ID3D12Resource_upload_texture_v");


            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
            srv_desc.Format = DXGI_FORMAT_R8_UNORM;
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srv_desc.Texture2D.MostDetailedMip = 0;
            srv_desc.Texture2D.MipLevels = 1;
            srv_desc.Texture2D.PlaneSlice = 0;
            srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

            D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_y = srv_handle_cpu;
            srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + INT64(srv_descriptor_size));
            vector_srv_handle_cpu_y.push_back(srv_handle_cpu_y);

            D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_u = srv_handle_cpu;
            srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + INT64(srv_descriptor_size));
            vector_srv_handle_cpu_u.push_back(srv_handle_cpu_u);

            D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_v = srv_handle_cpu;
            srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + INT64(srv_descriptor_size));
            vector_srv_handle_cpu_v.push_back(srv_handle_cpu_v);


            data_device->device->CreateShaderResourceView(texture_y, &srv_desc, srv_handle_cpu_y);
            data_device->device->CreateShaderResourceView(texture_u, &srv_desc, srv_handle_cpu_u);
            data_device->device->CreateShaderResourceView(texture_v, &srv_desc, srv_handle_cpu_v);


            D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_y = srv_handle_gpu;
            srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + INT64(srv_descriptor_size));
            vector_srv_handle_gpu_y.push_back(srv_handle_gpu_y);

            D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_u = srv_handle_gpu;
            srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + INT64(srv_descriptor_size));
            vector_srv_handle_gpu_u.push_back(srv_handle_gpu_u);

            D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_v = srv_handle_gpu;
            srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + INT64(srv_descriptor_size));
            vector_srv_handle_gpu_v.push_back(srv_handle_gpu_v);
        }

        pst_texture data_texture_y = new st_texture();
        data_texture_y->vector_texture = vector_texture_y;
        data_texture_y->device_index = data_device->device_index;
        _map_texture_y.insert({ data_device->device_index, data_texture_y });

        pst_texture data_texture_u = new st_texture();
        data_texture_u->vector_texture = vector_texture_u;
        data_texture_u->device_index = data_device->device_index;
        _map_texture_u.insert({ data_device->device_index, data_texture_u });

        pst_texture data_texture_v = new st_texture();
        data_texture_v->vector_texture = vector_texture_v;
        data_texture_v->device_index = data_device->device_index;
        _map_texture_v.insert({ data_device->device_index, data_texture_v });


        pst_texture data_upload_texture_y = new st_texture();
        data_upload_texture_y->vector_texture = vector_upload_texture_y;
        data_upload_texture_y->device_index = data_device->device_index;
        _map_upload_texture_y.insert({ data_device->device_index, data_upload_texture_y });

        pst_texture data_upload_texture_u = new st_texture();
        data_upload_texture_u->vector_texture = vector_upload_texture_u;
        data_upload_texture_u->device_index = data_device->device_index;
        _map_upload_texture_u.insert({ data_device->device_index, data_upload_texture_u });

        pst_texture data_upload_texture_v = new st_texture();
        data_upload_texture_v->vector_texture = vector_upload_texture_v;
        data_upload_texture_v->device_index = data_device->device_index;
        _map_upload_texture_v.insert({ data_device->device_index, data_upload_texture_v });


        pst_srv_handle data_srv_handle_y = new st_srv_handle();
        data_srv_handle_y->vector_handle_cpu = vector_srv_handle_cpu_y;
        data_srv_handle_y->vector_handle_gpu = vector_srv_handle_gpu_y;
        data_srv_handle_y->device_index = data_device->device_index;
        _map_srv_handle_y.insert({ data_device->device_index, data_srv_handle_y });

        pst_srv_handle data_srv_handle_u = new st_srv_handle();
        data_srv_handle_u->vector_handle_cpu = vector_srv_handle_cpu_u;
        data_srv_handle_u->vector_handle_gpu = vector_srv_handle_gpu_u;
        data_srv_handle_u->device_index = data_device->device_index;
        _map_srv_handle_u.insert({ data_device->device_index, data_srv_handle_u });

        pst_srv_handle data_srv_handle_v = new st_srv_handle();
        data_srv_handle_v->vector_handle_cpu = vector_srv_handle_cpu_v;
        data_srv_handle_v->vector_handle_gpu = vector_srv_handle_gpu_v;
        data_srv_handle_v->device_index = data_device->device_index;
        _map_srv_handle_v.insert({ data_device->device_index, data_srv_handle_v });
    }
}

void prepare_frames()
{
    int32_t result = INT32_MIN;

    for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
    {
        pst_scene data_scene = it_scene->second;

        for (size_t i = 0; i < _prepare_texture_count; i++)
        {
            AVFrame* frame = av_frame_alloc();

            result = INT32_MIN;
            while (result < 0)
            {
                result = cpp_ffmpeg_wrapper_get_frame(data_scene->instance, frame);
            }

            result = INT32_MIN;
            while (result != 0)
            {
                result = cpp_ffmpeg_wrapper_frame_to_next_non_waiting(data_scene->instance);
            }

            data_scene->vector_frame.push_back(frame);
        }
    }
}

void prepare_texture(pst_device data_device)
{
    auto it_command_list = _map_command_list.find(data_device->device_index);
    pst_command_list data_command_list = it_command_list->second;

    auto it_texture_y = _map_texture_y.find(data_device->device_index);
    pst_texture data_texture_y = it_texture_y->second;
    auto it_texture_u = _map_texture_u.find(data_device->device_index);
    pst_texture data_texture_u = it_texture_u->second;
    auto it_texture_v = _map_texture_v.find(data_device->device_index);
    pst_texture data_texture_v = it_texture_v->second;

    auto it_upload_texture_y = _map_upload_texture_y.find(data_device->device_index);
    pst_texture data_upload_texture_y = it_upload_texture_y->second;
    auto it_upload_texture_u = _map_upload_texture_u.find(data_device->device_index);
    pst_texture data_upload_texture_u = it_upload_texture_u->second;
    auto it_upload_texture_v = _map_upload_texture_v.find(data_device->device_index);
    pst_texture data_upload_texture_v = it_upload_texture_v->second;

    uint64_t counter = 0;
    
    for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
    {
        pst_scene data_scene = it_scene->second;

        if (data_scene->device_index != data_device->device_index)
        {
            continue;
        }

        for (size_t i = 0 + (_prepare_texture_count * counter); i < _prepare_texture_count * (counter + 1); i++)
        {
            AVFrame* frame = data_scene->vector_frame.at(i - (_prepare_texture_count * counter));

            ID3D12Resource* texture_y = nullptr;
            ID3D12Resource* texture_u = nullptr;
            ID3D12Resource* texture_v = nullptr;
            ID3D12Resource* upload_texture_y = nullptr;
            ID3D12Resource* upload_texture_u = nullptr;
            ID3D12Resource* upload_texture_v = nullptr;

            texture_y = data_texture_y->vector_texture.at(i);
            texture_u = data_texture_u->vector_texture.at(i);
            texture_v = data_texture_v->vector_texture.at(i);
            upload_texture_y = data_upload_texture_y->vector_texture.at(i);
            upload_texture_u = data_upload_texture_u->vector_texture.at(i);
            upload_texture_v = data_upload_texture_v->vector_texture.at(i);

            D3D12_SUBRESOURCE_DATA texture_data_y{};
            texture_data_y.pData = frame->data[0];
            texture_data_y.RowPitch = frame->linesize[0];
            texture_data_y.SlicePitch = texture_data_y.RowPitch * frame->height;

            CD3DX12_RESOURCE_BARRIER transition_barrier_y = CD3DX12_RESOURCE_BARRIER::Transition(texture_y, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
            data_command_list->command_list->ResourceBarrier(1, &transition_barrier_y);
            UpdateSubresources(data_command_list->command_list, texture_y, upload_texture_y, 0, 0, 1, &texture_data_y);
            transition_barrier_y = CD3DX12_RESOURCE_BARRIER::Transition(texture_y, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            data_command_list->command_list->ResourceBarrier(1, &transition_barrier_y);

            D3D12_SUBRESOURCE_DATA texture_data_u{};
            texture_data_u.pData = frame->data[1];
            texture_data_u.RowPitch = frame->linesize[1];
            texture_data_u.SlicePitch = texture_data_u.RowPitch * frame->height / 2;

            CD3DX12_RESOURCE_BARRIER transition_barrier_u = CD3DX12_RESOURCE_BARRIER::Transition(texture_u, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
            data_command_list->command_list->ResourceBarrier(1, &transition_barrier_u);
            UpdateSubresources(data_command_list->command_list, texture_u, upload_texture_u, 0, 0, 1, &texture_data_u);
            transition_barrier_u = CD3DX12_RESOURCE_BARRIER::Transition(texture_u, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            data_command_list->command_list->ResourceBarrier(1, &transition_barrier_u);

            D3D12_SUBRESOURCE_DATA texture_data_v{};
            texture_data_v.pData = frame->data[2];
            texture_data_v.RowPitch = frame->linesize[2];
            texture_data_v.SlicePitch = texture_data_v.RowPitch * frame->height / 2;

            CD3DX12_RESOURCE_BARRIER transition_barrier_v = CD3DX12_RESOURCE_BARRIER::Transition(texture_v, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
            data_command_list->command_list->ResourceBarrier(1, &transition_barrier_v);
            UpdateSubresources(data_command_list->command_list, texture_v, upload_texture_v, 0, 0, 1, &texture_data_v);
            transition_barrier_v = CD3DX12_RESOURCE_BARRIER::Transition(texture_v, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            data_command_list->command_list->ResourceBarrier(1, &transition_barrier_v);
        }

        counter++;
    }
}



void delete_factory()
{
    if (_factory)
    {
        _factory->Release();
        _factory = nullptr;
    }
}

void delete_adapters()
{
    for (auto it_adapter = _map_adapter.begin(); it_adapter != _map_adapter.end();)
    {
        pst_adapter data_adapter = it_adapter->second;

        if (data_adapter->adapter != nullptr)
        {
            data_adapter->adapter->Release();
            data_adapter->adapter = nullptr;
        }

        delete data_adapter;
        data_adapter = nullptr;

        it_adapter = _map_adapter.erase(it_adapter);
    }
}

void delete_outputs()
{
    for (auto it_output = _map_output.begin(); it_output != _map_output.end();)
    {
        pst_output data_output = it_output->second;

        if (data_output->output != nullptr)
        {
            data_output->output->Release();
            data_output->output = nullptr;
        }

        delete data_output;
        data_output = nullptr;

        it_output = _map_output.erase(it_output);
    }
}

void delete_windows()
{
    for (auto it_window = _map_window.begin(); it_window != _map_window.end();)
    {
        pst_window data_window = it_window->second;

        if (data_window->handle)
        {
            DestroyWindow(data_window->handle);
            data_window->handle = nullptr;
        }

        delete data_window;
        data_window = nullptr;

        it_window = _map_window.erase(it_window);
    }
}

void delete_devices()
{
    for (auto it_device = _map_device.begin(); it_device != _map_device.end();)
    {
        pst_device data_device = it_device->second;

        if (data_device->device)
        {
            data_device->device->Release();
            data_device->device = nullptr;
        }

        delete data_device;
        data_device = nullptr;

        it_device = _map_device.erase(it_device);
    }
}

void delete_command_queues()
{
    for (auto it_command_queue = _map_command_queue.begin(); it_command_queue != _map_command_queue.end();)
    {
        pst_command_queue data_command_queue = it_command_queue->second;

        if (data_command_queue->command_queue)
        {
            data_command_queue->command_queue->Release();
            data_command_queue->command_queue = nullptr;
        }

        delete data_command_queue;
        data_command_queue = nullptr;

        it_command_queue = _map_command_queue.erase(it_command_queue);
    }
}

void delete_swap_chains()
{
    for (auto it_swap_chain = _map_swap_chain.begin(); it_swap_chain != _map_swap_chain.end();)
    {
        pst_swap_chain data_swap_chain = it_swap_chain->second;

        if (data_swap_chain->swap_chain)
        {
            data_swap_chain->swap_chain->Release();
            data_swap_chain->swap_chain = nullptr;
        }

        delete data_swap_chain;
        data_swap_chain = nullptr;

        it_swap_chain = _map_swap_chain.erase(it_swap_chain);
    }
}

void delete_rtv_heaps()
{
    for (auto it_rtv_heap = _map_rtv_heap.begin(); it_rtv_heap != _map_rtv_heap.end();)
    {
        pst_rtv_heap data_rtv_heap = it_rtv_heap->second;

        if (data_rtv_heap->rtv_heap)
        {
            data_rtv_heap->rtv_heap->Release();
            data_rtv_heap->rtv_heap = nullptr;
        }

        delete data_rtv_heap;
        data_rtv_heap = nullptr;

        it_rtv_heap = _map_rtv_heap.erase(it_rtv_heap);
    }
}

void delete_rtvs()
{
    for (auto it_rtv = _map_rtv.begin(); it_rtv != _map_rtv.end();)
    {
        pst_rtv data_rtv = it_rtv->second;

        for (auto it_vector = data_rtv->vector_rtv.begin(); it_vector != data_rtv->vector_rtv.end(); )
        {
            ID3D12Resource* rtv = *it_vector;

            rtv->Release();
            rtv = nullptr;

            it_vector = data_rtv->vector_rtv.erase(it_vector);
        }

        delete data_rtv;
        data_rtv = nullptr;

        it_rtv = _map_rtv.erase(it_rtv);
    }
}

void delete_srv_heaps()
{
    for (auto it_srv_heap = _map_srv_heap.begin(); it_srv_heap != _map_srv_heap.end();)
    {
        pst_srv_heap data_srv_heap = it_srv_heap->second;

        if (data_srv_heap->srv_heap)
        {
            data_srv_heap->srv_heap->Release();
            data_srv_heap->srv_heap = nullptr;
        }

        delete data_srv_heap;
        data_srv_heap = nullptr;

        it_srv_heap = _map_srv_heap.erase(it_srv_heap);
    }
}

void delete_command_allocators()
{
    for (auto it_command_allocator = _map_command_allocator.begin(); it_command_allocator != _map_command_allocator.end();)
    {
        pst_command_allocator data_command_allocator = it_command_allocator->second;

        for (auto it_vector = data_command_allocator->vector_command_allocator.begin(); it_vector != data_command_allocator->vector_command_allocator.end();)
        {
            ID3D12CommandAllocator* command_allocator = *it_vector;

            command_allocator->Release();
            command_allocator = nullptr;

            it_vector = data_command_allocator->vector_command_allocator.erase(it_vector);
        }

        delete data_command_allocator;
        data_command_allocator = nullptr;

        it_command_allocator = _map_command_allocator.erase(it_command_allocator);
    }
}

void delete_root_signatures()
{
    for (auto it_root_sig = _map_root_signature.begin(); it_root_sig != _map_root_signature.end();)
    {
        pst_root_signature data_root_sig = it_root_sig->second;

        if (data_root_sig->root_sig)
        {
            data_root_sig->root_sig->Release();
            data_root_sig->root_sig = nullptr;
        }

        delete data_root_sig;
        data_root_sig = nullptr;

        it_root_sig = _map_root_signature.erase(it_root_sig);
    }
}

void delete_psos()
{
    for (auto it_pso = _map_pso.begin(); it_pso != _map_pso.end();)
    {
        pst_pso data_pso = it_pso->second;

        if (data_pso->pso)
        {
            data_pso->pso->Release();
            data_pso->pso = nullptr;
        }

        delete data_pso;
        data_pso = nullptr;

        it_pso = _map_pso.erase(it_pso);
    }
}

void delete_command_lists()
{
    for (auto it_command_list = _map_command_list.begin(); it_command_list != _map_command_list.end();)
    {
        pst_command_list data_command_list = it_command_list->second;

        if (data_command_list->command_list)
        {
            data_command_list->command_list->Release();
            data_command_list->command_list = nullptr;
        }

        delete data_command_list;
        data_command_list = nullptr;

        it_command_list = _map_command_list.erase(it_command_list);
    }
}

void delete_fences()
{
    for (auto it_fence = _map_fence.begin(); it_fence != _map_fence.end();)
    {
        pst_fence data_fence = it_fence->second;

        if (data_fence->fence)
        {
            data_fence->fence->Release();
            data_fence->fence = nullptr;
        }

        if (data_fence->fence_event)
        {
            CloseHandle(data_fence->fence_event);
            data_fence->fence_event = nullptr;
        }

        delete data_fence;
        data_fence = nullptr;

        it_fence = _map_fence.erase(it_fence);
    }
}

void delete_viewports()
{
    for (auto it_viewport = _map_viewport.begin(); it_viewport != _map_viewport.end();)
    {
        pst_viewport data_viewport = it_viewport->second;

        delete data_viewport;
        data_viewport = nullptr;

        it_viewport = _map_viewport.erase(it_viewport);
    }
}

void delete_scenes()
{
    for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end();)
    {
        pst_scene data_scene = it_scene->second;

        if (data_scene->instance)
        {
            cpp_ffmpeg_wrapper_play_stop(data_scene->instance, nullptr);
            cpp_ffmpeg_wrapper_shutdown(data_scene->instance);
            cpp_ffmpeg_wrapper_delete(data_scene->instance);
            data_scene->instance = nullptr;
        }

        delete data_scene;
        data_scene = nullptr;

        it_scene = _map_scene.erase(it_scene);
    }
}

void delete_vertex_buffers_512()
{
    for (auto it_vertex_buffer = _map_vertex_buffer_512.begin(); it_vertex_buffer != _map_vertex_buffer_512.end();)
    {
        pst_vertex_buffer_512 data_vertex_buffer = it_vertex_buffer->second;

        for (auto it_vector = data_vertex_buffer->vector_vertex_buffer.begin(); it_vector != data_vertex_buffer->vector_vertex_buffer.end();)
        {
            ID3D12Resource* vertex_buffer = *it_vector;
            vertex_buffer->Release();
            vertex_buffer = nullptr;

            it_vector = data_vertex_buffer->vector_vertex_buffer.erase(it_vector);
        }

        delete data_vertex_buffer;
        data_vertex_buffer = nullptr;

        it_vertex_buffer = _map_vertex_buffer_512.erase(it_vertex_buffer);
    }

    for (auto it_vertex_upload_buffer = _map_vertex_upload_buffer_512.begin(); it_vertex_upload_buffer != _map_vertex_upload_buffer_512.end();)
    {
        pst_vertex_upload_buffer_512 data_vertex_upload_buffer = it_vertex_upload_buffer->second;

        for (auto it_vector = data_vertex_upload_buffer->vector_vertex_upload_buffer.begin(); it_vector != data_vertex_upload_buffer->vector_vertex_upload_buffer.end();)
        {
            ID3D12Resource* vertex_upload_buffer = *it_vector;
            vertex_upload_buffer->Release();
            vertex_upload_buffer = nullptr;

            it_vector = data_vertex_upload_buffer->vector_vertex_upload_buffer.erase(it_vector);
        }

        delete data_vertex_upload_buffer;
        data_vertex_upload_buffer = nullptr;

        it_vertex_upload_buffer = _map_vertex_upload_buffer_512.erase(it_vertex_upload_buffer);
    }
}

void delete_index_buffers()
{
    for (auto it_index_buffer = _map_index_buffer.begin(); it_index_buffer != _map_index_buffer.end();)
    {
        pst_index_buffer data_index_buffer = it_index_buffer->second;

        if (data_index_buffer->index_buffer)
        {
            data_index_buffer->index_buffer->Release();
            data_index_buffer->index_buffer = nullptr;
        }

        delete data_index_buffer;
        data_index_buffer = nullptr;

        it_index_buffer = _map_index_buffer.erase(it_index_buffer);
    }

    for (auto it_index_upload_buffer = _map_index_upload_buffer.begin(); it_index_upload_buffer != _map_index_upload_buffer.end();)
    {
        pst_index_upload_buffer data_index_upload_buffer = it_index_upload_buffer->second;

        if (data_index_upload_buffer->index_upload_buffer)
        {
            data_index_upload_buffer->index_upload_buffer->Release();
            data_index_upload_buffer->index_upload_buffer = nullptr;
        }

        delete data_index_upload_buffer;
        data_index_upload_buffer = nullptr;

        it_index_upload_buffer = _map_index_upload_buffer.erase(it_index_upload_buffer);
    }
}

void delete_textures()
{
    for (auto it_texture_y = _map_texture_y.begin(); it_texture_y != _map_texture_y.end();)
    {
        pst_texture data_texture_y = it_texture_y->second;

        for (auto it_vector_texture_y = data_texture_y->vector_texture.begin(); it_vector_texture_y != data_texture_y->vector_texture.end();)
        {
            ID3D12Resource* texture_y = *it_vector_texture_y;
            texture_y->Release();
            texture_y = nullptr;

            it_vector_texture_y = data_texture_y->vector_texture.erase(it_vector_texture_y);
        }

        delete data_texture_y;
        data_texture_y = nullptr;

        it_texture_y = _map_texture_y.erase(it_texture_y);
    }

    for (auto it_texture_u = _map_texture_u.begin(); it_texture_u != _map_texture_u.end();)
    {
        pst_texture data_texture_u = it_texture_u->second;

        for (auto it_vector_texture_u = data_texture_u->vector_texture.begin(); it_vector_texture_u != data_texture_u->vector_texture.end();)
        {
            ID3D12Resource* texture_u = *it_vector_texture_u;
            texture_u->Release();
            texture_u = nullptr;

            it_vector_texture_u = data_texture_u->vector_texture.erase(it_vector_texture_u);
        }

        delete data_texture_u;
        data_texture_u = nullptr;

        it_texture_u = _map_texture_u.erase(it_texture_u);
    }

    for (auto it_texture_v = _map_texture_v.begin(); it_texture_v != _map_texture_v.end();)
    {
        pst_texture data_texture_v = it_texture_v->second;

        for (auto it_vector_texture_v = data_texture_v->vector_texture.begin(); it_vector_texture_v != data_texture_v->vector_texture.end();)
        {
            ID3D12Resource* texture_v = *it_vector_texture_v;
            texture_v->Release();
            texture_v = nullptr;

            it_vector_texture_v = data_texture_v->vector_texture.erase(it_vector_texture_v);
        }

        delete data_texture_v;
        data_texture_v = nullptr;

        it_texture_v = _map_texture_v.erase(it_texture_v);
    }

    for (auto it_upload_texture_y = _map_upload_texture_y.begin(); it_upload_texture_y != _map_upload_texture_y.end();)
    {
        pst_texture data_upload_texture_y = it_upload_texture_y->second;

        for (auto it_vector_upload_texture_y = data_upload_texture_y->vector_texture.begin(); it_vector_upload_texture_y != data_upload_texture_y->vector_texture.end();)
        {
            ID3D12Resource* upload_texture_y = *it_vector_upload_texture_y;
            upload_texture_y->Release();
            upload_texture_y = nullptr;

            it_vector_upload_texture_y = data_upload_texture_y->vector_texture.erase(it_vector_upload_texture_y);
        }

        delete data_upload_texture_y;
        data_upload_texture_y = nullptr;

        it_upload_texture_y = _map_upload_texture_y.erase(it_upload_texture_y);
    }

    for (auto it_upload_texture_u = _map_upload_texture_u.begin(); it_upload_texture_u != _map_upload_texture_u.end();)
    {
        pst_texture data_upload_texture_u = it_upload_texture_u->second;

        for (auto it_vector_upload_texture_u = data_upload_texture_u->vector_texture.begin(); it_vector_upload_texture_u != data_upload_texture_u->vector_texture.end();)
        {
            ID3D12Resource* upload_texture_u = *it_vector_upload_texture_u;
            upload_texture_u->Release();
            upload_texture_u = nullptr;

            it_vector_upload_texture_u = data_upload_texture_u->vector_texture.erase(it_vector_upload_texture_u);
        }

        delete data_upload_texture_u;
        data_upload_texture_u = nullptr;

        it_upload_texture_u = _map_upload_texture_u.erase(it_upload_texture_u);
    }

    for (auto it_upload_texture_v = _map_upload_texture_v.begin(); it_upload_texture_v != _map_upload_texture_v.end();)
    {
        pst_texture data_upload_texture_v = it_upload_texture_v->second;

        for (auto it_vector_upload_texture_v = data_upload_texture_v->vector_texture.begin(); it_vector_upload_texture_v != data_upload_texture_v->vector_texture.end();)
        {
            ID3D12Resource* upload_texture_v = *it_vector_upload_texture_v;
            upload_texture_v->Release();
            upload_texture_v = nullptr;

            it_vector_upload_texture_v = data_upload_texture_v->vector_texture.erase(it_vector_upload_texture_v);
        }

        delete data_upload_texture_v;
        data_upload_texture_v = nullptr;

        it_upload_texture_v = _map_upload_texture_v.erase(it_upload_texture_v);
    }

    for (auto it_srv_handle_y = _map_srv_handle_y.begin(); it_srv_handle_y != _map_srv_handle_y.end();)
    {
        pst_srv_handle data_srv_handle_y = it_srv_handle_y->second;

        data_srv_handle_y->vector_handle_cpu.clear();
        data_srv_handle_y->vector_handle_gpu.clear();

        delete data_srv_handle_y;
        data_srv_handle_y = nullptr;

        it_srv_handle_y = _map_srv_handle_y.erase(it_srv_handle_y);
    }

    for (auto it_srv_handle_u = _map_srv_handle_u.begin(); it_srv_handle_u != _map_srv_handle_u.end();)
    {
        pst_srv_handle data_srv_handle_u = it_srv_handle_u->second;

        data_srv_handle_u->vector_handle_cpu.clear();
        data_srv_handle_u->vector_handle_gpu.clear();

        delete data_srv_handle_u;
        data_srv_handle_u = nullptr;

        it_srv_handle_u = _map_srv_handle_u.erase(it_srv_handle_u);
    }

    for (auto it_srv_handle_v = _map_srv_handle_v.begin(); it_srv_handle_v != _map_srv_handle_v.end();)
    {
        pst_srv_handle data_srv_handle_v = it_srv_handle_v->second;

        data_srv_handle_v->vector_handle_cpu.clear();
        data_srv_handle_v->vector_handle_gpu.clear();

        delete data_srv_handle_v;
        data_srv_handle_v = nullptr;

        it_srv_handle_v = _map_srv_handle_v.erase(it_srv_handle_v);
    }
}

void delete_frames()
{
    for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
    {
        pst_scene data_scene = it_scene->second;

        for (auto it_frame = data_scene->vector_frame.begin(); it_frame != data_scene->vector_frame.end();)
        {
            AVFrame* frame = *it_frame;

            av_frame_free(&frame);

            it_frame = data_scene->vector_frame.erase(it_frame);
        }
    }
}




void initialize_swap_lock(ID3D12Device* device, IDXGISwapChain1* swap_chain)
{
    NvU32 max_group = 0;
    NvU32 max_barrier = 0;

    NvAPI_Status nvapi_status = NVAPI_OK;

    nvapi_status = NvAPI_D3D1x_QueryMaxSwapGroup(device, &max_group, &max_barrier);
    if (!(max_group > 0 && max_barrier > 0))
    {
        return;
    }

    nvapi_status = NvAPI_D3D1x_JoinSwapGroup(device, swap_chain, _swap_group, _block_swap_group_present);

    nvapi_status = NvAPI_D3D1x_BindSwapBarrier(device, _swap_group, _swap_barrier);
}

void initialize_swap_locks()
{
    for (auto it_swap_chain = _map_swap_chain.begin(); it_swap_chain != _map_swap_chain.end(); it_swap_chain++)
    {
        pst_swap_chain data_swap_chain = it_swap_chain->second;

        auto it_device = _map_device.find(data_swap_chain->device_index);
        if (it_device == _map_device.end())
        {
            continue;
        }

        pst_device data_device = it_device->second;

        initialize_swap_lock(data_device->device, data_swap_chain->swap_chain);
    }
}

void delete_swap_lock(ID3D12Device* device, IDXGISwapChain1* swap_chain)
{
    NvAPI_D3D1x_BindSwapBarrier(device, _swap_group, 0);
    NvAPI_D3D1x_JoinSwapGroup(device, swap_chain, 0, false);
}

void delete_swap_locks()
{
    for (auto it_swap_chain = _map_swap_chain.begin(); it_swap_chain != _map_swap_chain.end(); it_swap_chain++)
    {
        pst_swap_chain data_swap_chain = it_swap_chain->second;

        auto it_device = _map_device.find(data_swap_chain->device_index);
        if (it_device == _map_device.end())
        {
            continue;
        }

        pst_device data_device = it_device->second;

        delete_swap_lock(data_device->device, data_swap_chain->swap_chain);

        data_swap_chain->swap_chain = nullptr;
    }
}



void vector_input(st_input_object object)
{
    std::lock_guard<std::mutex> lk(_mutex_input_object);

    _vector_input_object.push_back(object);
}

void thread_vector_input()
{
    while (_flag_vector_input)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_main_loop));

        std::lock_guard<std::mutex> lk(_mutex_input_object);

        size_t size = _vector_input_object.size();

        if (size == 0)
        {
            continue;
        }

        for (auto it = _vector_input_object.begin(); it != _vector_input_object.end();)
        {
            st_input_object object = *it;

            HANDLE event_object = nullptr;
            std::condition_variable* condition_variable_object = nullptr;
            std::mutex* mutex_object = nullptr;
            bool* flag_object = nullptr;

            switch (object.object_type)
            {
            case ObjectType::event_object:
            {
                event_object = (HANDLE)object.object;
            }
            break;
            case ObjectType::condition_variable_object:
            {
                condition_variable_object = (std::condition_variable*)object.object;
            }
            break;
            case ObjectType::mutex_object:
            {
                mutex_object = (std::mutex*)object.object;
            }
            break;
            case ObjectType::flag_object:
            {
                flag_object = (bool*)object.object;
            }
            break;
            default:
                break;
            }

            switch (object.wait_type)
            {
            case WaitType::adapter_to_window:
            {
                if (event_object != nullptr)
                {
                    _mutex_vector_event_adapter_to_window.lock();
                    _vector_event_adapter_to_window.push_back(event_object);
                    _mutex_vector_event_adapter_to_window.unlock();
                }

                if (condition_variable_object != nullptr)
                {
                    _mutex_vector_condition_variable_adapter_to_window.lock();
                    _vector_condition_variable_adapter_to_window.push_back(condition_variable_object);
                    _mutex_vector_condition_variable_adapter_to_window.unlock();
                }

                if (mutex_object != nullptr)
                {
                    _mutex_vector_mutex_adapter_to_window.lock();
                    _vector_mutex_adapter_to_window.push_back(mutex_object);
                    _mutex_vector_mutex_adapter_to_window.unlock();
                }

                if (flag_object != nullptr)
                {
                    _mutex_vector_flag_adapter_to_window.lock();
                    _vector_flag_adapter_to_window.push_back(flag_object);
                    _mutex_vector_flag_adapter_to_window.unlock();
                }
            }
            break;
            case WaitType::window_to_adapter:
            {
                if (event_object != nullptr)
                {
                    _mutex_vector_event_window_to_adapter.lock();
                    _vector_event_window_to_adapter.push_back(event_object);
                    _mutex_vector_event_window_to_adapter.unlock();
                }

                if (condition_variable_object != nullptr)
                {
                    _mutex_vector_condition_variable_window_to_adapter.lock();
                    _vector_condition_variable_window_to_adapter.push_back(condition_variable_object);
                    _mutex_vector_condition_variable_window_to_adapter.unlock();
                }

                if (mutex_object != nullptr)
                {
                    _mutex_vector_mutex_window_to_adapter.lock();
                    _vector_mutex_window_to_adapter.push_back(mutex_object);
                    _mutex_vector_mutex_window_to_adapter.unlock();
                }

                if (flag_object != nullptr)
                {
                    _mutex_vector_flag_window_to_adapter.lock();
                    _vector_flag_window_to_adapter.push_back(flag_object);
                    _mutex_vector_flag_window_to_adapter.unlock();
                }
            }
            break;
            default:
                break;
            }

            it = _vector_input_object.erase(it);
        }
    }
}

void thread_wait_for_multiple_objects(WaitType wait_type, bool* flag_thread)
{
    std::vector<HANDLE>* vector_handle = nullptr;
    std::vector<std::condition_variable*>* vector_condition_variable = nullptr;
    std::vector<std::mutex*>* vector_mutex = nullptr;
    std::vector<bool*>* vector_flag = nullptr;

    bool flag_timeout = false;
    bool flag_vector_empty = false;

    while (*flag_thread == true)
    {
        switch (wait_type)
        {
        case WaitType::adapter_to_window:
        {
            vector_handle = &_vector_event_adapter_to_window;


            vector_condition_variable = &_vector_condition_variable_adapter_to_window;
            _mutex_vector_condition_variable_adapter_to_window.lock();
            if (vector_condition_variable->size() < _create_window_count)
            {
                flag_vector_empty = true;
            }
            _mutex_vector_condition_variable_adapter_to_window.unlock();

            vector_mutex = &_vector_mutex_adapter_to_window;
            _mutex_vector_mutex_adapter_to_window.lock();
            if (vector_mutex->size() < _create_window_count)
            {
                flag_vector_empty = true;
            }
            _mutex_vector_mutex_adapter_to_window.unlock();

            vector_flag = &_vector_flag_adapter_to_window;
            _mutex_vector_flag_adapter_to_window.lock();
            if (vector_flag->size() < _create_window_count)
            {
                flag_vector_empty = true;
            }
            _mutex_vector_flag_adapter_to_window.unlock();
        }
        break;
        case WaitType::window_to_adapter:
        {
            vector_handle = &_vector_event_window_to_adapter;
            _mutex_vector_event_window_to_adapter.lock();
            if (vector_handle->size() < _create_window_count)
            {
                flag_vector_empty = true;
            }
            _mutex_vector_event_window_to_adapter.unlock();

            vector_condition_variable = &_vector_condition_variable_window_to_adapter;

            vector_mutex = &_vector_mutex_window_to_adapter;

            vector_flag = &_vector_flag_window_to_adapter;
        }
        break;
        default:
            break;
        }

        if (flag_vector_empty == true)
        {
            flag_vector_empty = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_main_loop));
            continue;
        }

        DWORD hr = 0;

        // Wait Multiple Objects
        DWORD n_count = vector_handle->size();

        size_t n_cv_count = vector_condition_variable->size();

        HANDLE* lp_handles = vector_handle->data();
        bool b_wait_all = true;
        DWORD dw_milliseconds = 1000;
        hr = WaitForMultipleObjects(n_count, lp_handles, b_wait_all, dw_milliseconds);
        if (hr >= WAIT_OBJECT_0 && hr < WAIT_ABANDONED_0)
        {
        }
        else if (hr >= WAIT_ABANDONED_0 && hr < WAIT_TIMEOUT)
        {
            flag_timeout = true;
        }
        else if (hr == WAIT_TIMEOUT)
        {
            flag_timeout = true;
        }
        else if (hr == WAIT_FAILED)
        {
            flag_timeout = true;
        }

        if (flag_timeout == true)
        {
            flag_timeout = false;

            continue;
        }

        // Notify to condition variables
        for (int i = 0; i < n_cv_count; i++)
        {
            ((*vector_mutex)[i])->lock();
            ((*vector_condition_variable)[i])->notify_one();
            (*(*vector_flag)[i]) = true;
            ((*vector_mutex)[i])->unlock();
        }
    }
}


void thread_window(pst_window data_window)
{
    auto it_swap_chain = _map_swap_chain.find(data_window->window_index);
    pst_swap_chain data_swap_chain = it_swap_chain->second;
    IDXGISwapChain1* swap_chain = data_swap_chain->swap_chain;

    auto it_device = _map_device.find(data_window->device_index);
    pst_device data_device = it_device->second;
    ID3D12Device* device = data_device->device;

    while (data_window->flag_thread_window)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_main_loop));

        {
            // Wait condition_variable_adapter_to_window
            std::unique_lock<std::mutex> lk(*data_window->mutex_adapter_to_window);
            if (data_window->flag_adapter_to_window == false)
            {
                data_window->condition_variable_adapter_to_window->wait(lk);
            }
            data_window->flag_adapter_to_window = false;
        }

        if (_nvapi_initialized)
        {
            NvAPI_Status status = NvAPI_D3D1x_Present(device, swap_chain, (UINT)1, (UINT)0);
        }
        else
        {
            swap_chain->Present(1, 0);
        }

        // SetEvent to event_window_to_adapter
        SetEvent(data_window->event_window_to_adapter);
    }
}

void thread_adapter()
{
    float color_offset = 0.0f;
    int vector_rtv_index = -1;
    int srv_index = -1;

    int counter_entry = 0;
    int size_adapter = _map_adapter.size();

    while (_flag_thread_adapter)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_main_loop));

        vector_rtv_index += 1;
        if (!(vector_rtv_index < frame_buffer_count))
        {
            vector_rtv_index = 0;
        }

        srv_index += 1;
        if (!(srv_index < _prepare_texture_count))
        {
            srv_index = 0;
        }

        for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
        {
            pst_device data_device = it_device->second;

            auto it_command_allocator = _map_command_allocator.find(data_device->device_index);
            pst_command_allocator data_command_allocator = it_command_allocator->second;
            ID3D12CommandAllocator* command_allocator = data_command_allocator->vector_command_allocator.at(vector_rtv_index);

            auto it_command_list = _map_command_list.find(data_device->device_index);
            pst_command_list data_command_list = it_command_list->second;
            ID3D12GraphicsCommandList* command_list = data_command_list->command_list;

            auto it_pso = _map_pso.find(data_device->device_index);
            pst_pso data_pso = it_pso->second;
            ID3D12PipelineState* pipeline_state = data_pso->pso;

            command_allocator->Reset();
            command_list->Reset(command_allocator, pipeline_state);

            if (counter_entry < size_adapter)
            {
                counter_entry++;

                create_vertex_buffer_512(data_device);
                create_index_buffer(data_device);
                prepare_texture(data_device);
            }

            auto it_root_sig = _map_root_signature.find(data_device->device_index);
            pst_root_signature data_root_sig = it_root_sig->second;

            command_list->SetGraphicsRootSignature(data_root_sig->root_sig);

            auto it_srv_heap = _map_srv_heap.find(data_device->device_index);
            pst_srv_heap data_srv_heap = it_srv_heap->second;
            ID3D12DescriptorHeap* pp_heaps[] = { data_srv_heap->srv_heap };

            command_list->SetDescriptorHeaps(_countof(pp_heaps), pp_heaps);

            auto it_rtv = _map_rtv.find(data_device->device_index);
            pst_rtv data_rtv = it_rtv->second;
            CD3DX12_RESOURCE_BARRIER barrier_before = CD3DX12_RESOURCE_BARRIER::Transition(data_rtv->vector_rtv.at(vector_rtv_index), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            command_list->ResourceBarrier(1, &barrier_before);

            command_list->OMSetRenderTargets(1, &data_rtv->vector_rtv_handle.at(vector_rtv_index), FALSE, nullptr);
            float color[4] = { color_offset, color_offset, color_offset, color_offset };
            command_list->ClearRenderTargetView(data_rtv->vector_rtv_handle.at(vector_rtv_index), color, 0, nullptr);
            command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            auto it_index_buffer_view = _map_index_buffer_view.find(data_device->device_index);
            pst_index_buffer_view data_index_buffer_view = it_index_buffer_view->second;
            command_list->IASetIndexBuffer(&data_index_buffer_view->index_buffer_view);
            
            uint64_t counter = 0;
            for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
            {
                pst_scene data_scene = it_scene->second;

                if (data_scene->device_index != data_device->device_index)
                {
                    continue;
                }

                auto it_vertex_buffer_view_512 = _map_vertex_buffer_view_512.find(data_device->device_index);
                pst_vertex_buffer_view_512 data_vertex_buffer_view_512 = it_vertex_buffer_view_512->second;
                command_list->IASetVertexBuffers(0, 1, &data_vertex_buffer_view_512->vector_vertex_buffer_view.at(counter));

                auto it_viewport = _map_viewport.find(data_scene->device_index);
                pst_viewport data_viewport = it_viewport->second;
                command_list->RSSetViewports(1, &data_viewport->viewport);
                command_list->RSSetScissorRects(1, &data_viewport->scissor_rect);

                auto it_srv_handle_y = _map_srv_handle_y.find(data_device->device_index);
                pst_srv_handle data_srv_handle_y = it_srv_handle_y->second;
                command_list->SetGraphicsRootDescriptorTable(0, data_srv_handle_y->vector_handle_gpu.at(srv_index + (_prepare_texture_count * counter)));

                auto it_srv_handle_u = _map_srv_handle_u.find(data_device->device_index);
                pst_srv_handle data_srv_handle_u = it_srv_handle_u->second;
                command_list->SetGraphicsRootDescriptorTable(1, data_srv_handle_u->vector_handle_gpu.at(srv_index + (_prepare_texture_count * counter)));

                auto it_srv_handle_v = _map_srv_handle_v.find(data_device->device_index);
                pst_srv_handle data_srv_handle_v = it_srv_handle_v->second;
                command_list->SetGraphicsRootDescriptorTable(2, data_srv_handle_v->vector_handle_gpu.at(srv_index + (_prepare_texture_count * counter)));

                command_list->DrawIndexedInstanced(6, 1, 0, 0, 0);

                counter++;
            }

            CD3DX12_RESOURCE_BARRIER barrier_after = CD3DX12_RESOURCE_BARRIER::Transition(data_rtv->vector_rtv.at(vector_rtv_index), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            command_list->ResourceBarrier(1, &barrier_after);

            command_list->Close();

            ID3D12CommandList* command_lists[] = { command_list };

            auto it_command_queue = _map_command_queue.find(data_device->device_index);
            pst_command_queue data_command_queue = it_command_queue->second;
            ID3D12CommandQueue* command_queue = data_command_queue->command_queue;

            command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

            auto it_fence = _map_fence.find(data_device->device_index);
            pst_fence data_fence = it_fence->second;

            data_fence->fence->SetEventOnCompletion(data_fence->fence_value, data_fence->fence_event);
            command_queue->Signal(data_fence->fence, data_fence->fence_value);

            data_fence->fence_value++;
            if (data_fence->fence_value == UINT64_MAX)
            {
                data_fence->fence_value = 0;
            }
        }

        color_offset += 0.1f;
        if (color_offset > 1.0f)
        {
            color_offset = 0.0f;
        }

        for (auto it_fence = _map_fence.begin(); it_fence != _map_fence.end(); it_fence++)
        {
            pst_fence data_fence = it_fence->second;

            WaitForSingleObject(data_fence->fence_event, INFINITE);
        }
        
        // SetEvent to event_adapter_to_window
        SetEvent(_event_adapter_to_window);

        {
            // Wait condition_variable_window_to_adapter
            std::unique_lock<std::mutex> lk(*_mutex_window_to_adapter);
            if (_flag_window_to_adapter == false)
            {
                _condition_variable_window_to_adapter->wait(lk);
            }
            _flag_window_to_adapter = false;
        }
    }
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

    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WPLAYER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    config_setting();

    if (_use_nvapi)
    {
        _nvapi_status = NvAPI_Initialize();
        if (_nvapi_status == NVAPI_OK)
        {
            _nvapi_initialized = true;
        }
    }

    _thread_packet_processing = std::thread(thread_packet_processing);
    _thread_tcp_server = std::thread(thread_tcp_server);

    _thread_vector_input = std::thread(thread_vector_input);

    wchar_t asset_path[512];
    get_asset_path(asset_path, _countof(asset_path));
    _asset_path = asset_path;

    // create
    // factory
    create_factory();

    // adapter
    enum_adapters();

    // output
    enum_outputs();

    // window
    create_windows();

    delete_adapter_has_none_window();

    // device
    create_devices();

    // command_queue
    create_command_queues();

    // swapchain
    create_swap_chains();

    // swap_lock
    if (_nvapi_initialized)
    {
        initialize_swap_locks();
    }

    // rtv_heap
    create_rtv_heaps();

    // rtv
    create_rtvs();

    // srv_heap
    create_srv_heaps();

    // command_allocator
    create_command_allocators();

    // root signature
    create_root_signatures();

    // pso
    create_psos();

    // command_list
    create_command_lists();

    // fence
    create_fences();

    // viewport
    create_viewports();

    // scene
    create_scenes();

    create_textures(_scene_texture_width, _scene_texture_height);

    prepare_frames();

    _thread_wait_for_multiple_objects_adapter_to_window = std::thread(thread_wait_for_multiple_objects, WaitType::adapter_to_window, &_flag_wait_for_multiple_objects_adapter_to_window);
    _thread_wait_for_multiple_objects_window_to_adapter = std::thread(thread_wait_for_multiple_objects, WaitType::window_to_adapter, &_flag_wait_for_multiple_objects_window_to_adapter);

    for (auto it_window = _map_window.begin(); it_window != _map_window.end(); it_window++)
    {
        pst_window data_window = it_window->second;

        std::thread* thread_window_ = new std::thread(thread_window, data_window);

        _map_thread_window.insert({ data_window->window_index, thread_window_ });
    }

    _thread_adapter = std::thread(thread_adapter);

    MSG msg{};

    // 기본 메시지 루프입니다:
    while (_is_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_main_loop));

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // delete

    if (_thread_tcp_server.joinable())
    {
        _flag_tcp_server = false;
        _thread_tcp_server.join();
    }

    if (_thread_packet_processing.joinable())
    {
        _flag_packet_processing = false;
        _thread_packet_processing.join();
    }

    if (_thread_adapter.joinable())
    {
        _flag_thread_adapter = false;
        _condition_variable_window_to_adapter->notify_one();
        _flag_window_to_adapter = true;
        _thread_adapter.join();
    }

    for (auto it_thread_window = _map_thread_window.begin(); it_thread_window != _map_thread_window.end();)
    {
        std::thread* thread_window = it_thread_window->second;

        auto it_window = _map_window.find(it_thread_window->first);
        pst_window data_window = it_window->second;

        if (thread_window->joinable())
        {
            data_window->flag_thread_window = false;
            data_window->condition_variable_adapter_to_window->notify_one();
            data_window->flag_adapter_to_window = true;
            thread_window->join();
        }

        delete thread_window;
        thread_window = nullptr;

        it_thread_window = _map_thread_window.erase(it_thread_window);
    }

    if (_thread_wait_for_multiple_objects_adapter_to_window.joinable())
    {
        _flag_wait_for_multiple_objects_adapter_to_window = false;
        _thread_wait_for_multiple_objects_adapter_to_window.join();
    }

    if (_thread_wait_for_multiple_objects_window_to_adapter.joinable())
    {
        _flag_wait_for_multiple_objects_window_to_adapter = false;
        _thread_wait_for_multiple_objects_window_to_adapter.join();
    }

    if (_thread_vector_input.joinable())
    {
        _flag_vector_input = false;
        _thread_vector_input.join();
    }

    if (_nvapi_initialized == true)
    {
        NvAPI_Unload();
    }

    delete_frames();

    delete_textures();
    delete_index_buffers();
    delete_vertex_buffers_512();

    delete_scenes();

    delete_viewports();
    delete_fences();
    delete_command_lists();

    delete_psos();
    delete_root_signatures();
    delete_command_allocators();

    delete_srv_heaps();
    delete_rtvs();
    delete_rtv_heaps();

    delete_swap_locks();

    delete_swap_chains();
    delete_command_queues();
    delete_devices();
    delete_windows();
    delete_outputs();
    delete_adapters();
    delete_factory();

#if _DEBUG
    d3d_memory_check();
#endif

    return (int)msg.wParam;
}
