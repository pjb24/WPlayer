// DPlayer.cpp : ���ø����̼ǿ� ���� �������� �����մϴ�.
//

// // SDKDDKVer.h�� �����ϸ� �ְ� ������ ���뼺�� ���� Windows �÷����� ���ǵ˴ϴ�.
// ���� Windows �÷����� ���ø����̼��� �����Ϸ��� ��쿡�� SDKDDKVer.h�� �����ϱ� ����
// WinSDKVer.h�� �����ϰ� _WIN32_WINNT�� �����Ϸ��� �÷������� �����մϴ�.
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����
#include <windows.h>
// C ��Ÿ�� ��� �����Դϴ�.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "DPlayer.h"

// --------------------------------

#if _DEBUG
#include <crtdbg.h>
#endif // _DEBUG

#include <thread>
#include <mutex>
#include <deque>
#include <map>

#if _DEBUG
#include "dxgidebug.h"
#endif // _DEBUG

#include "GraphicsHeaders.h"

#include "nvapi.h"
#pragma comment(lib, "nvapi64.lib")

#include "CppSocketAPI.h"
#include "PacketDefine.h"
#include "ApiFunctionStructures.h"
#pragma comment(lib, "CppSocket.lib")

#include "CppFFmpegWrapperAPI.h"
#include "CppFFmpegWrapperCallbackStruct.h"
#pragma comment(lib, "CppFFmpegWrapper.lib")

#include "libavutil/hwcontext_d3d12va.h"


extern "C"
{
#include "libavutil/time.h"
}

#pragma comment(lib, "avutil.lib")

#include "StringConverter.h"

// --------------------------------

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"

#pragma comment(lib, "spdlog.lib")

// --------------------------------

#define MAX_LOADSTRING 100

// --------------------------------

#pragma region Enumerations

enum class ObjectType
{
    none = 0,
    object_event = 1,
    object_condition_variable = 2,
    object_mutex = 3,
    object_flag = 4,

};

enum class WaitType
{
    none = 0,

    scene_to_upload = 1,
    upload_to_device = 2,
    device_to_window = 3,
    window_to_scene = 4,
};

#pragma endregion

// --------------------------------

#pragma region Structures

struct NormalizedRect
{
    float left;
    float top;
    float right;
    float bottom;
};

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 uv;

};

struct st_input_object
{
    void* object = nullptr;

    ObjectType object_type = ObjectType::none;
    WaitType wait_type = WaitType::none;

};

typedef struct st_adapter
{
    IDXGIAdapter1* adapter = nullptr;
    DXGI_ADAPTER_DESC1 adapter_desc{};

    uint64_t adapter_index = UINT64_MAX;

}*pst_adapter;

typedef struct st_device
{
    ID3D12Device* device = nullptr;

    uint64_t device_index = UINT64_MAX;

    bool flag_thread_device = true;
    bool flag_thread_upload = true;

    HANDLE event_device_to_window = nullptr;

    std::condition_variable* condition_variable_upload_to_device = nullptr;
    std::mutex* mutex_upload_to_device = nullptr;
    bool flag_upload_to_device = false;


    HANDLE event_upload_to_device = nullptr;

    std::condition_variable* condition_variable_scene_to_upload = nullptr;
    std::mutex* mutex_scene_to_upload = nullptr;
    bool flag_scene_to_upload = false;

}*pst_device;

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

    HANDLE event_window_to_scene = nullptr;

    std::condition_variable* condition_variable_device_to_window = nullptr;
    std::mutex* mutex_device_to_window = nullptr;
    bool flag_device_to_window = false;

    bool flag_first_entry = true;

}*pst_window;

typedef struct st_command_queue
{
    ID3D12CommandQueue* command_queue = nullptr;

    uint64_t device_index = UINT64_MAX;

}*pst_command_queue;

typedef struct st_swap_chain
{
    IDXGISwapChain1* swap_chain = nullptr;

    uint64_t window_index = UINT64_MAX;
    uint64_t device_index = UINT64_MAX;

}*pst_swap_chain;

typedef struct st_rtv_heap
{
    ID3D12DescriptorHeap* rtv_heap = nullptr;
    uint32_t rtv_descriptor_size = UINT32_MAX;

    uint64_t device_index = UINT64_MAX;

}*pst_rtv_heap;

typedef struct st_rtv
{
    std::vector<ID3D12Resource*> vector_rtv;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> vector_rtv_handle;

    uint64_t window_index = UINT64_MAX;
    uint64_t device_index = UINT64_MAX;

}*pst_rtv;

typedef struct st_srv_heap
{
    ID3D12DescriptorHeap* srv_heap = nullptr;
    uint32_t srv_descriptor_size = UINT32_MAX;

    uint64_t device_index = UINT64_MAX;

}*pst_srv_heap;

typedef struct st_command_allocator
{
    std::vector<ID3D12CommandAllocator*> vector_command_allocator;

    uint64_t device_index = UINT64_MAX;

}*pst_command_allocator;

typedef struct st_root_signature
{
    ID3D12RootSignature* root_sig = nullptr;

    uint64_t device_index = UINT64_MAX;

}*pst_root_signature;

typedef struct st_pipeline_state_object
{
    ID3D12PipelineState* pso = nullptr;

    uint64_t device_index = UINT64_MAX;

}*pst_pipeline_state_object, st_pso, * pst_pso;

typedef struct st_command_list
{
    ID3D12GraphicsCommandList* command_list = nullptr;

    uint64_t device_index = UINT64_MAX;

}*pst_command_list;

typedef struct st_fence
{
    ID3D12Fence* fence_device = nullptr;
    ID3D12Fence* fence_upload = nullptr;
    HANDLE fence_event_device = nullptr;
    HANDLE fence_event_upload = nullptr;

    uint64_t fence_value_device = 0;
    uint64_t fence_value_upload = 0;

    uint64_t device_index = UINT64_MAX;

}*pst_fence;

typedef struct st_viewport
{
    D3D12_VIEWPORT viewport{};
    D3D12_RECT scissor_rect{};

    uint64_t window_index = UINT64_MAX;
    uint64_t device_index = UINT64_MAX;

}*pst_viewport;

typedef struct st_vertex_buffer
{
    std::vector<ID3D12Resource*> vector_vertex_buffer;

    uint64_t device_index = UINT64_MAX;

}*pst_vertex_buffer;

typedef struct st_vertex_upload_buffer
{
    std::vector<ID3D12Resource*> vector_vertex_upload_buffer;

    uint64_t device_index = UINT64_MAX;

}*pst_vertex_upload_buffer;

typedef struct st_vertex_buffer_view
{
    std::vector<D3D12_VERTEX_BUFFER_VIEW> vector_vertex_buffer_view{};

    uint64_t device_index = UINT64_MAX;

}*pst_vertex_buffer_view;

typedef struct st_index_buffer
{
    ID3D12Resource* index_buffer = nullptr;

    uint64_t device_index = UINT64_MAX;

}*pst_index_buffer;

typedef struct st_index_upload_buffer
{
    ID3D12Resource* index_upload_buffer = nullptr;

    uint64_t device_index = UINT64_MAX;

}*pst_index_upload_buffer;

typedef struct st_index_buffer_view
{
    D3D12_INDEX_BUFFER_VIEW index_buffer_view{};

    uint64_t device_index = UINT64_MAX;

}*pst_index_buffer_view;

typedef struct st_texture
{
    std::vector<ID3D12Resource*> vector_texture;

    uint64_t device_index = UINT64_MAX;

}*pst_texture;

typedef struct st_upload_texture
{
    std::vector<ID3D12Resource*> vector_texture;

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout{};
    UINT numRows = 0;
    UINT64 rowSizeInBytes = 0;
    UINT64 totalBytes = 0;

    uint64_t device_index = UINT64_MAX;

}*pst_upload_texture;

typedef struct st_srv_handle
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> vector_handle_cpu;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> vector_handle_gpu;

    uint64_t device_index = UINT64_MAX;

}*pst_srv_handle;

typedef struct st_scene
{
    void* ffmpeg_instance = nullptr;

    std::string url;
    RECT rect{ 0, 0, 0, 0 };
    NormalizedRect normal_rect{ 0.0f, 0.0f, 0.0f, 0.0f };

    uint64_t scene_index = UINT64_MAX;
    uint64_t device_index = UINT64_MAX;
    uint64_t output_index = UINT64_MAX;

    bool flag_thread_scene = true;

    HANDLE event_scene_to_upload = nullptr;

    std::condition_variable* condition_variable_window_to_scene = nullptr;
    std::mutex* mutex_window_to_scene = nullptr;
    bool flag_window_to_scene = false;

    std::vector<AVFrame*> vector_frame;

}*pst_scene;

typedef struct st_coordinate
{
    int left;
    int top;
    int width;
    int height;
}*pst_coordinate;

#pragma endregion

// --------------------------------

// ���� ����:
HINSTANCE hInst;                                // ���� �ν��Ͻ��Դϴ�.
WCHAR szTitle[MAX_LOADSTRING];                  // ���� ǥ���� �ؽ�Ʈ�Դϴ�.
WCHAR szWindowClass[MAX_LOADSTRING];            // �⺻ â Ŭ���� �̸��Դϴ�.

bool _is_running = true;
std::wstring _asset_path;

constexpr int64_t _sleep_time_main_loop = 1;
constexpr int64_t _sleep_time_processing = 10;
constexpr uint32_t _frame_buffer_count = 3;
constexpr uint32_t _rtv_descriptor_count = 4096;
constexpr uint32_t _srv_descriptor_count = 4096;
constexpr uint32_t _texture_resource_count_nv12 = 2;
constexpr uint32_t _texture_resource_count_yuv = 3;

#pragma region Config Values

// Server IP
std::string _ip;
// Server PORT
uint16_t _port = UINT16_MAX;

bool _flag_set_logger = false;

int _log_level = 6;

// �ؽ�ó Ÿ�� ����. 0: NV12, 1: YUV
int _texture_type = 0;

// nvapi ���
bool _use_nvapi = false;

// nvapi�� ����� present���� ���
bool _block_swap_group_present = false;

// SetMaximumFrameLatency. 0: unset, 1 ~16
UINT _set_maximum_frame_latency = 0;

// WaitForVBlank ���
bool _use_wait_for_vblank = false;

// �ݺ� ����� ������ �� �ѹ��� WaitForVBlank ���
bool _use_wait_for_vblank_repeat = false;

// data_window�� flag�� �����Ͽ� WaitForVBlank ���
bool _use_wait_for_vblank_first_entry = false;

// texture ���� ũ��
int _count_texture_store = 0;

// �ϵ���� ���ڵ� Ÿ��. 2: CUDA, 4: DXVA2, 7: D3D11VA, 12: D3D12VA
int _hw_device_type = 12;

int _control_monitor_left = 0;
int _control_monitor_top = 0;
int _control_monitor_right = 0;
int _control_monitor_bottom = 0;

// �ݺ� ��� ���
bool _use_play_repeat = false;

// scene fps. 0: 60fps, 1: 30fps
int _count_scene_fps = 0;

#pragma endregion

// ������ ��ǥ ����
std::map<uint64_t, pst_coordinate> _map_window_coordinate;

// scene ���� ����
int _count_scene = 0;
// scene url
std::map<uint64_t, std::string> _map_scene_url;
// scene ǥ�� ��ǥ
std::map<uint64_t, pst_coordinate> _map_scene_coordinate;

uint32_t _player_sync_group_index = UINT32_MAX;


bool _nvapi_initialized = false;
NvAPI_Status _nvapi_status = NVAPI_OK;
NvU32 _swap_group = 1;
NvU32 _swap_barrier = 1;

std::thread _thread_packet_processing;
bool _flag_packet_processing = true;
std::mutex _mutex_packet_processing;
std::deque<void*> _queue_packet_processing;

std::thread _thread_client;
bool _flag_thread_client = true;
void* _client = nullptr;

std::thread _thread_vector_input;
bool _flag_vector_input = true;
std::vector<st_input_object> _vector_input_object;
std::mutex _mutex_input_object;


std::vector<HANDLE> _vector_event_scene_to_upload;
std::mutex _mutex_vector_event_scene_to_upload;
std::vector<std::condition_variable*> _vector_condition_variable_scene_to_upload;
std::mutex _mutex_vector_condition_variable_scene_to_upload;
std::vector<std::mutex*> _vector_mutex_scene_to_upload;
std::mutex _mutex_vector_mutex_scene_to_upload;
std::vector<bool*> _vector_flag_scene_to_upload;
std::mutex _mutex_vector_flag_scene_to_upload;

std::vector<HANDLE> _vector_event_upload_to_device;
std::mutex _mutex_vector_event_upload_to_device;
std::vector<std::condition_variable*> _vector_condition_variable_upload_to_device;
std::mutex _mutex_vector_condition_variable_upload_to_device;
std::vector<std::mutex*> _vector_mutex_upload_to_device;
std::mutex _mutex_vector_mutex_upload_to_device;
std::vector<bool*> _vector_flag_upload_to_device;
std::mutex _mutex_vector_flag_upload_to_device;

std::vector<HANDLE> _vector_event_device_to_window;
std::mutex _mutex_vector_event_device_to_window;
std::vector<std::condition_variable*> _vector_condition_variable_device_to_window;
std::mutex _mutex_vector_condition_variable_device_to_window;
std::vector<std::mutex*> _vector_mutex_device_to_window;
std::mutex _mutex_vector_mutex_device_to_window;
std::vector<bool*> _vector_flag_device_to_window;
std::mutex _mutex_vector_flag_device_to_window;

std::vector<HANDLE> _vector_event_window_to_scene;
std::mutex _mutex_vector_event_window_to_scene;
std::vector<std::condition_variable*> _vector_condition_variable_window_to_scene;
std::mutex _mutex_vector_condition_variable_window_to_scene;
std::vector<std::mutex*> _vector_mutex_window_to_scene;
std::mutex _mutex_vector_mutex_window_to_scene;
std::vector<bool*> _vector_flag_window_to_scene;
std::mutex _mutex_vector_flag_window_to_scene;


std::thread _thread_wait_for_multiple_objects_scene_to_upload;
bool _flag_wait_for_multiple_objects_scene_to_upload = true;

std::thread _thread_wait_for_multiple_objects_upload_to_device;
bool _flag_wait_for_multiple_objects_upload_to_device = true;

std::thread _thread_wait_for_multiple_objects_device_to_window;
bool _flag_wait_for_multiple_objects_device_to_window = true;

std::thread _thread_wait_for_multiple_objects_window_to_scene;
bool _flag_wait_for_multiple_objects_window_to_scene = true;


std::map<uint64_t, std::thread*> _map_thread_scene;
std::map<uint64_t, std::thread*> _map_thread_upload;
std::map<uint64_t, std::thread*> _map_thread_device;
std::map<uint64_t, std::thread*> _map_thread_window;

std::map<uint64_t, pst_scene> _map_scene;

// --------------------------------

IDXGIFactory2* _factory = nullptr;

std::map<uint64_t, pst_adapter> _map_adapter;
std::map<uint64_t, pst_device> _map_device;
std::map<uint64_t, pst_output> _map_output;

std::map<uint64_t, pst_command_queue> _map_command_queue;
std::map<uint64_t, pst_command_allocator> _map_command_allocator;
std::map<uint64_t, pst_command_list> _map_command_list;
std::map<uint64_t, pst_root_signature> _map_root_sig;
std::map<uint64_t, pst_pso> _map_pso;
std::map<uint64_t, pst_fence> _map_fence;

std::map<uint64_t, pst_rtv_heap> _map_rtv_heap;
std::map<uint64_t, pst_rtv> _map_rtv;

std::map<uint64_t, pst_srv_heap> _map_srv_heap;

std::map<uint64_t, pst_vertex_buffer> _map_vertex_buffer;
std::mutex* _mutex_map_vertex_buffer = nullptr;
std::map<uint64_t, pst_vertex_upload_buffer> _map_vertex_upload_buffer;
std::mutex* _mutex_map_vertex_upload_buffer = nullptr;
std::map<uint64_t, pst_vertex_buffer_view> _map_vertex_buffer_view;
std::mutex* _mutex_map_vertex_buffer_view = nullptr;

std::map<uint64_t, pst_index_buffer> _map_index_buffer;
std::mutex* _mutex_map_index_buffer = nullptr;
std::map<uint64_t, pst_index_upload_buffer> _map_index_upload_buffer;
std::mutex* _mutex_map_index_upload_buffer = nullptr;
std::map<uint64_t, pst_index_buffer_view> _map_index_buffer_view;
std::mutex* _mutex_map_index_buffer_view = nullptr;

std::map<uint64_t, pst_texture> _map_texture;
std::mutex* _mutex_map_texture = nullptr;
std::map<uint64_t, pst_srv_handle> _map_srv_handle_luminance;
std::mutex* _mutex_map_srv_handle_luminance = nullptr;
std::map<uint64_t, pst_srv_handle> _map_srv_handle_chrominance;
std::mutex* _mutex_map_srv_handle_chrominance = nullptr;

std::map<uint64_t, pst_texture> _map_texture_y;
std::mutex* _mutex_map_texture_y = nullptr;
std::map<uint64_t, pst_texture> _map_texture_u;
std::mutex* _mutex_map_texture_u = nullptr;
std::map<uint64_t, pst_texture> _map_texture_v;
std::mutex* _mutex_map_texture_v = nullptr;
std::map<uint64_t, pst_upload_texture> _map_upload_texture_y;
std::mutex* _mutex_map_upload_texture_y = nullptr;
std::map<uint64_t, pst_upload_texture> _map_upload_texture_u;
std::mutex* _mutex_map_upload_texture_u = nullptr;
std::map<uint64_t, pst_upload_texture> _map_upload_texture_v;
std::mutex* _mutex_map_upload_texture_v = nullptr;
std::map<uint64_t, pst_srv_handle> _map_srv_handle_y;
std::mutex* _mutex_map_srv_handle_y = nullptr;
std::map<uint64_t, pst_srv_handle> _map_srv_handle_u;
std::mutex* _mutex_map_srv_handle_u = nullptr;
std::map<uint64_t, pst_srv_handle> _map_srv_handle_v;
std::mutex* _mutex_map_srv_handle_v = nullptr;

std::map<uint64_t, pst_window> _map_window;
std::map<uint64_t, pst_swap_chain> _map_swap_chain;
std::map<uint64_t, pst_viewport> _map_viewport;

bool _flag_wait_for_vblank_repeat = false;
bool _flag_scene_repeat = false;

std::string _logger_name;

bool _flag_repeat = false;

DWORD _wait_for_multiple_objects_wait_time = 1000;

// --------------------------------

// �� �ڵ� ��⿡ ���Ե� �Լ��� ������ �����մϴ�:
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void create_factory();
void enum_adapters();
void enum_outputs();
void create_window(WCHAR* window_class, WCHAR* title, HINSTANCE instance, RECT rect, void* data, HWND& handle);
void create_windows();
void delete_adapter_has_none_window();
void create_devices();
void create_command_queues();
void create_command_allocators();
void create_command_lists();
void create_rtv_heaps();
void create_srv_heaps();
void create_root_sigs();
void create_root_sigs_yuv();
void create_pipeline_state_objects();
void create_fences();
void create_swap_chains();
void create_rtvs();
void create_viewports();


void delete_viewports();
void delete_rtvs();
void delete_swap_chains();
void delete_srv_heaps();
void delete_rtv_heaps();
void delete_fences();
void delete_command_lists();
void delete_pipeline_state_objects();
void delete_root_sigs();
void delete_command_allocators();
void delete_command_queues();
void delete_devices();
void delete_windows();
void delete_outputs();
void delete_adapters();
void delete_factory();

void create_vertex_buffer(pst_device data_device);
void delete_vertex_buffers();
void create_index_buffer(pst_device data_device);
void delete_index_buffers();
void create_texture(pst_device data_device, uint64_t width, uint32_t height, uint64_t counter_texture);
void delete_textures();
void create_texture_yuv(pst_device data_device, uint64_t width, uint32_t height, uint64_t counter_texture);
void delete_textures_yuv();

void upload_texture(pst_device data_device, AVFrame* frame, uint64_t counter_texture, uint64_t srv_index);
void upload_texture_yuv(pst_device data_device, AVFrame* frame, uint64_t counter_texture, uint64_t srv_index);


void initialize_swap_lock(ID3D12Device* device, IDXGISwapChain1* swap_chain);
void initialize_swap_locks();
void delete_swap_lock(ID3D12Device* device, IDXGISwapChain1* swap_chain);
void delete_swap_locks();


void create_scenes();
void delete_scenes();

void callback_ffmpeg_wrapper_ptr(void* param);

#if _DEBUG
void d3d_memory_check();
#endif

void get_asset_path(wchar_t* path, u32 path_size);
std::wstring get_asset_full_path(LPCWSTR asset_name);
void config_setting();


void thread_client();

void thread_packet_processing();

void callback_ptr_client(void* data);


void thread_vector_input();

void vector_input(st_input_object data);

void thread_wait_for_multiple_objects(WaitType wait_type, bool* flag_thread);

void thread_device(pst_device data_device);
void thread_device_yuv(pst_device data_device);
void thread_upload(pst_device data_device);
void thread_upload_yuv(pst_device data_device);
void thread_window(pst_window data_window);
void thread_scene(pst_scene data_scene);

void wait_gpus_end();

float normalize_min_max(int min, int max, int target, int normalized_min, int normalized_max);
void normalize_rect(RECT base_rect, RECT target_rect, NormalizedRect& normalized_rect);

void message_processing_window();

void check_ready_to_playback();

void start_playback();

int play_repeat_instance_recreate();
int play_repeat_instance_delete();
int play_repeat_instance_create();

// --------------------------------

void set_logger();

// --------------------------------

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
#if 0
            debug_controller->SetEnableGPUBasedValidation(true);
#endif // 0

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
    uint64_t i = 0;

    for (auto it_adapter = _map_adapter.begin(); it_adapter != _map_adapter.end(); it_adapter++)
    {
        pst_adapter data_adapter = it_adapter->second;

        RECT rect{ LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN };

        uint64_t output_index = 0;

        for (auto it_output = _map_output.begin(); it_output != _map_output.end(); it_output++)
        {
            pst_output data_output = it_output->second;

            if (data_output->device_index != data_adapter->adapter_index)
            {
                continue;
            }

            if (data_output->output_desc.DesktopCoordinates.left == _control_monitor_left
                && data_output->output_desc.DesktopCoordinates.top == _control_monitor_top
                && data_output->output_desc.DesktopCoordinates.right == _control_monitor_right
                && data_output->output_desc.DesktopCoordinates.bottom == _control_monitor_bottom
                )
            {
                continue;
            }

            if (rect.left > data_output->output_desc.DesktopCoordinates.left)
            {
                rect.left = data_output->output_desc.DesktopCoordinates.left;
            }
            if (rect.top > data_output->output_desc.DesktopCoordinates.top)
            {
                rect.top = data_output->output_desc.DesktopCoordinates.top;
            }
            if (rect.right < data_output->output_desc.DesktopCoordinates.right)
            {
                rect.right = data_output->output_desc.DesktopCoordinates.right;
            }
            if (rect.bottom < data_output->output_desc.DesktopCoordinates.bottom)
            {
                rect.bottom = data_output->output_desc.DesktopCoordinates.bottom;
            }

            output_index = data_output->output_index;
        }

        HWND handle = nullptr;

        create_window(szWindowClass, szTitle, hInst, rect, nullptr, handle);

        if (handle != nullptr)
        {
            pst_window data_window = new st_window;
            data_window->handle = handle;

            data_window->event_window_to_scene = CreateEvent(NULL, FALSE, FALSE, NULL);

            data_window->condition_variable_device_to_window = new std::condition_variable();
            data_window->mutex_device_to_window = new std::mutex();

            st_input_object event_window_to_scene{};
            event_window_to_scene.object = data_window->event_window_to_scene;
            event_window_to_scene.object_type = ObjectType::object_event;
            event_window_to_scene.wait_type = WaitType::window_to_scene;

            vector_input(event_window_to_scene);


            st_input_object condition_variable_device_to_window{};
            condition_variable_device_to_window.object = data_window->condition_variable_device_to_window;
            condition_variable_device_to_window.object_type = ObjectType::object_condition_variable;
            condition_variable_device_to_window.wait_type = WaitType::device_to_window;

            st_input_object mutex_device_to_window{};
            mutex_device_to_window.object = data_window->mutex_device_to_window;
            mutex_device_to_window.object_type = ObjectType::object_mutex;
            mutex_device_to_window.wait_type = WaitType::device_to_window;

            st_input_object flag_device_to_window{};
            flag_device_to_window.object = &data_window->flag_device_to_window;
            flag_device_to_window.object_type = ObjectType::object_flag;
            flag_device_to_window.wait_type = WaitType::device_to_window;

            vector_input(condition_variable_device_to_window);
            vector_input(mutex_device_to_window);
            vector_input(flag_device_to_window);


            GetWindowRect(data_window->handle, &data_window->rect);

            data_window->device_index = data_adapter->adapter_index;
            data_window->output_index = output_index;
            data_window->window_index = i;
            _map_window.insert({ data_window->window_index, data_window });

            i++;
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
            data_device->device_index = data_adapter->adapter_index;

            NAME_D3D12_OBJECT_INDEXED(data_device->device, data_device->device_index, L"ID3D12Device");

            _map_device.insert({ data_device->device_index, data_device });


            data_device->event_device_to_window = CreateEvent(NULL, FALSE, FALSE, NULL);

            data_device->condition_variable_upload_to_device = new std::condition_variable();
            data_device->mutex_upload_to_device = new std::mutex();

            st_input_object event_device_to_window{};
            event_device_to_window.object = data_device->event_device_to_window;
            event_device_to_window.object_type = ObjectType::object_event;
            event_device_to_window.wait_type = WaitType::device_to_window;

            vector_input(event_device_to_window);


            st_input_object condition_variable_upload_to_device{};
            condition_variable_upload_to_device.object = data_device->condition_variable_upload_to_device;
            condition_variable_upload_to_device.object_type = ObjectType::object_condition_variable;
            condition_variable_upload_to_device.wait_type = WaitType::upload_to_device;

            st_input_object mutex_upload_to_device{};
            mutex_upload_to_device.object = data_device->mutex_upload_to_device;
            mutex_upload_to_device.object_type = ObjectType::object_mutex;
            mutex_upload_to_device.wait_type = WaitType::upload_to_device;

            st_input_object flag_upload_to_device{};
            flag_upload_to_device.object = &data_device->flag_upload_to_device;
            flag_upload_to_device.object_type = ObjectType::object_flag;
            flag_upload_to_device.wait_type = WaitType::upload_to_device;

            vector_input(condition_variable_upload_to_device);
            vector_input(mutex_upload_to_device);
            vector_input(flag_upload_to_device);


            data_device->event_upload_to_device = CreateEvent(NULL, FALSE, FALSE, NULL);

            data_device->condition_variable_scene_to_upload = new std::condition_variable();
            data_device->mutex_scene_to_upload = new std::mutex();

            st_input_object event_upload_to_device{};
            event_upload_to_device.object = data_device->event_upload_to_device;
            event_upload_to_device.object_type = ObjectType::object_event;
            event_upload_to_device.wait_type = WaitType::upload_to_device;

            vector_input(event_upload_to_device);


            st_input_object condition_variable_scene_to_upload{};
            condition_variable_scene_to_upload.object = data_device->condition_variable_scene_to_upload;
            condition_variable_scene_to_upload.object_type = ObjectType::object_condition_variable;
            condition_variable_scene_to_upload.wait_type = WaitType::scene_to_upload;

            st_input_object mutex_scene_to_upload{};
            mutex_scene_to_upload.object = data_device->mutex_scene_to_upload;
            mutex_scene_to_upload.object_type = ObjectType::object_mutex;
            mutex_scene_to_upload.wait_type = WaitType::scene_to_upload;

            st_input_object flag_scene_to_upload{};
            flag_scene_to_upload.object = &data_device->flag_scene_to_upload;
            flag_scene_to_upload.object_type = ObjectType::object_flag;
            flag_scene_to_upload.wait_type = WaitType::scene_to_upload;

            vector_input(condition_variable_scene_to_upload);
            vector_input(mutex_scene_to_upload);
            vector_input(flag_scene_to_upload);
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

            NAME_D3D12_OBJECT_INDEXED(data_command_queue->command_queue, data_command_queue->device_index, L"ID3D12CommandQueue");

            _map_command_queue.insert({ data_command_queue->device_index, data_command_queue });
        }
    }
}

void create_command_allocators()
{
    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        pst_command_allocator data_command_allocator = new st_command_allocator();

        for (uint32_t i = 0; i < _frame_buffer_count * 2; i++)
        {
            ID3D12CommandAllocator* command_allocator = nullptr;

            data_device->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator));

            NAME_D3D12_OBJECT_INDEXED_2(command_allocator, data_device->device_index, i, L"ID3D12CommandAllocator");

            data_command_allocator->vector_command_allocator.push_back(command_allocator);
        }

        data_command_allocator->device_index = data_device->device_index;

        _map_command_allocator.insert({ data_command_allocator->device_index, data_command_allocator });
    }
}

void create_root_sigs()
{
    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        CD3DX12_DESCRIPTOR_RANGE1 ranges[_texture_resource_count_nv12]{};
        CD3DX12_ROOT_PARAMETER1 root_parameters[_texture_resource_count_nv12]{};

        // srv 1
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
        root_parameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        // srv 2
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
        root_parameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

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

        _map_root_sig.insert({ data_root_sig->device_index, data_root_sig });
    }
}

void create_root_sigs_yuv()
{
    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        CD3DX12_DESCRIPTOR_RANGE1 ranges[_texture_resource_count_yuv]{};
        CD3DX12_ROOT_PARAMETER1 root_parameters[_texture_resource_count_yuv]{};

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

        _map_root_sig.insert({ data_root_sig->device_index, data_root_sig });
    }
}

void create_pipeline_state_objects()
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

    if (_texture_type == 0)
    {
        // NV12
        hr = D3DCompileFromFile(get_asset_full_path(L"shaders_nv12.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compile_flags, 0, &vertex_shader, nullptr);
        hr = D3DCompileFromFile(get_asset_full_path(L"shaders_nv12.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compile_flags, 0, &pixel_shader, nullptr);
    }
    else
    {
        // YUV
        hr = D3DCompileFromFile(get_asset_full_path(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compile_flags, 0, &vertex_shader, nullptr);
        hr = D3DCompileFromFile(get_asset_full_path(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compile_flags, 0, &pixel_shader, nullptr);
    }

    D3D12_INPUT_ELEMENT_DESC input_element_descs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        auto it_root_sig = _map_root_sig.find(data_device->device_index);
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

        _map_pso.insert({ data_pso->device_index, data_pso });
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

        _map_command_list.insert({ data_command_list->device_index, data_command_list });
    }
}

void create_fences()
{
    HRESULT hr = S_OK;

    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        ID3D12Fence* fence_device = nullptr;
        ID3D12Fence* fence_upload = nullptr;

        hr = data_device->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_device));
        hr = data_device->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_upload));

        NAME_D3D12_OBJECT_INDEXED(fence_device, data_device->device_index, L"ID3D12Fence_Device");
        NAME_D3D12_OBJECT_INDEXED(fence_upload, data_device->device_index, L"ID3D12Fence_Upload");

        HANDLE fence_event_device = CreateEvent(NULL, FALSE, FALSE, NULL);
        HANDLE fence_event_upload = CreateEvent(NULL, FALSE, FALSE, NULL);

        pst_fence data_fence = new st_fence();

        data_fence->fence_device = fence_device;
        data_fence->fence_upload = fence_upload;
        data_fence->fence_event_device = fence_event_device;
        data_fence->fence_event_upload = fence_event_upload;
        data_fence->device_index = data_device->device_index;

        _map_fence.insert({ data_fence->device_index, data_fence });
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
        rtv_heap_desc.NumDescriptors = _rtv_descriptor_count;
        rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        hr = data_device->device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap));

        pst_rtv_heap data_rtv_heap = new st_rtv_heap();

        data_rtv_heap->rtv_heap = rtv_heap;
        data_rtv_heap->rtv_descriptor_size = data_device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        data_rtv_heap->device_index = data_device->device_index;

        NAME_D3D12_OBJECT_INDEXED(data_rtv_heap->rtv_heap, data_rtv_heap->device_index, L"ID3D12DescriptorHeap_rtv");

        _map_rtv_heap.insert({ data_rtv_heap->device_index, data_rtv_heap });
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
        srv_heap_desc.NumDescriptors = _srv_descriptor_count;
        srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        hr = data_device->device->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&srv_heap));

        pst_srv_heap data_srv_heap = new st_srv_heap();

        data_srv_heap->srv_heap = srv_heap;
        data_srv_heap->srv_descriptor_size = data_device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        data_srv_heap->device_index = data_device->device_index;

        NAME_D3D12_OBJECT_INDEXED(data_srv_heap->srv_heap, data_srv_heap->device_index, L"ID3D12DescriptorHeap_srv");

        _map_srv_heap.insert({ data_srv_heap->device_index , data_srv_heap });
    }
}

void create_swap_chains()
{
    HRESULT hr = S_OK;

    for (auto it_window = _map_window.begin(); it_window != _map_window.end(); it_window++)
    {
        pst_window data_window = it_window->second;

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
        swap_chain_desc.BufferCount = _frame_buffer_count;
        swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        if (_set_maximum_frame_latency != 0)
        {
            swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        }
        else
        {
            swap_chain_desc.Flags = 0;
        }

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

            if (_set_maximum_frame_latency != 0)
            {
                IDXGISwapChain2* swap_chain_2 = nullptr;
                swap_chain->QueryInterface(IID_PPV_ARGS(&swap_chain_2));

                swap_chain_2->SetMaximumFrameLatency(_set_maximum_frame_latency);

                swap_chain_2->Release();
                swap_chain_2 = nullptr;
            }

            _map_swap_chain.insert({ data_swap_chain->window_index, data_swap_chain });
        }
    }
}

void create_rtvs()
{
    for (auto it_swap_chain = _map_swap_chain.begin(); it_swap_chain != _map_swap_chain.end(); it_swap_chain++)
    {
        pst_swap_chain data_swap_chain = it_swap_chain->second;

        auto it_device = _map_device.find(data_swap_chain->device_index);
        pst_device data_device = it_device->second;

        auto it_rtv_heap = _map_rtv_heap.find(data_swap_chain->device_index);
        pst_rtv_heap data_rtv_heap = it_rtv_heap->second;

        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = data_rtv_heap->rtv_heap->GetCPUDescriptorHandleForHeapStart();

        pst_rtv data_rtv = new st_rtv();

        for (size_t i = 0; i < _frame_buffer_count; i++)
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
        data_rtv->window_index = data_swap_chain->window_index;

        _map_rtv.insert({ data_rtv->device_index, data_rtv });
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

        _map_viewport.insert({ data_viewport->window_index, data_viewport });
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

void delete_rtvs()
{
    for (auto it_rtv = _map_rtv.begin(); it_rtv != _map_rtv.end();)
    {
        pst_rtv data_rtv = it_rtv->second;

        for (auto it_vector = data_rtv->vector_rtv.begin(); it_vector != data_rtv->vector_rtv.end();)
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

void delete_fences()
{
    for (auto it_fence = _map_fence.begin(); it_fence != _map_fence.end();)
    {
        pst_fence data_fence = it_fence->second;

        if (data_fence->fence_device)
        {
            data_fence->fence_device->Release();
            data_fence->fence_device = nullptr;
        }

        if (data_fence->fence_upload)
        {
            data_fence->fence_upload->Release();
            data_fence->fence_upload = nullptr;
        }

        if (data_fence->fence_event_device)
        {
            CloseHandle(data_fence->fence_event_device);
            data_fence->fence_event_device = nullptr;
        }

        if (data_fence->fence_event_upload)
        {
            CloseHandle(data_fence->fence_event_upload);
            data_fence->fence_event_upload = nullptr;
        }

        delete data_fence;
        data_fence = nullptr;

        it_fence = _map_fence.erase(it_fence);
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

void delete_pipeline_state_objects()
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

void delete_root_sigs()
{
    for (auto it_root_sig = _map_root_sig.begin(); it_root_sig != _map_root_sig.end();)
    {
        pst_root_signature data_root_sig = it_root_sig->second;

        if (data_root_sig->root_sig)
        {
            data_root_sig->root_sig->Release();
            data_root_sig->root_sig = nullptr;
        }

        delete data_root_sig;
        data_root_sig = nullptr;

        it_root_sig = _map_root_sig.erase(it_root_sig);
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

        if (data_device->event_upload_to_device)
        {
            CloseHandle(data_device->event_upload_to_device);
            data_device->event_upload_to_device = nullptr;
        }

        if (data_device->event_device_to_window)
        {
            CloseHandle(data_device->event_device_to_window);
            data_device->event_device_to_window = nullptr;
        }

        delete data_device->condition_variable_scene_to_upload;
        data_device->condition_variable_scene_to_upload = nullptr;

        delete data_device->condition_variable_upload_to_device;
        data_device->condition_variable_upload_to_device = nullptr;

        delete data_device->mutex_scene_to_upload;
        data_device->mutex_scene_to_upload = nullptr;

        delete data_device->mutex_upload_to_device;
        data_device->mutex_upload_to_device = nullptr;

        delete data_device;
        data_device = nullptr;

        it_device = _map_device.erase(it_device);
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

        if (data_window->event_window_to_scene)
        {
            CloseHandle(data_window->event_window_to_scene);
            data_window->event_window_to_scene = nullptr;
        }

        delete data_window->condition_variable_device_to_window;
        data_window->condition_variable_device_to_window = nullptr;

        delete data_window->mutex_device_to_window;
        data_window->mutex_device_to_window = nullptr;

        delete data_window;
        data_window = nullptr;

        it_window = _map_window.erase(it_window);
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

void delete_factory()
{
    if (_factory)
    {
        _factory->Release();
        _factory = nullptr;
    }
}


void create_vertex_buffer(pst_device data_device)
{
    HRESULT hr = S_OK;

    pst_vertex_buffer data_vertex_buffer = new st_vertex_buffer();
    pst_vertex_upload_buffer data_vertex_upload_buffer = new st_vertex_upload_buffer();
    pst_vertex_buffer_view data_vertex_buffer_view = new st_vertex_buffer_view();

    data_vertex_buffer->device_index = data_device->device_index;
    data_vertex_upload_buffer->device_index = data_device->device_index;
    data_vertex_buffer_view->device_index = data_device->device_index;

    int i = -1;

    for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
    {
        pst_scene data_scene = it_scene->second;

        if (data_scene->device_index != data_device->device_index)
        {
            continue;
        }

        i++;

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

        vertices[0] = { { data_scene->normal_rect.left, data_scene->normal_rect.top, 0.0f }, { 0.0f, 0.0f } };
        vertices[1] = { { data_scene->normal_rect.right, data_scene->normal_rect.top, 0.0f }, { 1.0f, 0.0f } };
        vertices[2] = { { data_scene->normal_rect.left, data_scene->normal_rect.bottom, 0.0f }, { 0.0f, 1.0f } };
        vertices[3] = { { data_scene->normal_rect.right, data_scene->normal_rect.bottom, 0.0f }, { 1.0f, 1.0f } };

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

    _mutex_map_vertex_buffer->lock();
    _map_vertex_buffer.insert({ data_vertex_buffer->device_index, data_vertex_buffer });
    _mutex_map_vertex_buffer->unlock();
    
    _mutex_map_vertex_upload_buffer->lock();
    _map_vertex_upload_buffer.insert({ data_vertex_upload_buffer->device_index, data_vertex_upload_buffer });
    _mutex_map_vertex_upload_buffer->unlock();
    
    _mutex_map_vertex_buffer_view->lock();
    _map_vertex_buffer_view.insert({ data_vertex_buffer_view->device_index, data_vertex_buffer_view });
    _mutex_map_vertex_buffer_view->unlock();
}

void delete_vertex_buffers()
{
    _mutex_map_vertex_buffer->lock();
    for (auto it_vertex_buffer = _map_vertex_buffer.begin(); it_vertex_buffer != _map_vertex_buffer.end();)
    {
        pst_vertex_buffer data_vertex_buffer = it_vertex_buffer->second;

        for (auto it_vector = data_vertex_buffer->vector_vertex_buffer.begin(); it_vector != data_vertex_buffer->vector_vertex_buffer.end();)
        {
            ID3D12Resource* resource = *it_vector;

            resource->Release();
            resource = nullptr;

            it_vector = data_vertex_buffer->vector_vertex_buffer.erase(it_vector);
        }

        delete data_vertex_buffer;
        data_vertex_buffer = nullptr;

        it_vertex_buffer = _map_vertex_buffer.erase(it_vertex_buffer);
    }
    _mutex_map_vertex_buffer->unlock();

    _mutex_map_vertex_upload_buffer->lock();
    for (auto it_vertex_upload_buffer = _map_vertex_upload_buffer.begin(); it_vertex_upload_buffer != _map_vertex_upload_buffer.end();)
    {
        pst_vertex_upload_buffer data_vertex_upload_buffer = it_vertex_upload_buffer->second;

        for (auto it_vector = data_vertex_upload_buffer->vector_vertex_upload_buffer.begin(); it_vector != data_vertex_upload_buffer->vector_vertex_upload_buffer.end();)
        {
            ID3D12Resource* resource = *it_vector;

            resource->Release();
            resource = nullptr;

            it_vector = data_vertex_upload_buffer->vector_vertex_upload_buffer.erase(it_vector);
        }

        delete data_vertex_upload_buffer;
        data_vertex_upload_buffer = nullptr;

        it_vertex_upload_buffer = _map_vertex_upload_buffer.erase(it_vertex_upload_buffer);
    }
    _mutex_map_vertex_upload_buffer->unlock();

    _mutex_map_vertex_buffer_view->lock();
    for (auto it_vertex_buffer_view = _map_vertex_buffer_view.begin(); it_vertex_buffer_view != _map_vertex_buffer_view.end();)
    {
        pst_vertex_buffer_view data_vertex_buffer_view = it_vertex_buffer_view->second;

        data_vertex_buffer_view->vector_vertex_buffer_view.clear();

        delete data_vertex_buffer_view;
        data_vertex_buffer_view = nullptr;

        it_vertex_buffer_view = _map_vertex_buffer_view.erase(it_vertex_buffer_view);
    }
    _mutex_map_vertex_buffer_view->unlock();
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
    data_index_buffer->device_index = data_device->device_index;
    
    _mutex_map_index_buffer->lock();
    _map_index_buffer.insert({ data_index_buffer->device_index, data_index_buffer });
    _mutex_map_index_buffer->unlock();

    pst_index_upload_buffer data_index_upload_buffer = new st_index_upload_buffer();
    data_index_upload_buffer->index_upload_buffer = index_upload_buffer;
    data_index_upload_buffer->device_index = data_device->device_index;
    
    _mutex_map_index_upload_buffer->lock();
    _map_index_upload_buffer.insert({ data_index_upload_buffer->device_index, data_index_upload_buffer });
    _mutex_map_index_upload_buffer->unlock();

    pst_index_buffer_view data_index_buffer_view = new st_index_buffer_view();
    data_index_buffer_view->index_buffer_view = index_buffer_view;
    data_index_buffer_view->device_index = data_device->device_index;
    
    _mutex_map_index_buffer_view->lock();
    _map_index_buffer_view.insert({ data_index_buffer_view->device_index, data_index_buffer_view });
    _mutex_map_index_buffer_view->unlock();
}

void delete_index_buffers()
{
    _mutex_map_index_buffer->lock();
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
    _mutex_map_index_buffer->unlock();

    _mutex_map_index_upload_buffer->lock();
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
    _mutex_map_index_upload_buffer->unlock();

    _mutex_map_index_buffer_view->lock();
    for (auto it_index_buffer_view = _map_index_buffer_view.begin(); it_index_buffer_view != _map_index_buffer_view.end();)
    {
        pst_index_buffer_view data_index_buffer_view = it_index_buffer_view->second;

        delete data_index_buffer_view;
        data_index_buffer_view = nullptr;

        it_index_buffer_view = _map_index_buffer_view.erase(it_index_buffer_view);
    }
    _mutex_map_index_buffer_view->unlock();
}

void create_texture(pst_device data_device, uint64_t width, uint32_t height, uint64_t counter_texture)
{
    HRESULT hr = S_OK;

    auto it_srv_heap = _map_srv_heap.find(data_device->device_index);
    pst_srv_heap data_srv_heap = it_srv_heap->second;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu(data_srv_heap->srv_heap->GetCPUDescriptorHandleForHeapStart());
    D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu(data_srv_heap->srv_heap->GetGPUDescriptorHandleForHeapStart());

    const uint32_t srv_descriptor_size = data_device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    _mutex_map_texture->lock();
    pst_texture data_texture = nullptr;
    auto it_texture = _map_texture.find(data_device->device_index);
    if (it_texture != _map_texture.end())
    {
        data_texture = it_texture->second;
    }
    else
    {
        data_texture = new st_texture();
        _map_texture.insert({ data_device->device_index, data_texture });
    }
    _mutex_map_texture->unlock();

    _mutex_map_srv_handle_luminance->lock();
    pst_srv_handle data_srv_handle_luminance = nullptr;
    auto it_srv_handle_luminance = _map_srv_handle_luminance.find(data_device->device_index);
    if (it_srv_handle_luminance != _map_srv_handle_luminance.end())
    {
        data_srv_handle_luminance = it_srv_handle_luminance->second;
    }
    else
    {
        data_srv_handle_luminance = new st_srv_handle();
        _map_srv_handle_luminance.insert({ data_device->device_index, data_srv_handle_luminance });
    }
    _mutex_map_srv_handle_luminance->unlock();
    
    _mutex_map_srv_handle_chrominance->lock();
    pst_srv_handle data_srv_handle_chrominance = nullptr;
    auto it_srv_handle_chrominance = _map_srv_handle_chrominance.find(data_device->device_index);
    if (it_srv_handle_chrominance != _map_srv_handle_chrominance.end())
    {
        data_srv_handle_chrominance = it_srv_handle_chrominance->second;
    }
    else
    {
        data_srv_handle_chrominance = new st_srv_handle();
        _map_srv_handle_chrominance.insert({ data_device->device_index, data_srv_handle_chrominance });
    }
    _mutex_map_srv_handle_chrominance->unlock();

    D3D12_RESOURCE_DESC texture_desc{};
    texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texture_desc.Alignment = 0;
    texture_desc.Width = width;
    texture_desc.Height = height;
    texture_desc.DepthOrArraySize = 1;
    texture_desc.MipLevels = 1;
    texture_desc.Format = DXGI_FORMAT_NV12;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.SampleDesc.Quality = 0;
    texture_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texture_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    CD3DX12_HEAP_PROPERTIES texture_properties(D3D12_HEAP_TYPE_DEFAULT);

    for (size_t i = (_count_texture_store * counter_texture); i < _count_texture_store * (counter_texture + 1); i++)
    {
        ID3D12Resource* texture = nullptr;
        hr = data_device->device->CreateCommittedResource(
            &texture_properties,
            D3D12_HEAP_FLAG_NONE,
            &texture_desc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr,
            IID_PPV_ARGS(&texture)
        );
        data_texture->vector_texture.push_back(texture);
        NAME_D3D12_OBJECT_INDEXED_2(texture, data_device->device_index, i, L"ID3D12Resource_texture");
    }


    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.Format = DXGI_FORMAT_R8_UNORM;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Texture2D.PlaneSlice = 0;
    srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

    srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + INT64(srv_descriptor_size * (_count_texture_store * counter_texture * _texture_resource_count_nv12)));
    srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + INT64(srv_descriptor_size * (_count_texture_store * counter_texture * _texture_resource_count_nv12)));

    for (size_t i = (_count_texture_store * counter_texture); i < _count_texture_store * (counter_texture + 1); i++)
    {
        ID3D12Resource* texture = data_texture->vector_texture.at(i);

        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_luminance = srv_handle_cpu;
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + INT64(srv_descriptor_size));
        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_chrominance = srv_handle_cpu;
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + INT64(srv_descriptor_size));

        srv_desc.Format = DXGI_FORMAT_R8_UNORM;
        srv_desc.Texture2D.PlaneSlice = 0;
        data_device->device->CreateShaderResourceView(texture, &srv_desc, srv_handle_cpu_luminance);
        data_srv_handle_luminance->vector_handle_cpu.push_back(srv_handle_cpu_luminance);

        srv_desc.Format = DXGI_FORMAT_R8G8_UNORM;
        srv_desc.Texture2D.PlaneSlice = 1;
        data_device->device->CreateShaderResourceView(texture, &srv_desc, srv_handle_cpu_chrominance);
        data_srv_handle_chrominance->vector_handle_cpu.push_back(srv_handle_cpu_chrominance);


        D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_luminance = srv_handle_gpu;
        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + INT64(srv_descriptor_size));
        data_srv_handle_luminance->vector_handle_gpu.push_back(srv_handle_gpu_luminance);

        D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_chrominance = srv_handle_gpu;
        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + INT64(srv_descriptor_size));
        data_srv_handle_chrominance->vector_handle_gpu.push_back(srv_handle_gpu_chrominance);
    }
}

void delete_textures()
{
    _mutex_map_texture->lock();
    for (auto it_texture = _map_texture.begin(); it_texture != _map_texture.end();)
    {
        pst_texture data_texture = it_texture->second;

        for (auto it_vector = data_texture->vector_texture.begin(); it_vector != data_texture->vector_texture.end();)
        {
            ID3D12Resource* resource = *it_vector;

            resource->Release();
            resource = nullptr;

            it_vector = data_texture->vector_texture.erase(it_vector);
        }

        delete data_texture;
        data_texture = nullptr;

        it_texture = _map_texture.erase(it_texture);
    }
    _mutex_map_texture->unlock();

    _mutex_map_srv_handle_luminance->lock();
    for (auto it_srv_handle_luminance = _map_srv_handle_luminance.begin(); it_srv_handle_luminance != _map_srv_handle_luminance.end();)
    {
        pst_srv_handle data_srv_handle = it_srv_handle_luminance->second;

        data_srv_handle->vector_handle_cpu.clear();
        data_srv_handle->vector_handle_gpu.clear();

        delete data_srv_handle;
        data_srv_handle = nullptr;

        it_srv_handle_luminance = _map_srv_handle_luminance.erase(it_srv_handle_luminance);
    }
    _mutex_map_srv_handle_luminance->unlock();
    
    _mutex_map_srv_handle_chrominance->lock();
    for (auto it_srv_handle_chrominance = _map_srv_handle_chrominance.begin(); it_srv_handle_chrominance != _map_srv_handle_chrominance.end();)
    {
        pst_srv_handle data_srv_handle = it_srv_handle_chrominance->second;

        data_srv_handle->vector_handle_cpu.clear();
        data_srv_handle->vector_handle_gpu.clear();

        delete data_srv_handle;
        data_srv_handle = nullptr;

        it_srv_handle_chrominance = _map_srv_handle_chrominance.erase(it_srv_handle_chrominance);
    }
    _mutex_map_srv_handle_chrominance->unlock();
}

void create_texture_yuv(pst_device data_device, uint64_t width, uint32_t height, uint64_t counter_texture)
{
    HRESULT hr = S_OK;

    auto it_srv_heap = _map_srv_heap.find(data_device->device_index);
    pst_srv_heap data_srv_heap = it_srv_heap->second;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu(data_srv_heap->srv_heap->GetCPUDescriptorHandleForHeapStart());
    D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu(data_srv_heap->srv_heap->GetGPUDescriptorHandleForHeapStart());

    const uint32_t srv_descriptor_size = data_device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    _mutex_map_texture_y->lock();
    pst_texture data_texture_y = nullptr;
    auto it_texture_y = _map_texture_y.find(data_device->device_index);
    if (it_texture_y != _map_texture_y.end())
    {
        data_texture_y = it_texture_y->second;
    }
    else
    {
        data_texture_y = new st_texture();
        _map_texture_y.insert({ data_device->device_index, data_texture_y });
    }
    _mutex_map_texture_y->unlock();
    _mutex_map_texture_u->lock();
    pst_texture data_texture_u = nullptr;
    auto it_texture_u = _map_texture_u.find(data_device->device_index);
    if (it_texture_u != _map_texture_u.end())
    {
        data_texture_u = it_texture_u->second;
    }
    else
    {
        data_texture_u = new st_texture();
        _map_texture_u.insert({ data_device->device_index, data_texture_u });
    }
    _mutex_map_texture_u->unlock();
    _mutex_map_texture_v->lock();
    pst_texture data_texture_v = nullptr;
    auto it_texture_v = _map_texture_v.find(data_device->device_index);
    if (it_texture_v != _map_texture_v.end())
    {
        data_texture_v = it_texture_v->second;
    }
    else
    {
        data_texture_v = new st_texture();
        _map_texture_v.insert({ data_device->device_index, data_texture_v });
    }
    _mutex_map_texture_v->unlock();

    _mutex_map_upload_texture_y->lock();
    pst_upload_texture data_upload_texture_y = nullptr;
    auto it_upload_texture_y = _map_upload_texture_y.find(data_device->device_index);
    if (it_upload_texture_y != _map_upload_texture_y.end())
    {
        data_upload_texture_y = it_upload_texture_y->second;
    }
    else
    {
        data_upload_texture_y = new st_upload_texture();
        _map_upload_texture_y.insert({ data_device->device_index, data_upload_texture_y });
    }
    _mutex_map_upload_texture_y->unlock();
    _mutex_map_upload_texture_u->lock();
    pst_upload_texture data_upload_texture_u = nullptr;
    auto it_upload_texture_u = _map_upload_texture_u.find(data_device->device_index);
    if (it_upload_texture_u != _map_upload_texture_u.end())
    {
        data_upload_texture_u = it_upload_texture_u->second;
    }
    else
    {
        data_upload_texture_u = new st_upload_texture();
        _map_upload_texture_u.insert({ data_device->device_index, data_upload_texture_u });
    }
    _mutex_map_upload_texture_u->unlock();
    _mutex_map_upload_texture_v->lock();
    pst_upload_texture data_upload_texture_v = nullptr;
    auto it_upload_texture_v = _map_upload_texture_v.find(data_device->device_index);
    if (it_upload_texture_v != _map_upload_texture_v.end())
    {
        data_upload_texture_v = it_upload_texture_v->second;
    }
    else
    {
        data_upload_texture_v = new st_upload_texture();
        _map_upload_texture_v.insert({ data_device->device_index, data_upload_texture_v });
    }
    _mutex_map_upload_texture_v->unlock();

    _mutex_map_srv_handle_y->lock();
    pst_srv_handle data_srv_handle_y = nullptr;
    auto it_srv_handle_y = _map_srv_handle_y.find(data_device->device_index);
    if (it_srv_handle_y != _map_srv_handle_y.end())
    {
        data_srv_handle_y = it_srv_handle_y->second;
    }
    else
    {
        data_srv_handle_y = new st_srv_handle();
        _map_srv_handle_y.insert({ data_device->device_index, data_srv_handle_y });
    }
    _mutex_map_srv_handle_y->unlock();
    _mutex_map_srv_handle_u->lock();
    pst_srv_handle data_srv_handle_u = nullptr;
    auto it_srv_handle_u = _map_srv_handle_u.find(data_device->device_index);
    if (it_srv_handle_u != _map_srv_handle_u.end())
    {
        data_srv_handle_u = it_srv_handle_u->second;
    }
    else
    {
        data_srv_handle_u = new st_srv_handle();
        _map_srv_handle_u.insert({ data_device->device_index, data_srv_handle_u });
    }
    _mutex_map_srv_handle_u->unlock();
    _mutex_map_srv_handle_v->lock();
    pst_srv_handle data_srv_handle_v = nullptr;
    auto it_srv_handle_v = _map_srv_handle_v.find(data_device->device_index);
    if (it_srv_handle_v != _map_srv_handle_v.end())
    {
        data_srv_handle_v = it_srv_handle_v->second;
    }
    else
    {
        data_srv_handle_v = new st_srv_handle();
        _map_srv_handle_v.insert({ data_device->device_index, data_srv_handle_v });
    }
    _mutex_map_srv_handle_v->unlock();

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

    for (size_t i = (_count_texture_store * counter_texture); i < _count_texture_store * (counter_texture + 1); i++)
    {
        ID3D12Resource* texture_y = nullptr;
        hr = data_device->device->CreateCommittedResource(
            &texture_properties,
            D3D12_HEAP_FLAG_NONE,
            &texture_desc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr,
            IID_PPV_ARGS(&texture_y)
        );
        data_texture_y->vector_texture.push_back(texture_y);
        NAME_D3D12_OBJECT_INDEXED_2(texture_y, data_device->device_index, i, L"ID3D12Resource_texture_y");
    }

    texture_desc.Width /= 2;
    texture_desc.Height /= 2;

    for (size_t i = (_count_texture_store * counter_texture); i < _count_texture_store * (counter_texture + 1); i++)
    {
        ID3D12Resource* texture_u = nullptr;
        hr = data_device->device->CreateCommittedResource(
            &texture_properties,
            D3D12_HEAP_FLAG_NONE,
            &texture_desc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr,
            IID_PPV_ARGS(&texture_u)
        );
        data_texture_u->vector_texture.push_back(texture_u);
        NAME_D3D12_OBJECT_INDEXED_2(texture_u, data_device->device_index, i, L"ID3D12Resource_texture_u");
    }
    for (size_t i = (_count_texture_store * counter_texture); i < _count_texture_store * (counter_texture + 1); i++)
    {
        ID3D12Resource* texture_v = nullptr;
        hr = data_device->device->CreateCommittedResource(
            &texture_properties,
            D3D12_HEAP_FLAG_NONE,
            &texture_desc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr,
            IID_PPV_ARGS(&texture_v)
        );
        data_texture_v->vector_texture.push_back(texture_v);
        NAME_D3D12_OBJECT_INDEXED_2(texture_v, data_device->device_index, i, L"ID3D12Resource_texture_v");
    }


    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout_y{};
    UINT numRows_y = 0;
    UINT64 rowSizeInBytes_y = 0;
    UINT64 totalBytes_y = 0;

    texture_desc.Format = DXGI_FORMAT_R8_TYPELESS;
    texture_desc.Width = width;
    texture_desc.Height = height;

    data_device->device->GetCopyableFootprints(&texture_desc, 0, 1, 0, &layout_y, &numRows_y, &rowSizeInBytes_y, &totalBytes_y);

    D3D12_RESOURCE_DESC upload_desc{};
    upload_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    upload_desc.Alignment = 0;
    upload_desc.Width = totalBytes_y;
    upload_desc.Height = 1;
    upload_desc.DepthOrArraySize = 1;
    upload_desc.MipLevels = 1;
    upload_desc.Format = DXGI_FORMAT_UNKNOWN;
    upload_desc.SampleDesc.Count = 1;
    upload_desc.SampleDesc.Quality = 0;
    upload_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    upload_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    CD3DX12_HEAP_PROPERTIES texture_upload_heap_properties(D3D12_HEAP_TYPE_UPLOAD);

    data_upload_texture_y->layout = layout_y;
    data_upload_texture_y->numRows = numRows_y;
    data_upload_texture_y->rowSizeInBytes = rowSizeInBytes_y;
    data_upload_texture_y->totalBytes = totalBytes_y;

    for (size_t i = (_count_texture_store * counter_texture); i < _count_texture_store * (counter_texture + 1); i++)
    {
        ID3D12Resource* upload_texture_y = nullptr;
        hr = data_device->device->CreateCommittedResource(
            &texture_upload_heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &upload_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&upload_texture_y)
        );
        data_upload_texture_y->vector_texture.push_back(upload_texture_y);

        NAME_D3D12_OBJECT_INDEXED_2(upload_texture_y, data_device->device_index, i, L"ID3D12Resource_upload_texture_y");
    }


    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout_u{};
    UINT numRows_u = 0;
    UINT64 rowSizeInBytes_u = 0;
    UINT64 totalBytes_u = 0;

    texture_desc.Width /= 2;
    texture_desc.Height /= 2;

    data_device->device->GetCopyableFootprints(&texture_desc, 0, 1, 0, &layout_u, &numRows_u, &rowSizeInBytes_u, &totalBytes_u);

    upload_desc.Width = totalBytes_u;

    data_upload_texture_u->layout = layout_u;
    data_upload_texture_u->numRows = numRows_u;
    data_upload_texture_u->rowSizeInBytes = rowSizeInBytes_u;
    data_upload_texture_u->totalBytes = totalBytes_u;

    for (size_t i = (_count_texture_store * counter_texture); i < _count_texture_store * (counter_texture + 1); i++)
    {
        ID3D12Resource* upload_texture_u = nullptr;
        hr = data_device->device->CreateCommittedResource(
            &texture_upload_heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &upload_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&upload_texture_u)
        );
        data_upload_texture_u->vector_texture.push_back(upload_texture_u);

        NAME_D3D12_OBJECT_INDEXED_2(upload_texture_u, data_device->device_index, i, L"ID3D12Resource_upload_texture_u");
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout_v{};
    UINT numRows_v = 0;
    UINT64 rowSizeInBytes_v = 0;
    UINT64 totalBytes_v = 0;

    data_device->device->GetCopyableFootprints(&texture_desc, 0, 1, 0, &layout_v, &numRows_v, &rowSizeInBytes_v, &totalBytes_v);

    upload_desc.Width = totalBytes_v;

    data_upload_texture_v->layout = layout_v;
    data_upload_texture_v->numRows = numRows_v;
    data_upload_texture_v->rowSizeInBytes = rowSizeInBytes_v;
    data_upload_texture_v->totalBytes = totalBytes_v;

    for (size_t i = (_count_texture_store * counter_texture); i < _count_texture_store * (counter_texture + 1); i++)
    {
        ID3D12Resource* upload_texture_v = nullptr;
        hr = data_device->device->CreateCommittedResource(
            &texture_upload_heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &upload_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&upload_texture_v)
        );
        data_upload_texture_v->vector_texture.push_back(upload_texture_v);

        NAME_D3D12_OBJECT_INDEXED_2(upload_texture_v, data_device->device_index, i, L"ID3D12Resource_upload_texture_v");
    }


    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.Format = DXGI_FORMAT_R8_UNORM;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Texture2D.PlaneSlice = 0;
    srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

    srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + INT64(srv_descriptor_size * (_count_texture_store * counter_texture * _texture_resource_count_yuv)));
    srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + INT64(srv_descriptor_size * (_count_texture_store * counter_texture * _texture_resource_count_yuv)));

    for (size_t i = (_count_texture_store * counter_texture); i < _count_texture_store * (counter_texture + 1); i++)
    {
        ID3D12Resource* texture_y = data_texture_y->vector_texture.at(i);
        ID3D12Resource* texture_u = data_texture_u->vector_texture.at(i);
        ID3D12Resource* texture_v = data_texture_v->vector_texture.at(i);

        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_y = srv_handle_cpu;
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + INT64(srv_descriptor_size));
        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_u = srv_handle_cpu;
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + INT64(srv_descriptor_size));
        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_v = srv_handle_cpu;
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + INT64(srv_descriptor_size));

        data_device->device->CreateShaderResourceView(texture_y, &srv_desc, srv_handle_cpu_y);
        data_srv_handle_y->vector_handle_cpu.push_back(srv_handle_cpu_y);
        data_device->device->CreateShaderResourceView(texture_u, &srv_desc, srv_handle_cpu_u);
        data_srv_handle_u->vector_handle_cpu.push_back(srv_handle_cpu_u);
        data_device->device->CreateShaderResourceView(texture_v, &srv_desc, srv_handle_cpu_v);
        data_srv_handle_v->vector_handle_cpu.push_back(srv_handle_cpu_v);


        D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_y = srv_handle_gpu;
        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + INT64(srv_descriptor_size));
        data_srv_handle_y->vector_handle_gpu.push_back(srv_handle_gpu_y);

        D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_u = srv_handle_gpu;
        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + INT64(srv_descriptor_size));
        data_srv_handle_u->vector_handle_gpu.push_back(srv_handle_gpu_u);

        D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_v = srv_handle_gpu;
        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + INT64(srv_descriptor_size));
        data_srv_handle_v->vector_handle_gpu.push_back(srv_handle_gpu_v);
    }
}

void delete_textures_yuv()
{
    _mutex_map_texture_y->lock();
    for (auto it_texture_y = _map_texture_y.begin(); it_texture_y != _map_texture_y.end();)
    {
        pst_texture data_texture_y = it_texture_y->second;

        for (auto it_vector = data_texture_y->vector_texture.begin(); it_vector != data_texture_y->vector_texture.end();)
        {
            ID3D12Resource* resource = *it_vector;

            resource->Release();
            resource = nullptr;

            it_vector = data_texture_y->vector_texture.erase(it_vector);
        }

        delete data_texture_y;
        data_texture_y = nullptr;

        it_texture_y = _map_texture_y.erase(it_texture_y);
    }
    _mutex_map_texture_y->unlock();
    _mutex_map_texture_u->lock();
    for (auto it_texture_u = _map_texture_u.begin(); it_texture_u != _map_texture_u.end();)
    {
        pst_texture data_texture_u = it_texture_u->second;

        for (auto it_vector = data_texture_u->vector_texture.begin(); it_vector != data_texture_u->vector_texture.end();)
        {
            ID3D12Resource* resource = *it_vector;

            resource->Release();
            resource = nullptr;

            it_vector = data_texture_u->vector_texture.erase(it_vector);
        }

        delete data_texture_u;
        data_texture_u = nullptr;

        it_texture_u = _map_texture_u.erase(it_texture_u);
    }
    _mutex_map_texture_u->unlock();
    _mutex_map_texture_v->lock();
    for (auto it_texture_v = _map_texture_v.begin(); it_texture_v != _map_texture_v.end();)
    {
        pst_texture data_texture_v = it_texture_v->second;

        for (auto it_vector = data_texture_v->vector_texture.begin(); it_vector != data_texture_v->vector_texture.end();)
        {
            ID3D12Resource* resource = *it_vector;

            resource->Release();
            resource = nullptr;

            it_vector = data_texture_v->vector_texture.erase(it_vector);
        }

        delete data_texture_v;
        data_texture_v = nullptr;

        it_texture_v = _map_texture_v.erase(it_texture_v);
    }
    _mutex_map_texture_v->unlock();

    _mutex_map_upload_texture_y->lock();
    for (auto it_upload_texture_y = _map_upload_texture_y.begin(); it_upload_texture_y != _map_upload_texture_y.end();)
    {
        pst_upload_texture data_upload_texture_y = it_upload_texture_y->second;

        for (auto it_vector = data_upload_texture_y->vector_texture.begin(); it_vector != data_upload_texture_y->vector_texture.end();)
        {
            ID3D12Resource* resource = *it_vector;

            resource->Release();
            resource = nullptr;

            it_vector = data_upload_texture_y->vector_texture.erase(it_vector);
        }

        delete data_upload_texture_y;
        data_upload_texture_y = nullptr;

        it_upload_texture_y = _map_upload_texture_y.erase(it_upload_texture_y);
    }
    _mutex_map_upload_texture_y->unlock();
    _mutex_map_upload_texture_u->lock();
    for (auto it_upload_texture_u = _map_upload_texture_u.begin(); it_upload_texture_u != _map_upload_texture_u.end();)
    {
        pst_upload_texture data_upload_texture_u = it_upload_texture_u->second;

        for (auto it_vector = data_upload_texture_u->vector_texture.begin(); it_vector != data_upload_texture_u->vector_texture.end();)
        {
            ID3D12Resource* resource = *it_vector;

            resource->Release();
            resource = nullptr;

            it_vector = data_upload_texture_u->vector_texture.erase(it_vector);
        }

        delete data_upload_texture_u;
        data_upload_texture_u = nullptr;

        it_upload_texture_u = _map_upload_texture_u.erase(it_upload_texture_u);
    }
    _mutex_map_upload_texture_u->unlock();
    _mutex_map_upload_texture_v->lock();
    for (auto it_upload_texture_v = _map_upload_texture_v.begin(); it_upload_texture_v != _map_upload_texture_v.end();)
    {
        pst_upload_texture data_upload_texture_v = it_upload_texture_v->second;

        for (auto it_vector = data_upload_texture_v->vector_texture.begin(); it_vector != data_upload_texture_v->vector_texture.end();)
        {
            ID3D12Resource* resource = *it_vector;

            resource->Release();
            resource = nullptr;

            it_vector = data_upload_texture_v->vector_texture.erase(it_vector);
        }

        delete data_upload_texture_v;
        data_upload_texture_v = nullptr;

        it_upload_texture_v = _map_upload_texture_v.erase(it_upload_texture_v);
    }
    _mutex_map_upload_texture_v->unlock();

    _mutex_map_srv_handle_y->lock();
    for (auto it_srv_handle_y = _map_srv_handle_y.begin(); it_srv_handle_y != _map_srv_handle_y.end();)
    {
        pst_srv_handle data_srv_handle = it_srv_handle_y->second;

        data_srv_handle->vector_handle_cpu.clear();
        data_srv_handle->vector_handle_gpu.clear();

        delete data_srv_handle;
        data_srv_handle = nullptr;

        it_srv_handle_y = _map_srv_handle_y.erase(it_srv_handle_y);
    }
    _mutex_map_srv_handle_y->unlock();
    _mutex_map_srv_handle_u->lock();
    for (auto it_srv_handle_u = _map_srv_handle_u.begin(); it_srv_handle_u != _map_srv_handle_u.end();)
    {
        pst_srv_handle data_srv_handle = it_srv_handle_u->second;

        data_srv_handle->vector_handle_cpu.clear();
        data_srv_handle->vector_handle_gpu.clear();

        delete data_srv_handle;
        data_srv_handle = nullptr;

        it_srv_handle_u = _map_srv_handle_u.erase(it_srv_handle_u);
    }
    _mutex_map_srv_handle_u->unlock();
    _mutex_map_srv_handle_v->lock();
    for (auto it_srv_handle_v = _map_srv_handle_v.begin(); it_srv_handle_v != _map_srv_handle_v.end();)
    {
        pst_srv_handle data_srv_handle = it_srv_handle_v->second;

        data_srv_handle->vector_handle_cpu.clear();
        data_srv_handle->vector_handle_gpu.clear();

        delete data_srv_handle;
        data_srv_handle = nullptr;

        it_srv_handle_v = _map_srv_handle_v.erase(it_srv_handle_v);
    }
    _mutex_map_srv_handle_v->unlock();
}

void upload_texture(pst_device data_device, AVFrame* frame, uint64_t counter_texture, uint64_t srv_index)
{
    //auto it_command_list = _map_command_list.find(data_device->device_index);
    //pst_command_list data_command_list = it_command_list->second;
    //auto it_texture = _map_texture.find(data_device->device_index);
    //pst_texture data_texture = it_texture->second;

    uint64_t texture_index = (_count_texture_store * counter_texture) + srv_index;

    ID3D12Resource* srcResource = ((AVD3D12VAFrame*)frame->data[0])->texture;

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.Format = DXGI_FORMAT_R8_UNORM;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Texture2D.PlaneSlice = 0;
    srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

    _mutex_map_srv_handle_luminance->lock();
    auto it_srv_handle_luminance = _map_srv_handle_luminance.find(data_device->device_index);
    _mutex_map_srv_handle_luminance->unlock();
    
    pst_srv_handle data_srv_handle_luminance = it_srv_handle_luminance->second;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_luminance = data_srv_handle_luminance->vector_handle_cpu.at(texture_index);

    _mutex_map_srv_handle_chrominance->lock();
    auto it_srv_handle_chrominance = _map_srv_handle_chrominance.find(data_device->device_index);
    _mutex_map_srv_handle_chrominance->unlock();
    
    pst_srv_handle data_srv_handle_chrominance = it_srv_handle_chrominance->second;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_chrominance = data_srv_handle_chrominance->vector_handle_cpu.at(texture_index);

    srv_desc.Format = DXGI_FORMAT_R8_UNORM;
    srv_desc.Texture2D.PlaneSlice = 0;
    data_device->device->CreateShaderResourceView(srcResource, &srv_desc, srv_handle_cpu_luminance);

    srv_desc.Format = DXGI_FORMAT_R8G8_UNORM;
    srv_desc.Texture2D.PlaneSlice = 1;
    data_device->device->CreateShaderResourceView(srcResource, &srv_desc, srv_handle_cpu_chrominance);
}

void upload_texture_yuv(pst_device data_device, AVFrame* frame, uint64_t counter_texture, uint64_t srv_index)
{
    auto it_command_list = _map_command_list.find(data_device->device_index);
    pst_command_list data_command_list = it_command_list->second;

    _mutex_map_texture_y->lock();
    auto it_texture_y = _map_texture_y.find(data_device->device_index);
    pst_texture data_texture_y = it_texture_y->second;
    _mutex_map_texture_y->unlock();
    _mutex_map_texture_u->lock();
    auto it_texture_u = _map_texture_u.find(data_device->device_index);
    pst_texture data_texture_u = it_texture_u->second;
    _mutex_map_texture_u->unlock();
    _mutex_map_texture_v->lock();
    auto it_texture_v = _map_texture_v.find(data_device->device_index);
    pst_texture data_texture_v = it_texture_v->second;
    _mutex_map_texture_v->unlock();

    _mutex_map_upload_texture_y->lock();
    auto it_upload_texture_y = _map_upload_texture_y.find(data_device->device_index);
    pst_upload_texture data_upload_texture_y = it_upload_texture_y->second;
    _mutex_map_upload_texture_y->unlock();
    _mutex_map_upload_texture_u->lock();
    auto it_upload_texture_u = _map_upload_texture_u.find(data_device->device_index);
    pst_upload_texture data_upload_texture_u = it_upload_texture_u->second;
    _mutex_map_upload_texture_u->unlock();
    _mutex_map_upload_texture_v->lock();
    auto it_upload_texture_v = _map_upload_texture_v.find(data_device->device_index);
    pst_upload_texture data_upload_texture_v = it_upload_texture_v->second;
    _mutex_map_upload_texture_v->unlock();

    uint64_t texture_index = (_count_texture_store * counter_texture) + srv_index;

    ID3D12Resource* texture_y = nullptr;
    ID3D12Resource* texture_u = nullptr;
    ID3D12Resource* texture_v = nullptr;
    ID3D12Resource* upload_texture_y = nullptr;
    ID3D12Resource* upload_texture_u = nullptr;
    ID3D12Resource* upload_texture_v = nullptr;

    texture_y = data_texture_y->vector_texture.at(texture_index);
    texture_u = data_texture_u->vector_texture.at(texture_index);
    texture_v = data_texture_v->vector_texture.at(texture_index);
    upload_texture_y = data_upload_texture_y->vector_texture.at(texture_index);
    upload_texture_u = data_upload_texture_u->vector_texture.at(texture_index);
    upload_texture_v = data_upload_texture_v->vector_texture.at(texture_index);

    D3D12_SUBRESOURCE_DATA texture_data_y{};
    texture_data_y.pData = frame->data[0];
    texture_data_y.RowPitch = frame->linesize[0];
    texture_data_y.SlicePitch = texture_data_y.RowPitch * frame->height;
    D3D12_SUBRESOURCE_DATA texture_data_u{};
    texture_data_u.pData = frame->data[1];
    texture_data_u.RowPitch = frame->linesize[1];
    texture_data_u.SlicePitch = texture_data_u.RowPitch * frame->height / 2;
    D3D12_SUBRESOURCE_DATA texture_data_v{};
    texture_data_v.pData = frame->data[2];
    texture_data_v.RowPitch = frame->linesize[2];
    texture_data_v.SlicePitch = texture_data_v.RowPitch * frame->height / 2;

    CD3DX12_RESOURCE_BARRIER transition_barrier_y = CD3DX12_RESOURCE_BARRIER::Transition(texture_y, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    data_command_list->command_list->ResourceBarrier(1, &transition_barrier_y);

    UpdateSubresources(data_command_list->command_list, texture_y, upload_texture_y, 0, 0, 1, &texture_data_y);

    transition_barrier_y = CD3DX12_RESOURCE_BARRIER::Transition(texture_y, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    data_command_list->command_list->ResourceBarrier(1, &transition_barrier_y);


    CD3DX12_RESOURCE_BARRIER transition_barrier_u = CD3DX12_RESOURCE_BARRIER::Transition(texture_u, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    data_command_list->command_list->ResourceBarrier(1, &transition_barrier_u);

    UpdateSubresources(data_command_list->command_list, texture_u, upload_texture_u, 0, 0, 1, &texture_data_u);

    transition_barrier_u = CD3DX12_RESOURCE_BARRIER::Transition(texture_u, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    data_command_list->command_list->ResourceBarrier(1, &transition_barrier_u);


    CD3DX12_RESOURCE_BARRIER transition_barrier_v = CD3DX12_RESOURCE_BARRIER::Transition(texture_v, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    data_command_list->command_list->ResourceBarrier(1, &transition_barrier_v);

    UpdateSubresources(data_command_list->command_list, texture_v, upload_texture_v, 0, 0, 1, &texture_data_v);

    transition_barrier_v = CD3DX12_RESOURCE_BARRIER::Transition(texture_v, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    data_command_list->command_list->ResourceBarrier(1, &transition_barrier_v);


    av_frame_unref(frame);
}


void initialize_swap_lock(ID3D12Device* device, IDXGISwapChain1* swap_chain)
{
    NvU32 max_group = 0;
    NvU32 max_barrier = 0;

    NvAPI_Status nvapi_status = NVAPI_OK;

    nvapi_status = NvAPI_D3D1x_QueryMaxSwapGroup(device, &max_group, &max_barrier);
    if (_flag_set_logger)
    {
        std::string str = "";
        str.append("thread_main, device index = ");
        str.append(", NvAPI_D3D1x_QueryMaxSwapGroup, ");
        str.append(std::to_string(av_gettime_relative()));
        str.append(", max_group = ");
        str.append(std::to_string(max_group));
        str.append(", max_barrier = ");
        str.append(std::to_string(max_barrier));

        auto logger = spdlog::get(_logger_name.c_str());

        if (nvapi_status == NVAPI_OK)
        {
            logger->debug(str.c_str());
        }
        else
        {
            logger->warn(str.c_str());
        }
    }

    if (!(max_group > 0 && max_barrier > 0))
    {
        return;
    }

    nvapi_status = NvAPI_D3D1x_JoinSwapGroup(device, swap_chain, _swap_group, _block_swap_group_present);
    if (_flag_set_logger)
    {
        std::string str = "";
        str.append("thread_main, device index = ");
        str.append(", NvAPI_D3D1x_JoinSwapGroup, ");
        str.append(std::to_string(av_gettime_relative()));
        str.append(", nvapi_status = ");
        str.append(std::to_string(nvapi_status));

        auto logger = spdlog::get(_logger_name.c_str());
        if (nvapi_status == NVAPI_OK)
        {
            logger->debug(str.c_str());
        }
        else
        {
            logger->warn(str.c_str());
        }
    }

    nvapi_status = NvAPI_D3D1x_BindSwapBarrier(device, _swap_group, _swap_barrier);
    if (_flag_set_logger)
    {
        std::string str = "";
        str.append("thread_main, device index = ");
        str.append(", NvAPI_D3D1x_BindSwapBarrier, ");
        str.append(std::to_string(av_gettime_relative()));
        str.append(", nvapi_status = ");
        str.append(std::to_string(nvapi_status));

        auto logger = spdlog::get(_logger_name.c_str());

        if (nvapi_status == NVAPI_OK)
        {
            logger->debug(str.c_str());
        }
        else
        {
            logger->warn(str.c_str());
        }
    }
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

        delete_swap_lock(data_device->device, data_swap_chain->swap_chain);

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

        //data_swap_chain->swap_chain = nullptr;
        data_device->device = nullptr;
    }
}


void create_scenes()
{
    for (auto it_scene_url = _map_scene_url.begin(); it_scene_url != _map_scene_url.end(); it_scene_url++)
    {
        uint64_t scene_index = it_scene_url->first;

        std::string url = it_scene_url->second;

        auto it_scene_coordinate = _map_scene_coordinate.find(scene_index);
        pst_coordinate data_scene_coordinate = it_scene_coordinate->second;
        int left = data_scene_coordinate->left;
        int top = data_scene_coordinate->top;
        int width = data_scene_coordinate->width;
        int height = data_scene_coordinate->height;

        RECT rect{ 0, 0, 0, 0 };
        rect.left = left;
        rect.top = top;
        rect.right = left + width;
        rect.bottom = top + height;

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

        void* instance = cpp_ffmpeg_wrapper_create();
        cpp_ffmpeg_wrapper_initialize(instance, callback_ffmpeg_wrapper_ptr);

        if (_texture_type == 0)
        {
            // NV12
            cpp_ffmpeg_wrapper_set_hw_decode(instance);
            cpp_ffmpeg_wrapper_set_hw_device_type(instance, _hw_device_type);
            cpp_ffmpeg_wrapper_set_hw_decode_adapter_index(instance, device_index);

            cpp_ffmpeg_wrapper_set_scale(instance, false);
        }
        else
        {
            // YUV
            cpp_ffmpeg_wrapper_set_scale(instance, true);
        }

        cpp_ffmpeg_wrapper_set_file_path(instance, (char*)url.c_str());
        if (cpp_ffmpeg_wrapper_open_file(instance) != 0)
        {
            cpp_ffmpeg_wrapper_shutdown(instance);
            cpp_ffmpeg_wrapper_delete(instance);

            continue;
        }


        cpp_ffmpeg_wrapper_set_scene_index(instance, scene_index);
        cpp_ffmpeg_wrapper_set_rect(instance, rect);
        cpp_ffmpeg_wrapper_play_start(instance, nullptr);


        pst_scene data_scene = new st_scene();
        data_scene->ffmpeg_instance = instance;

        data_scene->url = url;
        data_scene->rect = rect;

        data_scene->scene_index = scene_index;
        data_scene->device_index = device_index;
        data_scene->output_index = output_index;

        data_scene->event_scene_to_upload = CreateEvent(NULL, FALSE, FALSE, NULL);

        data_scene->condition_variable_window_to_scene = new std::condition_variable();
        data_scene->mutex_window_to_scene = new std::mutex();


        st_input_object event_scene_to_upload{};
        event_scene_to_upload.object = data_scene->event_scene_to_upload;
        event_scene_to_upload.object_type = ObjectType::object_event;
        event_scene_to_upload.wait_type = WaitType::scene_to_upload;

        vector_input(event_scene_to_upload);


        st_input_object condition_variable_window_to_scene{};
        condition_variable_window_to_scene.object = data_scene->condition_variable_window_to_scene;
        condition_variable_window_to_scene.object_type = ObjectType::object_condition_variable;
        condition_variable_window_to_scene.wait_type = WaitType::window_to_scene;

        st_input_object mutex_window_to_scene{};
        mutex_window_to_scene.object = data_scene->mutex_window_to_scene;
        mutex_window_to_scene.object_type = ObjectType::object_mutex;
        mutex_window_to_scene.wait_type = WaitType::window_to_scene;

        st_input_object flag_window_to_scene{};
        flag_window_to_scene.object = &data_scene->flag_window_to_scene;
        flag_window_to_scene.object_type = ObjectType::object_flag;
        flag_window_to_scene.wait_type = WaitType::window_to_scene;

        vector_input(condition_variable_window_to_scene);
        vector_input(mutex_window_to_scene);
        vector_input(flag_window_to_scene);

        RECT base_rect_window{ 0, 0, 0, 0 };
        for (auto it_window = _map_window.begin(); it_window != _map_window.end(); it_window++)
        {
            pst_window data_window = it_window->second;

            if (data_window->rect.left <= rect.left &&
                data_window->rect.top <= rect.top &&
                data_window->rect.right > rect.left &&
                data_window->rect.bottom > rect.top)
            {
                base_rect_window = data_window->rect;
            }
        }

        normalize_rect(base_rect_window, data_scene->rect, data_scene->normal_rect);

        for (size_t i = 0; i < _count_texture_store; i++)
        {
            AVFrame* frame = av_frame_alloc();
            data_scene->vector_frame.push_back(frame);
        }

        _map_scene.insert({ scene_index, data_scene });
    }
}

void delete_scenes()
{
    for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end();)
    {
        pst_scene data_scene = it_scene->second;

        if (data_scene->ffmpeg_instance)
        {
            cpp_ffmpeg_wrapper_play_stop(data_scene->ffmpeg_instance, nullptr);
            cpp_ffmpeg_wrapper_shutdown(data_scene->ffmpeg_instance);
            cpp_ffmpeg_wrapper_delete(data_scene->ffmpeg_instance);
            data_scene->ffmpeg_instance = nullptr;
        }

        for (auto it_vector = data_scene->vector_frame.begin(); it_vector != data_scene->vector_frame.end();)
        {
            AVFrame* frame = *it_vector;

            av_frame_free(&frame);

            it_vector = data_scene->vector_frame.erase(it_vector);
        }

        if (data_scene->event_scene_to_upload)
        {
            CloseHandle(data_scene->event_scene_to_upload);
            data_scene->event_scene_to_upload = nullptr;
        }

        delete data_scene->condition_variable_window_to_scene;
        data_scene->condition_variable_window_to_scene = nullptr;

        delete data_scene->mutex_window_to_scene;
        data_scene->mutex_window_to_scene = nullptr;

        delete data_scene;
        data_scene = nullptr;

        it_scene = _map_scene.erase(it_scene);
    }
}

void callback_ffmpeg_wrapper_ptr(void* param)
{

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
    std::wstring str_ini_path_w = str_path_w + L"\\DPlayer.ini";

    char path_a[260] = { 0, };
    GetModuleFileNameA(nullptr, path_a, 260);
    std::string str_path_a = path_a;
    str_path_a = str_path_a.substr(0, str_path_a.find_last_of("\\/"));
    std::string str_ini_path_a = str_path_a + "\\DPlayer.ini";

    char result_a[255];
    ZeroMemory(result_a, 255);
    wchar_t result_w[255];
    int result_i = 0;

    GetPrivateProfileString(L"DPlayer", L"flag_set_logger", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _flag_set_logger = result_i == 0 ? false : true;

    GetPrivateProfileString(L"DPlayer", L"log_level", L"6", result_w, 255, str_ini_path_w.c_str());
    _log_level = _ttoi(result_w);

    GetPrivateProfileString(L"DPlayer", L"wait_for_multiple_objects_wait_time", L"1000", result_w, 255, str_ini_path_w.c_str());
    _wait_for_multiple_objects_wait_time = _ttoi(result_w);

    GetPrivateProfileString(L"DPlayer", L"texture_type", L"0", result_w, 255, str_ini_path_w.c_str());
    _texture_type = _ttoi(result_w);

    GetPrivateProfileString(L"DPlayer", L"use_nvapi", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _use_nvapi = result_i == 0 ? false : true;

    GetPrivateProfileString(L"DPlayer", L"block_swap_group_present", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _block_swap_group_present = result_i == 0 ? false : true;

    GetPrivateProfileString(L"DPlayer", L"set_maximum_frame_latency", L"0", result_w, 255, str_ini_path_w.c_str());
    _set_maximum_frame_latency = _ttoi(result_w);

    GetPrivateProfileString(L"DPlayer", L"use_wait_for_vblank", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _use_wait_for_vblank = result_i == 0 ? false : true;

    GetPrivateProfileString(L"DPlayer", L"use_wait_for_vblank_repeat", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _use_wait_for_vblank_repeat = result_i == 0 ? false : true;

    GetPrivateProfileString(L"DPlayer", L"use_wait_for_vblank_first_entry", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _use_wait_for_vblank_first_entry = result_i == 0 ? false : true;

    GetPrivateProfileString(L"DPlayer", L"count_texture_store", L"30", result_w, 255, str_ini_path_w.c_str());
    _count_texture_store = _ttoi(result_w);

    GetPrivateProfileString(L"DPlayer", L"hw_device_type", L"12", result_w, 255, str_ini_path_w.c_str());
    _hw_device_type = _ttoi(result_w);

    GetPrivateProfileString(L"DPlayer", L"control_monitor_left", L"0", result_w, 255, str_ini_path_w.c_str());
    _control_monitor_left = _ttoi(result_w);
    GetPrivateProfileString(L"DPlayer", L"control_monitor_top", L"0", result_w, 255, str_ini_path_w.c_str());
    _control_monitor_top = _ttoi(result_w);
    GetPrivateProfileString(L"DPlayer", L"control_monitor_right", L"0", result_w, 255, str_ini_path_w.c_str());
    _control_monitor_right = _ttoi(result_w);
    GetPrivateProfileString(L"DPlayer", L"control_monitor_bottom", L"0", result_w, 255, str_ini_path_w.c_str());
    _control_monitor_bottom = _ttoi(result_w);

    GetPrivateProfileString(L"DPlayer", L"use_play_repeat", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _use_play_repeat = result_i == 0 ? false : true;

    GetPrivateProfileString(L"DPlayer", L"count_scene_fps", L"0", result_w, 255, str_ini_path_w.c_str());
    _count_scene_fps = _ttoi(result_w);
}

void thread_client()
{
    cppsocket_network_initialize();

    _client = cppsocket_client_create();

    cppsocket_client_set_callback_data(_client, callback_ptr_client);

    if (cppsocket_client_connect(_client, _ip.c_str(), _port))
    {
        if (cppsocket_client_is_connected(_client))
        {
            cppsocket_struct_client_send_player_connect data{};
            data.player_sync_group_index = _player_sync_group_index;

            cppsocket_client_send_dplayer_connect(_client, data);
        }

        while (_flag_thread_client)
        {
            if (cppsocket_client_is_connected(_client))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_main_loop));

                cppsocket_client_frame(_client);
            }
        }
    }
    cppsocket_client_delete(_client);

    cppsocket_client_connection_close(_client);
    cppsocket_network_shutdown();
}

void thread_packet_processing()
{
    while (_flag_packet_processing)
    {
        message_processing_window();

        bool flag_processing_command_is_empty = false;
        {
            std::lock_guard<std::mutex> lk(_mutex_packet_processing);
            flag_processing_command_is_empty = _queue_packet_processing.empty();
        }

        if (flag_processing_command_is_empty)
        {
            if (_flag_repeat)
            {
                if (!(_map_scene_url.empty() || _map_scene_coordinate.empty()))
                {
                    play_repeat_instance_recreate();
                }

                _flag_repeat = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_processing));
            continue;
        }

        // packet_data
        void* data = nullptr;
        {
            std::lock_guard<std::mutex> lk(_mutex_packet_processing);
            data = _queue_packet_processing.front();
            _queue_packet_processing.pop_front();
        }

        packet_header* header = (packet_header*)data;

        switch (header->cmd)
        {
            // �ڵ� �ۼ�
        case command_type::dplayer_connect_data_url:
        {
            if (_flag_set_logger)
            {
                std::string str = "";
                str.append("thread_packet_processing");
                str.append(", dplayer_connect_data_url");

                auto logger = spdlog::get(_logger_name.c_str());
                logger->debug(str.c_str());
            }

            packet_dplayer_connect_data_url_from_server* packet = new packet_dplayer_connect_data_url_from_server();
            memcpy(packet, data, header->size);

            if (_player_sync_group_index == packet->player_sync_group_index)
            {
                if (_count_scene == 0)
                {
                    _count_scene = packet->player_sync_group_input_count;
                }

                _map_scene_url.insert({ packet->scene_index, packet->url });
            }

            delete packet;

            check_ready_to_playback();
        }
        break;
        case command_type::dplayer_connect_data_rect:
        {
            if (_flag_set_logger)
            {
                std::string str = "";
                str.append("thread_packet_processing");
                str.append(", dplayer_connect_data_rect");

                auto logger = spdlog::get(_logger_name.c_str());
                logger->debug(str.c_str());
            }

            packet_dplayer_connect_data_rect_from_server* packet = new packet_dplayer_connect_data_rect_from_server();
            memcpy(packet, data, header->size);

            if (_player_sync_group_index == packet->player_sync_group_index)
            {
                if (_count_scene == 0)
                {
                    _count_scene = packet->player_sync_group_output_count;
                }

                pst_coordinate data_coordinate = new st_coordinate();
                data_coordinate->left = packet->left;
                data_coordinate->top = packet->top;
                data_coordinate->width = packet->width;
                data_coordinate->height = packet->height;

                _map_scene_coordinate.insert({ packet->scene_index, data_coordinate });
            }

            delete packet;

            check_ready_to_playback();
        }
        break;
        case command_type::dplayer_stop:
        {
            packet_dplayer_stop_from_server* packet = new packet_dplayer_stop_from_server();
            memcpy(packet, data, header->size);

            delete packet;

            for (auto it_scene_coordinate = _map_scene_coordinate.begin(); it_scene_coordinate != _map_scene_coordinate.end();)
            {
                pst_coordinate data_coordinate = it_scene_coordinate->second;
                delete data_coordinate;
                it_scene_coordinate = _map_scene_coordinate.erase(it_scene_coordinate);
            }

            _is_running = false;
        }
        break;
        default:
            break;
        }

        delete data;
    }
}

void callback_ptr_client(void* data)
{
    packet_header* header = (packet_header*)data;

    void* packet = new char[header->size];
    memcpy(packet, data, header->size);

    {
        std::lock_guard<std::mutex> lk(_mutex_packet_processing);
        _queue_packet_processing.push_back(packet);
    }
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

        for (auto it_object = _vector_input_object.begin(); it_object != _vector_input_object.end();)
        {
            st_input_object object = *it_object;

            HANDLE object_event = nullptr;
            std::condition_variable* object_condition_variable = nullptr;
            std::mutex* object_mutex = nullptr;
            bool* object_flag = nullptr;

            switch (object.object_type)
            {
            case ObjectType::object_event:
            {
                object_event = (HANDLE)object.object;
            }
            break;
            case ObjectType::object_condition_variable:
            {
                object_condition_variable = (std::condition_variable*)object.object;
            }
            break;
            case ObjectType::object_mutex:
            {
                object_mutex = (std::mutex*)object.object;
            }
            break;
            case ObjectType::object_flag:
            {
                object_flag = (bool*)object.object;
            }
            break;
            default:
                break;
            }

            switch (object.wait_type)
            {
            case WaitType::scene_to_upload:
            {
                if (object_event != nullptr)
                {
                    _mutex_vector_event_scene_to_upload.lock();
                    _vector_event_scene_to_upload.push_back(object_event);
                    _mutex_vector_event_scene_to_upload.unlock();
                }

                if (object_condition_variable != nullptr)
                {
                    _mutex_vector_condition_variable_scene_to_upload.lock();
                    _vector_condition_variable_scene_to_upload.push_back(object_condition_variable);
                    _mutex_vector_condition_variable_scene_to_upload.unlock();
                }

                if (object_mutex != nullptr)
                {
                    _mutex_vector_mutex_scene_to_upload.lock();
                    _vector_mutex_scene_to_upload.push_back(object_mutex);
                    _mutex_vector_mutex_scene_to_upload.unlock();
                }

                if (object_flag != nullptr)
                {
                    _mutex_vector_flag_scene_to_upload.lock();
                    _vector_flag_scene_to_upload.push_back(object_flag);
                    _mutex_vector_flag_scene_to_upload.unlock();
                }
            }
            break;
            case WaitType::upload_to_device:
            {
                if (object_event != nullptr)
                {
                    _mutex_vector_event_upload_to_device.lock();
                    _vector_event_upload_to_device.push_back(object_event);
                    _mutex_vector_event_upload_to_device.unlock();
                }

                if (object_condition_variable != nullptr)
                {
                    _mutex_vector_condition_variable_upload_to_device.lock();
                    _vector_condition_variable_upload_to_device.push_back(object_condition_variable);
                    _mutex_vector_condition_variable_upload_to_device.unlock();
                }

                if (object_mutex != nullptr)
                {
                    _mutex_vector_mutex_upload_to_device.lock();
                    _vector_mutex_upload_to_device.push_back(object_mutex);
                    _mutex_vector_mutex_upload_to_device.unlock();
                }

                if (object_flag != nullptr)
                {
                    _mutex_vector_flag_upload_to_device.lock();
                    _vector_flag_upload_to_device.push_back(object_flag);
                    _mutex_vector_flag_upload_to_device.unlock();
                }
            }
            break;
            case WaitType::device_to_window:
            {
                if (object_event != nullptr)
                {
                    _mutex_vector_event_device_to_window.lock();
                    _vector_event_device_to_window.push_back(object_event);
                    _mutex_vector_event_device_to_window.unlock();
                }

                if (object_condition_variable != nullptr)
                {
                    _mutex_vector_condition_variable_device_to_window.lock();
                    _vector_condition_variable_device_to_window.push_back(object_condition_variable);
                    _mutex_vector_condition_variable_device_to_window.unlock();
                }

                if (object_mutex != nullptr)
                {
                    _mutex_vector_mutex_device_to_window.lock();
                    _vector_mutex_device_to_window.push_back(object_mutex);
                    _mutex_vector_mutex_device_to_window.unlock();
                }

                if (object_flag != nullptr)
                {
                    _mutex_vector_flag_device_to_window.lock();
                    _vector_flag_device_to_window.push_back(object_flag);
                    _mutex_vector_flag_device_to_window.unlock();
                }
            }
            break;
            case WaitType::window_to_scene:
            {
                if (object_event != nullptr)
                {
                    _mutex_vector_event_window_to_scene.lock();
                    _vector_event_window_to_scene.push_back(object_event);
                    _mutex_vector_event_window_to_scene.unlock();
                }

                if (object_condition_variable != nullptr)
                {
                    _mutex_vector_condition_variable_window_to_scene.lock();
                    _vector_condition_variable_window_to_scene.push_back(object_condition_variable);
                    _mutex_vector_condition_variable_window_to_scene.unlock();
                }

                if (object_mutex != nullptr)
                {
                    _mutex_vector_mutex_window_to_scene.lock();
                    _vector_mutex_window_to_scene.push_back(object_mutex);
                    _mutex_vector_mutex_window_to_scene.unlock();
                }

                if (object_flag != nullptr)
                {
                    _mutex_vector_flag_window_to_scene.lock();
                    _vector_flag_window_to_scene.push_back(object_flag);
                    _mutex_vector_flag_window_to_scene.unlock();
                }
            }
            break;
            default:
                break;
            }

            it_object = _vector_input_object.erase(it_object);
        }
    }
}

void vector_input(st_input_object data)
{
    std::lock_guard<std::mutex> lk(_mutex_input_object);

    _vector_input_object.push_back(data);
}

void thread_wait_for_multiple_objects(WaitType wait_type, bool* flag_thread)
{
    std::vector<HANDLE>* vector_handle = nullptr;
    std::vector<std::condition_variable*>* vector_condition_variable = nullptr;
    std::vector<std::mutex*>* vector_mutex = nullptr;
    std::vector<bool*>* vector_flag = nullptr;

    bool flag_timeout = false;
    bool flag_vector_need_more = false;

    while (*flag_thread == true)
    {
        switch (wait_type)
        {
        case WaitType::scene_to_upload:
        {
            vector_handle = &_vector_event_scene_to_upload;
            _mutex_vector_event_scene_to_upload.lock();
            if (vector_handle->size() < _map_scene.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_event_scene_to_upload.unlock();

            vector_condition_variable = &_vector_condition_variable_scene_to_upload;
            _mutex_vector_condition_variable_scene_to_upload.lock();
            if (vector_condition_variable->size() < _map_device.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_condition_variable_scene_to_upload.unlock();

            vector_mutex = &_vector_mutex_scene_to_upload;
            _mutex_vector_mutex_scene_to_upload.lock();
            if (vector_mutex->size() < _map_device.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_mutex_scene_to_upload.unlock();

            vector_flag = &_vector_flag_scene_to_upload;
            _mutex_vector_flag_scene_to_upload.lock();
            if (vector_flag->size() < _map_device.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_flag_scene_to_upload.unlock();
        }
        break;
        case WaitType::upload_to_device:
        {
            vector_handle = &_vector_event_upload_to_device;
            _mutex_vector_event_upload_to_device.lock();
            if (vector_handle->size() < _map_device.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_event_upload_to_device.unlock();

            vector_condition_variable = &_vector_condition_variable_upload_to_device;
            _mutex_vector_condition_variable_upload_to_device.lock();
            if (vector_condition_variable->size() < _map_device.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_condition_variable_upload_to_device.unlock();

            vector_mutex = &_vector_mutex_upload_to_device;
            _mutex_vector_mutex_upload_to_device.lock();
            if (vector_mutex->size() < _map_device.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_mutex_upload_to_device.unlock();

            vector_flag = &_vector_flag_upload_to_device;
            _mutex_vector_flag_upload_to_device.lock();
            if (vector_flag->size() < _map_device.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_flag_upload_to_device.unlock();
        }
        break;
        case WaitType::device_to_window:
        {
            vector_handle = &_vector_event_device_to_window;
            _mutex_vector_event_device_to_window.lock();
            if (vector_handle->size() < _map_device.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_event_device_to_window.unlock();

            vector_condition_variable = &_vector_condition_variable_device_to_window;
            _mutex_vector_condition_variable_device_to_window.lock();
            if (vector_condition_variable->size() < _map_window.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_condition_variable_device_to_window.unlock();

            vector_mutex = &_vector_mutex_device_to_window;
            _mutex_vector_mutex_device_to_window.lock();
            if (vector_mutex->size() < _map_window.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_mutex_device_to_window.unlock();

            vector_flag = &_vector_flag_device_to_window;
            _mutex_vector_flag_device_to_window.lock();
            if (vector_flag->size() < _map_window.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_flag_device_to_window.unlock();
        }
        break;
        case WaitType::window_to_scene:
        {
            vector_handle = &_vector_event_window_to_scene;
            _mutex_vector_event_window_to_scene.lock();
            if (vector_handle->size() < _map_window.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_event_window_to_scene.unlock();

            vector_condition_variable = &_vector_condition_variable_window_to_scene;
            _mutex_vector_condition_variable_window_to_scene.lock();
            if (vector_condition_variable->size() < _map_scene.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_condition_variable_window_to_scene.unlock();

            vector_mutex = &_vector_mutex_window_to_scene;
            _mutex_vector_mutex_window_to_scene.lock();
            if (vector_mutex->size() < _map_scene.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_mutex_window_to_scene.unlock();

            vector_flag = &_vector_flag_window_to_scene;
            _mutex_vector_flag_window_to_scene.lock();
            if (vector_flag->size() < _map_scene.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_flag_window_to_scene.unlock();
        }
        break;
        default:
            break;
        }


        if (flag_vector_need_more == true)
        {
            flag_vector_need_more = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_main_loop));
            continue;
        }

        DWORD n_count = vector_handle->size();

        size_t n_cv_count = vector_condition_variable->size();

        // Wait Multiple Objects
        DWORD hr = 0;
        HANDLE* lp_handles = vector_handle->data();
        bool b_wait_all = true;
        hr = WaitForMultipleObjects(n_count, lp_handles, b_wait_all, _wait_for_multiple_objects_wait_time);
        {
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
        }

        if (flag_timeout == true)
        {
            flag_timeout = false;
            continue;
        }

        if (_use_wait_for_vblank_repeat)
        {
            if (wait_type == WaitType::window_to_scene)
            {
                if (_flag_wait_for_vblank_repeat)
                {
                    _flag_wait_for_vblank_repeat = false;
                }
            }
            else if (wait_type == WaitType::scene_to_upload)
            {
                if (_flag_scene_repeat)
                {
                    if (_flag_wait_for_vblank_repeat == false)
                    {
                        _flag_wait_for_vblank_repeat = true;
                    }

                    _flag_scene_repeat = false;
                }
            }
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

void thread_device(pst_device data_device)
{
    auto it_command_allocator = _map_command_allocator.find(data_device->device_index);
    pst_command_allocator data_command_allocator = it_command_allocator->second;

    auto it_command_list = _map_command_list.find(data_device->device_index);
    pst_command_list data_command_list = it_command_list->second;

    auto it_pso = _map_pso.find(data_device->device_index);
    pst_pso data_pso = it_pso->second;

    auto it_root_sig = _map_root_sig.find(data_device->device_index);
    pst_root_signature data_root_sig = it_root_sig->second;

    auto it_srv_heap = _map_srv_heap.find(data_device->device_index);
    pst_srv_heap data_srv_heap = it_srv_heap->second;

    auto it_rtv = _map_rtv.find(data_device->device_index);
    pst_rtv data_rtv = it_rtv->second;

    auto it_viewport = _map_viewport.find(data_device->device_index);
    pst_viewport data_viewport = it_viewport->second;

    auto it_command_queue = _map_command_queue.find(data_device->device_index);
    pst_command_queue data_command_queue = it_command_queue->second;

    auto it_fence = _map_fence.find(data_device->device_index);
    pst_fence data_fence = it_fence->second;

    bool flag_created = false;

    int64_t rtv_index = -1;
    int64_t srv_index = -1;

    float color_offset = 0.0f;

    std::vector<pst_scene> vector_scene;

    while (data_device->flag_thread_device)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_main_loop));

        rtv_index += 1;
        if (!(rtv_index < _frame_buffer_count))
        {
            rtv_index = 0;
        }

        srv_index += 1;
        if (!(srv_index < _count_texture_store))
        {
            srv_index = 0;
        }

        {
            std::unique_lock<std::mutex> lk(*data_device->mutex_upload_to_device);
            if (data_device->flag_upload_to_device == false)
            {
                data_device->condition_variable_upload_to_device->wait(lk);
            }
            data_device->flag_upload_to_device = false;
        }

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_device, device index = ");
            str.append(std::to_string(data_device->device_index));
            str.append(", wait notified, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }

        if (!data_device->flag_thread_device)
        {
            break;
        }

        ID3D12CommandAllocator* command_allocator = data_command_allocator->vector_command_allocator.at(rtv_index);
        ID3D12GraphicsCommandList* command_list = data_command_list->command_list;
        ID3D12PipelineState* pso = data_pso->pso;

        command_allocator->Reset();
        command_list->Reset(command_allocator, pso);

        if (flag_created == false)
        {
            for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
            {
                pst_scene data_scene = it_scene->second;

                if (data_scene->device_index != data_device->device_index)
                {
                    continue;
                }

                vector_scene.push_back(data_scene);
            }

            flag_created = true;
        }

        _mutex_map_index_buffer_view->lock();
        auto it_index_buffer_view = _map_index_buffer_view.find(data_device->device_index);
        pst_index_buffer_view data_index_buffer_view = it_index_buffer_view->second;
        _mutex_map_index_buffer_view->unlock();

        _mutex_map_vertex_buffer_view->lock();
        auto it_vertex_buffer_view = _map_vertex_buffer_view.find(data_device->device_index);
        pst_vertex_buffer_view data_vertex_buffer_view = it_vertex_buffer_view->second;
        _mutex_map_vertex_buffer_view->unlock();

        _mutex_map_srv_handle_luminance->lock();
        auto it_srv_handle_luminance = _map_srv_handle_luminance.find(data_device->device_index);
        _mutex_map_srv_handle_luminance->unlock();
        pst_srv_handle data_srv_handle_luminance = it_srv_handle_luminance->second;
        
        _mutex_map_srv_handle_chrominance->lock();
        auto it_srv_handle_chrominance = _map_srv_handle_chrominance.find(data_device->device_index);
        _mutex_map_srv_handle_chrominance->unlock();
        pst_srv_handle data_srv_handle_chrominance = it_srv_handle_chrominance->second;


        command_list->SetGraphicsRootSignature(data_root_sig->root_sig);

        ID3D12DescriptorHeap* pp_heaps[] = { data_srv_heap->srv_heap };
        command_list->SetDescriptorHeaps(_countof(pp_heaps), pp_heaps);

        CD3DX12_RESOURCE_BARRIER barrier_before = CD3DX12_RESOURCE_BARRIER::Transition(data_rtv->vector_rtv.at(rtv_index), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        command_list->ResourceBarrier(1, &barrier_before);

        command_list->OMSetRenderTargets(1, &data_rtv->vector_rtv_handle.at(rtv_index), FALSE, nullptr);

        float color[4] = { color_offset, color_offset, color_offset, color_offset };
        command_list->ClearRenderTargetView(data_rtv->vector_rtv_handle.at(rtv_index), color, 0, nullptr);
        command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        command_list->IASetIndexBuffer(&data_index_buffer_view->index_buffer_view);

        uint64_t counter = 0;
        for (auto it_vector = vector_scene.begin(); it_vector != vector_scene.end(); it_vector++)
        {
            pst_scene data_scene = *it_vector;

            command_list->IASetVertexBuffers(0, 1, &data_vertex_buffer_view->vector_vertex_buffer_view.at(counter));

            command_list->RSSetViewports(1, &data_viewport->viewport);
            command_list->RSSetScissorRects(1, &data_viewport->scissor_rect);

            command_list->SetGraphicsRootDescriptorTable(0, data_srv_handle_luminance->vector_handle_gpu.at((_count_texture_store * counter) + srv_index));
            command_list->SetGraphicsRootDescriptorTable(1, data_srv_handle_chrominance->vector_handle_gpu.at((_count_texture_store * counter) + srv_index));

            command_list->DrawIndexedInstanced(6, 1, 0, 0, 0);

            counter++;
        }

        CD3DX12_RESOURCE_BARRIER barrier_after = CD3DX12_RESOURCE_BARRIER::Transition(data_rtv->vector_rtv.at(rtv_index), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        command_list->ResourceBarrier(1, &barrier_after);

        command_list->Close();

        ID3D12CommandList* command_lists[] = { command_list };

        ID3D12CommandQueue* command_queue = data_command_queue->command_queue;

        command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

        data_fence->fence_value_device++;
        if (data_fence->fence_value_device == UINT64_MAX)
        {
            data_fence->fence_value_device = 0;
        }
        data_fence->fence_device->SetEventOnCompletion(data_fence->fence_value_device, data_fence->fence_event_device);
        command_queue->Signal(data_fence->fence_device, data_fence->fence_value_device);

        WaitForSingleObject(data_fence->fence_event_device, INFINITE);

        color_offset += 0.1f;
        if (color_offset > 1.0f)
        {
            color_offset = 0.0f;
        }

        SetEvent(data_device->event_device_to_window);

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_device, device index = ");
            str.append(std::to_string(data_device->device_index));
            str.append(", set event, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }
    }
}

void thread_device_yuv(pst_device data_device)
{
    auto it_command_allocator = _map_command_allocator.find(data_device->device_index);
    pst_command_allocator data_command_allocator = it_command_allocator->second;

    auto it_command_list = _map_command_list.find(data_device->device_index);
    pst_command_list data_command_list = it_command_list->second;

    auto it_pso = _map_pso.find(data_device->device_index);
    pst_pso data_pso = it_pso->second;

    auto it_root_sig = _map_root_sig.find(data_device->device_index);
    pst_root_signature data_root_sig = it_root_sig->second;

    auto it_srv_heap = _map_srv_heap.find(data_device->device_index);
    pst_srv_heap data_srv_heap = it_srv_heap->second;

    auto it_rtv = _map_rtv.find(data_device->device_index);
    pst_rtv data_rtv = it_rtv->second;

    auto it_viewport = _map_viewport.find(data_device->device_index);
    pst_viewport data_viewport = it_viewport->second;

    auto it_command_queue = _map_command_queue.find(data_device->device_index);
    pst_command_queue data_command_queue = it_command_queue->second;

    auto it_fence = _map_fence.find(data_device->device_index);
    pst_fence data_fence = it_fence->second;

    bool flag_created = false;

    int64_t rtv_index = -1;
    int64_t srv_index = -1;

    float color_offset = 0.0f;

    std::vector<pst_scene> vector_scene;

    while (data_device->flag_thread_device)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_main_loop));

        rtv_index += 1;
        if (!(rtv_index < _frame_buffer_count))
        {
            rtv_index = 0;
        }

        srv_index += 1;
        if (!(srv_index < _count_texture_store))
        {
            srv_index = 0;
        }

        {
            std::unique_lock<std::mutex> lk(*data_device->mutex_upload_to_device);
            if (data_device->flag_upload_to_device == false)
            {
                data_device->condition_variable_upload_to_device->wait(lk);
            }
            data_device->flag_upload_to_device = false;
        }

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_device, device index = ");
            str.append(std::to_string(data_device->device_index));
            str.append(", wait notified, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }

        ID3D12CommandAllocator* command_allocator = data_command_allocator->vector_command_allocator.at(rtv_index);
        ID3D12GraphicsCommandList* command_list = data_command_list->command_list;
        ID3D12PipelineState* pso = data_pso->pso;

        command_allocator->Reset();
        command_list->Reset(command_allocator, pso);

        if (flag_created == false)
        {
            for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
            {
                pst_scene data_scene = it_scene->second;

                if (data_scene->device_index != data_device->device_index)
                {
                    continue;
                }

                vector_scene.push_back(data_scene);
            }

            flag_created = true;
        }

        _mutex_map_index_buffer_view->lock();
        auto it_index_buffer_view = _map_index_buffer_view.find(data_device->device_index);
        pst_index_buffer_view data_index_buffer_view = it_index_buffer_view->second;
        _mutex_map_index_buffer_view->unlock();

        _mutex_map_vertex_buffer_view->lock();
        auto it_vertex_buffer_view = _map_vertex_buffer_view.find(data_device->device_index);
        pst_vertex_buffer_view data_vertex_buffer_view = it_vertex_buffer_view->second;
        _mutex_map_vertex_buffer_view->unlock();

        _mutex_map_srv_handle_y->lock();
        auto it_srv_handle_y = _map_srv_handle_y.find(data_device->device_index);
        pst_srv_handle data_srv_handle_y = it_srv_handle_y->second;
        _mutex_map_srv_handle_y->unlock();
        _mutex_map_srv_handle_u->lock();
        auto it_srv_handle_u = _map_srv_handle_u.find(data_device->device_index);
        pst_srv_handle data_srv_handle_u = it_srv_handle_u->second;
        _mutex_map_srv_handle_u->unlock();
        _mutex_map_srv_handle_v->lock();
        auto it_srv_handle_v = _map_srv_handle_v.find(data_device->device_index);
        pst_srv_handle data_srv_handle_v = it_srv_handle_v->second;
        _mutex_map_srv_handle_v->unlock();

        command_list->SetGraphicsRootSignature(data_root_sig->root_sig);

        ID3D12DescriptorHeap* pp_heaps[] = { data_srv_heap->srv_heap };

        command_list->SetDescriptorHeaps(_countof(pp_heaps), pp_heaps);

        CD3DX12_RESOURCE_BARRIER barrier_before = CD3DX12_RESOURCE_BARRIER::Transition(data_rtv->vector_rtv.at(rtv_index), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        command_list->ResourceBarrier(1, &barrier_before);

        command_list->OMSetRenderTargets(1, &data_rtv->vector_rtv_handle.at(rtv_index), FALSE, nullptr);
        float color[4] = { color_offset, color_offset, color_offset, color_offset };
        command_list->ClearRenderTargetView(data_rtv->vector_rtv_handle.at(rtv_index), color, 0, nullptr);
        command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        command_list->IASetIndexBuffer(&data_index_buffer_view->index_buffer_view);

        uint64_t counter = 0;
        for (auto it_vector = vector_scene.begin(); it_vector != vector_scene.end(); it_vector++)
        {
            pst_scene data_scene = *it_vector;

            command_list->IASetVertexBuffers(0, 1, &data_vertex_buffer_view->vector_vertex_buffer_view.at(counter));

            command_list->RSSetViewports(1, &data_viewport->viewport);
            command_list->RSSetScissorRects(1, &data_viewport->scissor_rect);

            command_list->SetGraphicsRootDescriptorTable(0, data_srv_handle_y->vector_handle_gpu.at((_count_texture_store * counter) + srv_index));
            command_list->SetGraphicsRootDescriptorTable(1, data_srv_handle_u->vector_handle_gpu.at((_count_texture_store * counter) + srv_index));
            command_list->SetGraphicsRootDescriptorTable(2, data_srv_handle_v->vector_handle_gpu.at((_count_texture_store * counter) + srv_index));

            command_list->DrawIndexedInstanced(6, 1, 0, 0, 0);

            counter++;
        }

        CD3DX12_RESOURCE_BARRIER barrier_after = CD3DX12_RESOURCE_BARRIER::Transition(data_rtv->vector_rtv.at(rtv_index), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        command_list->ResourceBarrier(1, &barrier_after);

        command_list->Close();

        ID3D12CommandList* command_lists[] = { command_list };

        ID3D12CommandQueue* command_queue = data_command_queue->command_queue;

        command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);


        data_fence->fence_value_device++;
        if (data_fence->fence_value_device == UINT64_MAX)
        {
            data_fence->fence_value_device = 0;
        }
        data_fence->fence_device->SetEventOnCompletion(data_fence->fence_value_device, data_fence->fence_event_device);
        command_queue->Signal(data_fence->fence_device, data_fence->fence_value_device);

        WaitForSingleObject(data_fence->fence_event_device, INFINITE);


        color_offset += 0.1f;
        if (color_offset > 1.0f)
        {
            color_offset = 0.0f;
        }

        SetEvent(data_device->event_device_to_window);

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_device, device index = ");
            str.append(std::to_string(data_device->device_index));
            str.append(", set event, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }
    }
}

void thread_upload(pst_device data_device)
{
    auto it_command_allocator = _map_command_allocator.find(data_device->device_index);
    pst_command_allocator data_command_allocator = it_command_allocator->second;

    auto it_command_list = _map_command_list.find(data_device->device_index);
    pst_command_list data_command_list = it_command_list->second;

    auto it_pso = _map_pso.find(data_device->device_index);
    pst_pso data_pso = it_pso->second;

    auto it_command_queue = _map_command_queue.find(data_device->device_index);
    pst_command_queue data_command_queue = it_command_queue->second;

    auto it_fence = _map_fence.find(data_device->device_index);
    pst_fence data_fence = it_fence->second;

    bool flag_created = false;

    int64_t upload_index = 2;
    int64_t srv_index = -1;

    std::vector<pst_scene> vector_scene;

    while (data_device->flag_thread_upload)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_main_loop));

        upload_index += 1;
        if (!(upload_index < 6))
        {
            upload_index = 3;
        }

        srv_index += 1;
        if (!(srv_index < _count_texture_store))
        {
            srv_index = 0;
        }

        {
            std::unique_lock<std::mutex> lk(*data_device->mutex_scene_to_upload);
            if (data_device->flag_scene_to_upload == false)
            {
                data_device->condition_variable_scene_to_upload->wait(lk);
            }
            data_device->flag_scene_to_upload = false;
        }

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_upload, device index = ");
            str.append(std::to_string(data_device->device_index));
            str.append(", wait notified, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }

        if (flag_created == true)
        {
            int temp_index = 0;

            if (srv_index != 0)
            {
                temp_index = srv_index - 1;
            }
            else
            {
                temp_index = _count_texture_store - 1;
            }

            for (auto it_vector = vector_scene.begin(); it_vector != vector_scene.end(); it_vector++)
            {
                pst_scene data_scene = *it_vector;

                av_frame_unref(data_scene->vector_frame.at(temp_index));
            }
        }

        if (!data_device->flag_thread_upload)
        {
            break;
        }

        ID3D12CommandAllocator* command_allocator = data_command_allocator->vector_command_allocator.at(upload_index);
        ID3D12GraphicsCommandList* command_list = data_command_list->command_list;
        ID3D12PipelineState* pso = data_pso->pso;

        command_allocator->Reset();
        command_list->Reset(command_allocator, pso);

        if (flag_created == false)
        {
            create_vertex_buffer(data_device);
            create_index_buffer(data_device);

            uint64_t counter_scene = 0;
            for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
            {
                pst_scene data_scene = it_scene->second;

                if (data_scene->device_index != data_device->device_index)
                {
                    continue;
                }

                AVFrame* frame = data_scene->vector_frame.at(srv_index);

                uint64_t width = frame->width;
                uint32_t height = frame->height;

                create_texture(data_device, width, height, counter_scene);

                vector_scene.push_back(data_scene);

                counter_scene++;
            }

            flag_created = true;
        }


        uint64_t counter_texture = 0;
        for (auto it_vector = vector_scene.begin(); it_vector != vector_scene.end(); it_vector++)
        {
            pst_scene data_scene = *it_vector;
            upload_texture(data_device, data_scene->vector_frame.at(srv_index), counter_texture, srv_index);

            counter_texture++;
        }

        command_list->Close();

        ID3D12CommandList* command_lists[] = { command_list };

        ID3D12CommandQueue* command_queue = data_command_queue->command_queue;

        command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);


        data_fence->fence_value_upload++;
        if (data_fence->fence_value_upload == UINT64_MAX)
        {
            data_fence->fence_value_upload = 0;
        }
        data_fence->fence_upload->SetEventOnCompletion(data_fence->fence_value_upload, data_fence->fence_event_upload);
        command_queue->Signal(data_fence->fence_upload, data_fence->fence_value_upload);

        WaitForSingleObject(data_fence->fence_event_upload, INFINITE);


        //for (auto it_vector = vector_scene.begin(); it_vector != vector_scene.end(); it_vector++)
        //{
        //    pst_scene data_scene = *it_vector;

        //    av_frame_unref(data_scene->vector_frame.at(srv_index));
        //}


        SetEvent(data_device->event_upload_to_device);

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_upload, device index = ");
            str.append(std::to_string(data_device->device_index));
            str.append(", set event, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }
    }
}

void thread_upload_yuv(pst_device data_device)
{
    auto it_command_allocator = _map_command_allocator.find(data_device->device_index);
    pst_command_allocator data_command_allocator = it_command_allocator->second;

    auto it_command_list = _map_command_list.find(data_device->device_index);
    pst_command_list data_command_list = it_command_list->second;

    auto it_pso = _map_pso.find(data_device->device_index);
    pst_pso data_pso = it_pso->second;

    auto it_command_queue = _map_command_queue.find(data_device->device_index);
    pst_command_queue data_command_queue = it_command_queue->second;

    auto it_fence = _map_fence.find(data_device->device_index);
    pst_fence data_fence = it_fence->second;

    bool flag_created = false;

    int64_t upload_index = 2;
    int64_t srv_index = -1;

    std::vector<pst_scene> vector_scene;

    while (data_device->flag_thread_upload)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_main_loop));

        upload_index += 1;
        if (!(upload_index < 6))
        {
            upload_index = 3;
        }

        srv_index += 1;
        if (!(srv_index < _count_texture_store))
        {
            srv_index = 0;
        }

        {
            std::unique_lock<std::mutex> lk(*data_device->mutex_scene_to_upload);
            if (data_device->flag_scene_to_upload == false)
            {
                data_device->condition_variable_scene_to_upload->wait(lk);
            }
            data_device->flag_scene_to_upload = false;
        }

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_upload, device index = ");
            str.append(std::to_string(data_device->device_index));
            str.append(", wait notified, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }

        if (!data_device->flag_thread_upload)
        {
            break;
        }

        ID3D12CommandAllocator* command_allocator = data_command_allocator->vector_command_allocator.at(upload_index);
        ID3D12GraphicsCommandList* command_list = data_command_list->command_list;
        ID3D12PipelineState* pso = data_pso->pso;

        command_allocator->Reset();
        command_list->Reset(command_allocator, pso);

        if (flag_created == false)
        {
            create_vertex_buffer(data_device);
            create_index_buffer(data_device);

            uint64_t counter_scene = 0;
            for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
            {
                pst_scene data_scene = it_scene->second;

                if (data_scene->device_index != data_device->device_index)
                {
                    continue;
                }

                AVFrame* frame = data_scene->vector_frame.at(srv_index);

                uint64_t width = frame->width;
                uint32_t height = frame->height;

                create_texture_yuv(data_device, width, height, counter_scene);

                vector_scene.push_back(data_scene);

                counter_scene++;
            }

            flag_created = true;
        }

        uint64_t counter_texture = 0;
        for (auto it_vector = vector_scene.begin(); it_vector != vector_scene.end(); it_vector++)
        {
            pst_scene data_scene = *it_vector;
            upload_texture_yuv(data_device, data_scene->vector_frame.at(srv_index), counter_texture, srv_index);

            counter_texture++;
        }

        command_list->Close();

        ID3D12CommandList* command_lists[] = { command_list };

        ID3D12CommandQueue* command_queue = data_command_queue->command_queue;

        command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);


        data_fence->fence_value_upload++;
        if (data_fence->fence_value_upload == UINT64_MAX)
        {
            data_fence->fence_value_upload = 0;
        }
        data_fence->fence_upload->SetEventOnCompletion(data_fence->fence_value_upload, data_fence->fence_event_upload);
        command_queue->Signal(data_fence->fence_upload, data_fence->fence_value_upload);

        WaitForSingleObject(data_fence->fence_event_upload, INFINITE);


        SetEvent(data_device->event_upload_to_device);

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_upload, device index = ");
            str.append(std::to_string(data_device->device_index));
            str.append(", set event, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
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

    pst_output data_output = nullptr;
    for (auto it_output = _map_output.begin(); it_output != _map_output.end(); it_output++)
    {
        pst_output data = it_output->second;

        if (data->device_index == data_device->device_index)
        {
            data_output = data;
            break;
        }
    }

    while (data_window->flag_thread_window)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_main_loop));

        {
            std::unique_lock<std::mutex> lk(*data_window->mutex_device_to_window);
            if (data_window->flag_device_to_window == false)
            {
                data_window->condition_variable_device_to_window->wait(lk);
            }
            data_window->flag_device_to_window = false;
        }

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_window, device index = ");
            str.append(std::to_string(data_device->device_index));
            str.append(", wait notified, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }

        if (!data_window->flag_thread_window)
        {
            break;
        }

        if (_use_wait_for_vblank)
        {
            if (_flag_set_logger)
            {
                std::string str = "";
                str.append("thread_window, device index = ");
                str.append(std::to_string(data_device->device_index));
                str.append(", WaitForVBlank Start, ");
                str.append(std::to_string(av_gettime_relative()));

                auto logger = spdlog::get(_logger_name.c_str());
                logger->debug(str.c_str());
            }

            data_output->output->WaitForVBlank();

            if (_flag_set_logger)
            {
                std::string str = "";
                str.append("thread_window, device index = ");
                str.append(std::to_string(data_device->device_index));
                str.append(", WaitForVBlank End, ");
                str.append(std::to_string(av_gettime_relative()));

                auto logger = spdlog::get(_logger_name.c_str());
                logger->debug(str.c_str());
            }
        }

        if (_use_wait_for_vblank_repeat)
        {
            if (_flag_wait_for_vblank_repeat)
            {
                data_output->output->WaitForVBlank();
            }
        }

        if (_use_wait_for_vblank_first_entry)
        {
            if (data_window->flag_first_entry)
            {
                data_output->output->WaitForVBlank();
                data_window->flag_first_entry = false;
            }
        }

        if (_nvapi_initialized)
        {
            NvAPI_Status status = NvAPI_D3D1x_Present(device, swap_chain, (UINT)1, (UINT)0);
        }
        else
        {
            swap_chain->Present(1, 0);
        }

        SetEvent(data_window->event_window_to_scene);

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_window, device index = ");
            str.append(std::to_string(data_device->device_index));
            str.append(", set event, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }
    }
}

void thread_scene(pst_scene data_scene)
{
    int32_t result = INT32_MIN;

    int64_t frame_index = -1;

    int64_t counter_scene_fps = -1;

    while (data_scene->flag_thread_scene)
    {
        frame_index += 1;
        if (!(frame_index < _count_texture_store))
        {
            frame_index = 0;
        }

        result = INT32_MIN;
        while (result < 0)
        {
            result = cpp_ffmpeg_wrapper_get_frame(data_scene->ffmpeg_instance, data_scene->vector_frame.at(frame_index));

            if (_use_play_repeat)
            {
                if (result == -2)
                {
                    _flag_repeat = true;
                    break;

                    //cpp_ffmpeg_wrapper_seek_pts(data_scene->ffmpeg_instance, 0);
                    
                    //cpp_ffmpeg_wrapper_repeat_sync_group(data_scene->ffmpeg_instance);

                    //if (_flag_set_logger)
                    //{
                    //    std::string str = "";
                    //    str.append("thread_scene, scene index = ");
                    //    str.append(std::to_string(data_scene->scene_index));
                    //    str.append(", set repeat, ");
                    //    str.append(std::to_string(av_gettime_relative()));

                    //    auto logger = spdlog::get(_logger_name.c_str());
                    //    logger->debug(str.c_str());
                    //}

                    //if (_use_wait_for_vblank_repeat)
                    //{
                    //    if (_flag_scene_repeat == false)
                    //    {
                    //        _flag_scene_repeat = true;
                    //    }
                    //}
                }
            }
        }

        if (_flag_repeat)
        {
            continue;
        }

        counter_scene_fps++;
        if (!(counter_scene_fps < _count_scene_fps))
        {
            counter_scene_fps = -1;

            result = INT32_MIN;
            while (result != 0)
            {
                result = cpp_ffmpeg_wrapper_frame_to_next_non_waiting(data_scene->ffmpeg_instance);

                if (_flag_set_logger)
                {
                    if (result == 10)
                    {
                        std::string str = "";
                        str.append("thread_scene, scene index = ");
                        str.append(std::to_string(data_scene->scene_index));
                        str.append(", frame queue empty, ");
                        str.append(std::to_string(av_gettime_relative()));

                        auto logger = spdlog::get(_logger_name.c_str());
                        logger->debug(str.c_str());
                    }
                }
            }
        }

        SetEvent(data_scene->event_scene_to_upload);

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_scene, scene index = ");
            str.append(std::to_string(data_scene->scene_index));
            str.append(", set event, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }

        {
            std::unique_lock<std::mutex> lk(*data_scene->mutex_window_to_scene);
            if (data_scene->flag_window_to_scene == false)
            {
                data_scene->condition_variable_window_to_scene->wait(lk);
            }
            data_scene->flag_window_to_scene = false;
        }

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_scene, scene index = ");
            str.append(std::to_string(data_scene->scene_index));
            str.append(", wait notified, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get(_logger_name.c_str());
            logger->debug(str.c_str());
        }
    }
}

void wait_gpus_end()
{
    for (auto it_fence = _map_fence.begin(); it_fence != _map_fence.end(); it_fence++)
    {
        pst_fence data_fence = it_fence->second;

        auto it_command_queue = _map_command_queue.find(data_fence->device_index);
        pst_command_queue data_command_queue = it_command_queue->second;
        ID3D12CommandQueue* command_queue = data_command_queue->command_queue;

        data_fence->fence_value_device++;
        if (data_fence->fence_value_device == UINT64_MAX)
        {
            data_fence->fence_value_device = 0;
        }
        data_fence->fence_device->SetEventOnCompletion(data_fence->fence_value_device, data_fence->fence_event_device);
        command_queue->Signal(data_fence->fence_device, data_fence->fence_value_device);

        WaitForSingleObject(data_fence->fence_event_device, INFINITE);
    }
}

float normalize_min_max(int min, int max, int target, int normalized_min = 0, int normalized_max = 1)
{
    return (float(target - min) / float(max - min) * (normalized_max - normalized_min)) + (normalized_min);
}

void normalize_rect(RECT base_rect, RECT target_rect, NormalizedRect& normalized_rect)
{
    normalized_rect.left = normalize_min_max(base_rect.left, base_rect.right, target_rect.left, -1, 1);
    normalized_rect.right = normalize_min_max(base_rect.left, base_rect.right, target_rect.right, -1, 1);
    normalized_rect.top = normalize_min_max(base_rect.bottom, base_rect.top, target_rect.top, -1, 1);
    normalized_rect.bottom = normalize_min_max(base_rect.bottom, base_rect.top, target_rect.bottom, -1, 1);

    if (normalized_rect.left < -1.0f)
    {
        normalized_rect.left = -1.0f;
    }

    if (normalized_rect.top > 1.0f)
    {
        normalized_rect.top = 1.0f;
    }

    if (normalized_rect.right > 1.0f)
    {
        normalized_rect.right = 1.0f;
    }

    if (normalized_rect.bottom < -1.0f)
    {
        normalized_rect.bottom = -1.0f;
    }
}

void set_logger()
{
    auto max_size = 1024 * 1024;
    auto max_files = 3;

    spdlog::init_thread_pool(8192, 1);

    _logger_name.append("dplayer_logger_");
    _logger_name.append(std::to_string(_player_sync_group_index));

    std::string log_name = "spdlogs/dplayer_log_";
    log_name.append(std::to_string(_player_sync_group_index));
    log_name.append(".txt");

    //auto logger = spdlog::rotating_logger_mt<spdlog::async_factory>(_logger_name.c_str(), log_name.c_str(), max_size, max_files);
    auto logger = spdlog::rotating_logger_mt(_logger_name.c_str(), log_name.c_str(), max_size, max_files);

    spdlog::set_level(spdlog::level::level_enum(_log_level));
    spdlog::flush_every(std::chrono::seconds(3));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l], %H:%M:%S.%e, %v");
}

// --------------------------------

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = 0;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DPLAYER));
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

    hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

    // ���� ���ڿ��� �ʱ�ȭ�մϴ�.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DPLAYER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    config_setting();

    set_logger();

    wchar_t asset_path[512];
    get_asset_path(asset_path, _countof(asset_path));
    _asset_path = asset_path;

    _mutex_map_index_buffer = new std::mutex();
    _mutex_map_index_upload_buffer = new std::mutex();
    _mutex_map_index_buffer_view = new std::mutex();

    _mutex_map_vertex_buffer = new std::mutex();
    _mutex_map_vertex_upload_buffer = new std::mutex();
    _mutex_map_vertex_buffer_view = new std::mutex();

    _mutex_map_texture = new std::mutex();
    _mutex_map_srv_handle_luminance = new std::mutex();
    _mutex_map_srv_handle_chrominance = new std::mutex();

    _mutex_map_texture_y = new std::mutex();
    _mutex_map_texture_u = new std::mutex();
    _mutex_map_texture_v = new std::mutex();

    _mutex_map_upload_texture_y = new std::mutex();
    _mutex_map_upload_texture_u = new std::mutex();
    _mutex_map_upload_texture_v = new std::mutex();

    _mutex_map_srv_handle_y = new std::mutex();
    _mutex_map_srv_handle_u = new std::mutex();
    _mutex_map_srv_handle_v = new std::mutex();


    if (_use_nvapi)
    {
        _nvapi_status = NvAPI_Initialize();
        if (_nvapi_status == NVAPI_OK)
        {
            _nvapi_initialized = true;
        }
    }

    create_factory();
    enum_adapters();
    enum_outputs();

    create_windows();
    delete_adapter_has_none_window();

    {
        create_devices();
        create_command_queues();
        create_command_allocators();

        if (_texture_type == 0)
        {
            // NV12
            create_root_sigs();
        }
        else
        {
            // YUV
            create_root_sigs_yuv();
        }

        create_pipeline_state_objects();
        create_command_lists();
        create_fences();
        create_rtv_heaps();
        create_srv_heaps();

        create_swap_chains();
        create_rtvs();
        create_viewports();
    }

    // swap_lock
    if (_nvapi_initialized)
    {
        initialize_swap_locks();
    }

    if (_flag_set_logger)
    {
        std::string str = "";
        str.append("winmain");
        str.append(", thread_start");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    _thread_vector_input = std::thread(thread_vector_input);
    _thread_packet_processing = std::thread(thread_packet_processing);
    _thread_client = std::thread(thread_client);


    if (_flag_set_logger)
    {
        std::string str = "";
        str.append("winmain");
        str.append(", thread_started");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    MSG msg{};

    // �⺻ �޽��� �����Դϴ�:
    while (_is_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_main_loop));

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    if (_thread_wait_for_multiple_objects_window_to_scene.joinable())
    {
        _flag_wait_for_multiple_objects_window_to_scene = false;
        _thread_wait_for_multiple_objects_window_to_scene.join();
    }

    if (_thread_wait_for_multiple_objects_device_to_window.joinable())
    {
        _flag_wait_for_multiple_objects_device_to_window = false;
        _thread_wait_for_multiple_objects_device_to_window.join();
    }

    if (_thread_wait_for_multiple_objects_upload_to_device.joinable())
    {
        _flag_wait_for_multiple_objects_upload_to_device = false;
        _thread_wait_for_multiple_objects_upload_to_device.join();
    }

    if (_thread_wait_for_multiple_objects_scene_to_upload.joinable())
    {
        _flag_wait_for_multiple_objects_scene_to_upload = false;
        _thread_wait_for_multiple_objects_scene_to_upload.join();
    }

    for (auto it_thread_window = _map_thread_window.begin(); it_thread_window != _map_thread_window.end();)
    {
        std::thread* data_thread_window = it_thread_window->second;

        auto it_window = _map_window.find(it_thread_window->first);
        pst_window data_window = it_window->second;

        if (data_thread_window->joinable())
        {
            data_window->flag_thread_window = false;
            data_window->condition_variable_device_to_window->notify_one();
            data_window->flag_device_to_window = true;
            data_thread_window->join();
        }

        delete data_thread_window;
        data_thread_window = nullptr;

        it_thread_window = _map_thread_window.erase(it_thread_window);
    }

    for (auto it_thread_device = _map_thread_device.begin(); it_thread_device != _map_thread_device.end();)
    {
        std::thread* data_thread_device = it_thread_device->second;

        auto it_device = _map_device.find(it_thread_device->first);
        pst_device data_device = it_device->second;

        if (data_thread_device->joinable())
        {
            data_device->flag_thread_device = false;
            data_device->condition_variable_upload_to_device->notify_one();
            data_device->flag_upload_to_device = true;
            data_thread_device->join();
        }

        delete data_thread_device;
        data_thread_device = nullptr;

        it_thread_device = _map_thread_device.erase(it_thread_device);
    }

    for (auto it_thread_upload = _map_thread_upload.begin(); it_thread_upload != _map_thread_upload.end();)
    {
        std::thread* data_thread_upload = it_thread_upload->second;

        auto it_device = _map_device.find(it_thread_upload->first);
        pst_device data_device = it_device->second;

        if (data_thread_upload->joinable())
        {
            data_device->flag_thread_upload = false;
            data_device->condition_variable_scene_to_upload->notify_one();
            data_device->flag_scene_to_upload = true;
            data_thread_upload->join();
        }

        delete data_thread_upload;
        data_thread_upload = nullptr;

        it_thread_upload = _map_thread_upload.erase(it_thread_upload);
    }

    for (auto it_thread_scene = _map_thread_scene.begin(); it_thread_scene != _map_thread_scene.end();)
    {
        std::thread* data_thread_scene = it_thread_scene->second;

        auto it_scene = _map_scene.find(it_thread_scene->first);
        pst_scene data_scene = it_scene->second;

        if (data_thread_scene->joinable())
        {
            data_scene->flag_thread_scene = false;
            data_scene->condition_variable_window_to_scene->notify_one();
            data_scene->flag_window_to_scene = true;
            data_thread_scene->join();
        }

        delete data_thread_scene;
        data_thread_scene = nullptr;

        it_thread_scene = _map_thread_scene.erase(it_thread_scene);
    }

    wait_gpus_end();

    if (_thread_vector_input.joinable())
    {
        _flag_vector_input = false;
        _thread_vector_input.join();
    }

    if (_thread_client.joinable())
    {
        _flag_thread_client = false;
        _thread_client.join();
    }

    if (_thread_packet_processing.joinable())
    {
        _flag_packet_processing = false;
        _thread_packet_processing.join();
    }

    delete_scenes();

    if (_texture_type == 0)
    {
        // NV12
        delete_textures();
    }
    else
    {
        // YUV
        delete_textures_yuv();
    }

    delete_vertex_buffers();
    delete_index_buffers();

    // swap_lock
    if (_nvapi_initialized)
    {
        delete_swap_locks();
    }

    delete_viewports();
    delete_rtvs();
    delete_swap_chains();

    delete_srv_heaps();
    delete_rtv_heaps();
    delete_fences();
    delete_command_lists();
    delete_pipeline_state_objects();
    delete_root_sigs();
    delete_command_allocators();
    delete_command_queues();
    delete_devices();

    delete_windows();
    delete_outputs();
    delete_adapters();
    delete_factory();


    if (_nvapi_initialized == true)
    {
        NvAPI_Unload();
    }

    delete _mutex_map_index_buffer;
    _mutex_map_index_buffer = nullptr;
    delete _mutex_map_index_upload_buffer;
    _mutex_map_index_upload_buffer = nullptr;
    delete _mutex_map_index_buffer_view;
    _mutex_map_index_buffer_view = nullptr;

    delete _mutex_map_vertex_buffer;
    _mutex_map_vertex_buffer = nullptr;
    delete _mutex_map_vertex_upload_buffer;
    _mutex_map_vertex_upload_buffer = nullptr;
    delete _mutex_map_vertex_buffer_view;
    _mutex_map_vertex_buffer_view = nullptr;

    delete _mutex_map_texture;
    _mutex_map_texture = nullptr;
    delete _mutex_map_srv_handle_luminance;
    _mutex_map_srv_handle_luminance = nullptr;
    delete _mutex_map_srv_handle_chrominance;
    _mutex_map_srv_handle_chrominance = nullptr;

    delete _mutex_map_texture_y;
    _mutex_map_texture_y = nullptr;
    delete _mutex_map_texture_u;
    _mutex_map_texture_u = nullptr;
    delete _mutex_map_texture_v;
    _mutex_map_texture_v = nullptr;

    delete _mutex_map_upload_texture_y;
    _mutex_map_upload_texture_y = nullptr;
    delete _mutex_map_upload_texture_u;
    _mutex_map_upload_texture_u = nullptr;
    delete _mutex_map_upload_texture_v;
    _mutex_map_upload_texture_v = nullptr;

    delete _mutex_map_srv_handle_y;
    _mutex_map_srv_handle_y = nullptr;
    delete _mutex_map_srv_handle_u;
    _mutex_map_srv_handle_u = nullptr;
    delete _mutex_map_srv_handle_v;
    _mutex_map_srv_handle_v = nullptr;

#if _DEBUG
    d3d_memory_check();
#endif

    return (int)msg.wParam;
}

void message_processing_window()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void check_ready_to_playback()
{
    if (_count_scene != _map_scene_url.size())
    {
        return;
    }

    if (_count_scene != _map_scene_coordinate.size())
    {
        return;
    }

    if (_flag_set_logger)
    {
        std::string str = "";
        str.append("thread_packet_processing");
        str.append(", check_ready_to_playback");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    start_playback();
}

void start_playback()
{
    if (_flag_set_logger)
    {
        std::string str = "";
        str.append("thread_packet_processing");
        str.append(", start_playback");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    create_scenes();

    if (_flag_set_logger)
    {
        std::string str = "";
        str.append("thread_packet_processing");
        str.append(", graphics thread start");

        auto logger = spdlog::get(_logger_name.c_str());
        logger->debug(str.c_str());
    }

    _thread_wait_for_multiple_objects_scene_to_upload = std::thread(thread_wait_for_multiple_objects, WaitType::scene_to_upload, &_flag_wait_for_multiple_objects_scene_to_upload);
    _thread_wait_for_multiple_objects_upload_to_device = std::thread(thread_wait_for_multiple_objects, WaitType::upload_to_device, &_flag_wait_for_multiple_objects_upload_to_device);
    _thread_wait_for_multiple_objects_device_to_window = std::thread(thread_wait_for_multiple_objects, WaitType::device_to_window, &_flag_wait_for_multiple_objects_device_to_window);;
    _thread_wait_for_multiple_objects_window_to_scene = std::thread(thread_wait_for_multiple_objects, WaitType::window_to_scene, &_flag_wait_for_multiple_objects_window_to_scene);

    for (auto it_window = _map_window.begin(); it_window != _map_window.end(); it_window++)
    {
        pst_window data_window = it_window->second;

        std::thread* data_thread_window = new std::thread(thread_window, data_window);

        _map_thread_window.insert({ data_window->window_index, data_thread_window });
    }

    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        std::thread* data_thread_device = nullptr;

        if (_texture_type == 0)
        {
            // NV12
            data_thread_device = new std::thread(thread_device, data_device);
        }
        else
        {
            // YUV
            data_thread_device = new std::thread(thread_device_yuv, data_device);
        }

        _map_thread_device.insert({ data_device->device_index, data_thread_device });
    }

    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        std::thread* data_thread_upload = nullptr;

        if (_texture_type == 0)
        {
            // NV12
            data_thread_upload = new std::thread(thread_upload, data_device);
        }
        else
        {
            // YUV
            data_thread_upload = new std::thread(thread_upload_yuv, data_device);
        }

        _map_thread_upload.insert({ data_device->device_index, data_thread_upload });
    }

    for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
    {
        pst_scene data_scene = it_scene->second;

        std::thread* data_thread_scene = new std::thread(thread_scene, data_scene);

        _map_thread_scene.insert({ data_scene->scene_index, data_thread_scene });
    }
}

int play_repeat_instance_recreate()
{
    play_repeat_instance_delete();

    play_repeat_instance_create();

    return 0;
}

int play_repeat_instance_delete()
{
    if (_thread_wait_for_multiple_objects_window_to_scene.joinable())
    {
        _flag_wait_for_multiple_objects_window_to_scene = false;
        _thread_wait_for_multiple_objects_window_to_scene.join();
        _flag_wait_for_multiple_objects_window_to_scene = true;
    }

    if (_thread_wait_for_multiple_objects_device_to_window.joinable())
    {
        _flag_wait_for_multiple_objects_device_to_window = false;
        _thread_wait_for_multiple_objects_device_to_window.join();
        _flag_wait_for_multiple_objects_device_to_window = true;
    }

    if (_thread_wait_for_multiple_objects_upload_to_device.joinable())
    {
        _flag_wait_for_multiple_objects_upload_to_device = false;
        _thread_wait_for_multiple_objects_upload_to_device.join();
        _flag_wait_for_multiple_objects_upload_to_device = true;
    }

    if (_thread_wait_for_multiple_objects_scene_to_upload.joinable())
    {
        _flag_wait_for_multiple_objects_scene_to_upload = false;
        _thread_wait_for_multiple_objects_scene_to_upload.join();
        _flag_wait_for_multiple_objects_scene_to_upload = true;
    }

    {
        _mutex_vector_event_scene_to_upload.lock();
        for (auto it = _vector_event_scene_to_upload.begin(); it != _vector_event_scene_to_upload.end();)
        {
            it = _vector_event_scene_to_upload.erase(it);
        }
        _mutex_vector_event_scene_to_upload.unlock();

        _mutex_vector_condition_variable_window_to_scene.lock();
        for (auto it = _vector_condition_variable_window_to_scene.begin(); it != _vector_condition_variable_window_to_scene.end();)
        {
            it = _vector_condition_variable_window_to_scene.erase(it);
        }
        _mutex_vector_condition_variable_window_to_scene.unlock();

        _mutex_vector_mutex_window_to_scene.lock();
        for (auto it = _vector_mutex_window_to_scene.begin(); it != _vector_mutex_window_to_scene.end();)
        {
            it = _vector_mutex_window_to_scene.erase(it);
        }
        _mutex_vector_mutex_window_to_scene.unlock();

        _mutex_vector_flag_window_to_scene.lock();
        for (auto it = _vector_flag_window_to_scene.begin(); it != _vector_flag_window_to_scene.end();)
        {
            it = _vector_flag_window_to_scene.erase(it);
        }
        _mutex_vector_flag_window_to_scene.unlock();
    }

    for (auto it_thread_window = _map_thread_window.begin(); it_thread_window != _map_thread_window.end();)
    {
        std::thread* data_thread_window = it_thread_window->second;

        auto it_window = _map_window.find(it_thread_window->first);
        pst_window data_window = it_window->second;

        if (data_thread_window->joinable())
        {
            data_window->flag_thread_window = false;
            data_window->mutex_device_to_window->lock();
            data_window->condition_variable_device_to_window->notify_one();
            data_window->flag_device_to_window = true;
            data_window->mutex_device_to_window->unlock();
            data_thread_window->join();

            data_window->mutex_device_to_window->lock();
            data_window->flag_thread_window = true;
            data_window->flag_device_to_window = false;

            data_window->flag_first_entry = true;
            data_window->mutex_device_to_window->unlock();
        }

        delete data_thread_window;
        data_thread_window = nullptr;

        it_thread_window = _map_thread_window.erase(it_thread_window);
    }

    for (auto it_thread_device = _map_thread_device.begin(); it_thread_device != _map_thread_device.end();)
    {
        std::thread* data_thread_device = it_thread_device->second;

        auto it_device = _map_device.find(it_thread_device->first);
        pst_device data_device = it_device->second;

        if (data_thread_device->joinable())
        {
            data_device->flag_thread_device = false;
            data_device->mutex_upload_to_device->lock();
            data_device->condition_variable_upload_to_device->notify_one();
            data_device->flag_upload_to_device = true;
            data_device->mutex_upload_to_device->unlock();
            data_thread_device->join();
            
            data_device->mutex_upload_to_device->lock();
            data_device->flag_thread_device = true;
            data_device->flag_upload_to_device = false;
            data_device->mutex_upload_to_device->unlock();
        }

        delete data_thread_device;
        data_thread_device = nullptr;

        it_thread_device = _map_thread_device.erase(it_thread_device);
    }

    for (auto it_thread_upload = _map_thread_upload.begin(); it_thread_upload != _map_thread_upload.end();)
    {
        std::thread* data_thread_upload = it_thread_upload->second;

        auto it_device = _map_device.find(it_thread_upload->first);
        pst_device data_device = it_device->second;

        if (data_thread_upload->joinable())
        {
            data_device->flag_thread_upload = false;
            data_device->mutex_scene_to_upload->lock();
            data_device->condition_variable_scene_to_upload->notify_one();
            data_device->flag_scene_to_upload = true;
            data_device->mutex_scene_to_upload->unlock();
            data_thread_upload->join();
            
            data_device->mutex_scene_to_upload->lock();
            data_device->flag_thread_upload = true;
            data_device->flag_scene_to_upload = false;
            data_device->mutex_scene_to_upload->unlock();
        }

        delete data_thread_upload;
        data_thread_upload = nullptr;

        it_thread_upload = _map_thread_upload.erase(it_thread_upload);
    }

    for (auto it_thread_scene = _map_thread_scene.begin(); it_thread_scene != _map_thread_scene.end();)
    {
        std::thread* data_thread_scene = it_thread_scene->second;

        auto it_scene = _map_scene.find(it_thread_scene->first);
        pst_scene data_scene = it_scene->second;

        if (data_thread_scene->joinable())
        {
            data_scene->flag_thread_scene = false;
            data_scene->mutex_window_to_scene->lock();
            data_scene->condition_variable_window_to_scene->notify_one();
            data_scene->flag_window_to_scene = true;
            data_scene->mutex_window_to_scene->unlock();
            data_thread_scene->join();

            data_scene->mutex_window_to_scene->lock();
            data_scene->flag_thread_scene = true;
            data_scene->flag_window_to_scene = false;
            data_scene->mutex_window_to_scene->unlock();
        }

        delete data_thread_scene;
        data_thread_scene = nullptr;

        it_thread_scene = _map_thread_scene.erase(it_thread_scene);
    }

    delete_scenes();

    if (_texture_type == 0)
    {
        // NV12
        delete_textures();
    }
    else
    {
        // YUV
        delete_textures_yuv();
    }

    delete_vertex_buffers();
    delete_index_buffers();

    return 0;
}

int play_repeat_instance_create()
{
    start_playback();

    return 0;
}