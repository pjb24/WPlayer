// WPlayer.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "WPlayer.h"

#include "CommonHeaders.h"
#include "GraphicsHeaders.h"

#include "CppSocketAPI.h"
#include "PacketDefine.h"
#include "ApiFunctionStructures.h"
#pragma comment(lib, "CppSocket.lib")

#include "CppFFmpegWrapperAPI.h"
#include "CppFFmpegWrapperCallbackStruct.h"
#pragma comment(lib, "CppFFmpegWrapper.lib")

#include "nvapi.h"
#pragma comment(lib, "nvapi64.lib")

#if _DEBUG
#include "dxgidebug.h"
#endif // _DEBUG

NvAPI_Status _nvapi_status = NVAPI_OK;

bool _present_barrier_supported = true;
bool _disable_present_barrier = true;

constexpr u32 frame_buffer_count = 3;
constexpr u32 texture_resource_count = 3;

int _test_window_count = 0;    // 0이면 기본 사용, 0이 아니면 개수만큼 window 생성

bool _repeat_play_flag = true;

bool _texture_create_each_panel = true;   // 텍스처를 패널마다 분리해서 만들기

bool _window_create_position_shift_up = false;  // 윈도우를 위로 이동해서 생성

bool _create_one_swapchain_for_each_adapter = false;    // 1 Adapter 당 1 SwapChain을 생성

int _create_one_swapchain_for_each_adapter_window_width = 0;    // create_one_swapchain_for_each_adapter 옵션을 사용할 때 생성될 window의 width
int _create_one_swapchain_for_each_adapter_window_height = 0;   // create_one_swapchain_for_each_adapter 옵션을 사용할 때 생성될 window의 height

bool _create_one_swapchain_for_each_adapter_without_control_output = false;    // 1 Adapter 당 1 SwapChain을 생성, 컨트롤 모니터 1개를 윈도우 생성에서 제외함.

// 윈도우 생성에서 제외할 모니터의 좌표 값들
int _create_one_swapchain_for_each_adapter_without_control_output_excluded_window_left = 0;
int _create_one_swapchain_for_each_adapter_without_control_output_excluded_window_top = 0;
int _create_one_swapchain_for_each_adapter_without_control_output_excluded_window_right = 0;
int _create_one_swapchain_for_each_adapter_without_control_output_excluded_window_bottom = 0;

// play_sync_group 명령을 수행할 때 Frame의 번호를 맞추어서 재생하도록 하는 Flag. 0: 사용 안함, 1 : 사용함
bool _sync_group_frame_numbering = false;

// 하드웨어 가속 디코딩 사용 옵션. 0: 사용 안함, 1: 사용함
bool _hw_accel = false;
// 하드웨어 가속 디코딩에 사용할 HWDeviceType. 2: CUDA, 4: DXVA2, 5: QSV, 7: D3D11VA
int _hw_accel_device_type = 0;
// 하드웨어 가속 디코딩에 사용할 Adapter의 번호.
int _hw_accel_adapter_index = 0;

// scene을 자른 좌표에 대해서 보정을 수행하는 옵션.
bool _scene_panel_coordinate_correction = false;


std::string _ip;
uint16_t _port;


struct RemoveScene
{
    u32 scene_index = u32_invalid_id;
    bool all_panel_removed_flag = false;
};

struct NormalizedRect
{
    float left;
    float top;
    float right;
    float bottom;
};

struct Panel
{
    u32 adapter_index = -1;
    s32 vertex_index = -1;
    s32 texture_index = -1;
    s32 output_index = -1;   // panel이 그려질 output의 index
    s32 output_frame_index = 0; // FFmpeg의 AVFrame 패킷 Index
    RECT rect;
    NormalizedRect normalized_rect;
    NormalizedRect normalized_uv;
    bool normalize_uv_flag = false;

    uint8_t* separated_frame_data_y = nullptr;
    uint8_t* separated_frame_data_u = nullptr;
    uint8_t* separated_frame_data_v = nullptr;
    
    int separated_frame_linesize_y = 0;
    int separated_frame_linesize_u = 0;
    int separated_frame_linesize_v = 0;

    int separated_frame_height = 0;
};

struct Scene
{
    u32 scene_index = u32_invalid_id;

    std::vector<Panel*> panel_list;
    int64_t pts = -1;

    RECT rect;
    bool using_flag = false;
    std::map<s32, bool> texture_upload_to_adapter_flag_map; // adapter_index, bool

    u32 sync_group_index = u32_invalid_id;
    u16 sync_group_count = 0;
};

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 uv;
};

struct output_data
{
    IDXGIOutput* output = nullptr;
    DXGI_OUTPUT_DESC output_desc{};
    HWND handle = nullptr;
    IDXGISwapChain3* swap_chain = nullptr;
    u32 frame_index = 0;    // Current Back Buffer Index

    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle{};
    std::vector<ID3D12Resource*> rtv_view_list;

    u64 fence_values[frame_buffer_count];
    ID3D12Fence* fence = nullptr;
    HANDLE fence_event = nullptr;

    D3D12_VIEWPORT viewport;
    D3D12_RECT scissor_rect;

    s32 output_index = -1;

    NvPresentBarrierClientHandle present_barrier_client = nullptr;
    ID3D12Fence* present_barrier_fence = nullptr;
    bool present_barrier_joined = false;
    _NV_PRESENT_BARRIER_FRAME_STATISTICS present_barrier_frame_stats;

    RECT create_one_swapchain_for_each_adapter_rect = { 0, 0, 0, 0 };
    RECT create_one_swapchain_for_each_adapter_without_control_output_rect = { 0, 0, 0, 0 };
};

enum class deferred_type : s32
{
    none = -1,
    vertex_buffer = 0,
    vertex_upload_buffer = 1,
    texture_y = 2,
    texture_u = 3,
    texture_v = 4,
    upload_texture_y = 5,
    upload_texture_u = 6,
    upload_texture_v = 7,
};

struct deferred_free_object
{
    ID3D12Resource* resource;
    s32 index = -1;
    deferred_type type;
    bool free_flag = false;
};

struct graphics_data
{
    IDXGIAdapter1* adapter = nullptr;
    ID3D12Device* device = nullptr;
    ID3D12CommandQueue* cmd_queue = nullptr;
    std::vector<output_data*> output_list;  // Adapter에 연결된 Output List

    u32 rtv_descriptor_size = 0;
    ID3D12DescriptorHeap* rtv_heaps = nullptr;

    u32 srv_descriptor_size = 0;
    ID3D12DescriptorHeap* srv_heaps = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle{};

    std::vector<ID3D12CommandAllocator*> cmd_allocator_list;

    ID3D12RootSignature* root_sig = nullptr;
    ID3D12PipelineState* pso = nullptr;
    ID3D12GraphicsCommandList* cmd_list = nullptr;

    // vertex_index, object
    std::map<s32, ID3D12Resource*> vertex_buffer_map;
    std::map<s32, ID3D12Resource*> vertex_upload_buffer_map;
    std::map<s32, D3D12_VERTEX_BUFFER_VIEW> vertex_buffer_view_map;

    ID3D12Resource* index_buffer = nullptr;
    ID3D12Resource* index_upload_buffer = nullptr;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view;
    u32 index_count = 0;

    // texture_index, object
    std::map<s32, ID3D12Resource*> texture_map_y[frame_buffer_count];
    std::map<s32, ID3D12Resource*> texture_map_u[frame_buffer_count];
    std::map<s32, ID3D12Resource*> texture_map_v[frame_buffer_count];
    std::map<s32, ID3D12Resource*> upload_texture_map_y[frame_buffer_count];
    std::map<s32, ID3D12Resource*> upload_texture_map_u[frame_buffer_count];
    std::map<s32, ID3D12Resource*> upload_texture_map_v[frame_buffer_count];

    bool create_index_buffer_flag = true;

    u32 adapter_index = 0;

    std::deque<s32> free_vertex_queue;  // vertex_index
    s32 next_vertex_index = 0;

    std::deque<s32> free_texture_queue; // texture_index
    s32 next_texture_index = 0;

    bool deferred_free_flag[frame_buffer_count];
    std::vector<deferred_free_object> deferred_free_object_list[frame_buffer_count];

    bool present_barrier_supported = false;
};

#pragma region Graphics
std::vector<graphics_data*> _graphics_data_list;

constexpr u32 srv_descriptor_count = 4096;

IDXGIFactory4*  _factory = nullptr;

std::vector<Scene*> _graphics_insert_list;
std::deque<RemoveScene> _graphics_remove_queue; // scene_index, all_panel_removed_flag
std::mutex _graphics_remove_mutex;  // _graphics_remove_queue의 mutex

bool ready = false;
std::wstring _asset_path;
const float _clear_color[] = { 0.0f, 0.2f, 0.4f, 1.0f };

std::vector<Scene*> _graphics_scene_list;

std::deque<u32> _free_scene_queue;  // scene_index
u32 _next_scene_index = 0;

void get_asset_path(wchar_t * path, u32 path_size);
std::wstring get_asset_full_path(LPCWSTR asset_name);
u32 enum_adapters();
u32 delete_adapters();
u32 enum_output_list();
u32 delete_output_list();
u32 create_factory();
u32 delete_factory();
u32 create_devices();
u32 delete_devices();
u32 create_command_queues();
u32 delete_command_queues();
u32 create_window(WCHAR* window_class, WCHAR* title, HINSTANCE instance, RECT rect, void * data /* = nullptr */, HWND & handle);
u32 create_swap_chains();
u32 delete_swap_chains();
u32 create_rtv_heaps();
u32 delete_rtv_heaps();
u32 create_srv_heaps();
u32 delete_srv_heaps();
u32 create_root_signatures();
u32 delete_root_signatures();
u32 create_psos();
u32 delete_psos();
u32 create_command_lists();
u32 delete_command_lists();
u32 create_index_buffer(graphics_data* data);
u32 delete_index_buffer();
u32 create_vertex_buffer(graphics_data* data, s32 vertex_index, NormalizedRect normalized_rect, NormalizedRect normalized_uv, u32 scene_index);
u32 delete_vertex_buffer_list();
u32 create_texture(graphics_data* data, u32 width, u32 height, s32 texture_index, u32 scene_index);
u32 delete_textures();
u32 create_fences();
u32 delete_fences();
u32 create_viewports();
u32 wait_for_gpu(ID3D12CommandQueue* cmd_queue, output_data* data);
u32 wait_for_gpus();
u32 move_to_next_frame(ID3D12CommandQueue* cmd_queue, output_data* data);
u32 populate_command_list(graphics_data* data);
u32 render();
u32 create_scene_data(RECT rect, u32 sync_group_index = u32_invalid_id, u16 sync_group_count = 0);
u32 delete_scene_data(u32 scene_index);
u32 delete_scene_datas();
u32 upload_texture(graphics_data* data, AVFrame* frame, s32 target_texture_index, s32 output_frame_index);
u32 upload_texture_each_panel(graphics_data* data, AVFrame* frame, s32 output_frame_index, Panel* panel);
float normalize_min_max(int min, int max, int target, int normalized_min, int normalized_max);
u32 normalize_rect(RECT base_rect, RECT target_rect, NormalizedRect& normalized_rect);
u32 normalize_uv(RECT base_rect, RECT target_rect, NormalizedRect& normalized_uv);
u32 deferred_free_processing(u32 back_buffer_index);

u32 delete_present_barriers();
#if _DEBUG
void d3d_memory_check();
#endif
#pragma endregion

#pragma region TCP Server
void* _server = nullptr;
std::thread _tcp_thread;
bool _tcp_server_flag = true;

void server_thread();
#pragma endregion

#pragma region Packet Processing
// first : packet_data, second : connection
// CppSocket의 콜백 명령 저장 큐
std::deque<std::pair<void*, void*>> _tcp_processing_command_queue;
constexpr u32 _sleep_time_tcp_processing = 10;
std::thread _tcp_processing_thread;
bool _tcp_processing_flag = true;
std::mutex _tcp_processing_mutex;
void tcp_processing_thread();
void callback_data_connection_server(void* data, void* connection);
#pragma endregion


#pragma region FFmpegWrapper
using FFmpegProcessingCommand = ffmpeg_wrapper_callback_data;

struct FFmpegInstanceData
{
    void* ffmpeg_instance = nullptr;
    uint32_t sync_group_index = u32_invalid_id;
    uint16_t sync_group_count = 0;
    bool render_ready = false;
    bool repeat_command_send_flag = false;
};

// CppFFmpegWrapper의 콜백 명령 저장 큐
std::deque<FFmpegProcessingCommand> _ffmpeg_processing_command_queue;
std::mutex _ffmpeg_processing_mutex;    // _ffmpeg_processing_command_queue의 mutex

constexpr u32 _sleep_time_ffmpeg_processing = 10;

std::map<u32, FFmpegInstanceData> _ffmpeg_data_map;  // scene_index, ffmpeg_instance

std::thread _ffmpeg_processing_thread;
bool _ffmpeg_processing_flag = true;
std::mutex _ffmpeg_data_mutex;  // _ffmpeg_data_map의 mutex

void ffmpeg_processing_thread();
void callback_ffmpeg_wrapper_ptr(void* param);

void delete_ffmpeg_instances();

struct SyncGroupCounter
{
    u16 sync_group_count = 0;
    u16 sync_group_input_count = 0;
};

std::map<u32, SyncGroupCounter> _sync_group_counter_map_play;
std::map<u32, SyncGroupCounter> _sync_group_counter_map_repeat;
std::map<u32, SyncGroupCounter> _sync_group_counter_map_repeat_completed;
std::map<u32, SyncGroupCounter> _sync_group_counter_map_stop;
std::mutex  _sync_group_counter_mutex_stop;

struct FrameSyncData
{
    std::map<u32, u32> inner_map;   // key: scene_index, value: scene에 대한 frame 번호
    u32 frame_number = 0;   // sync_group에 대한 번호
};

std::map<u32, FrameSyncData> _sync_group_counter_map_frame_numbering;   // key: sync_group_index
std::mutex  _sync_group_counter_mutex_frame_numbering;
#pragma endregion

void config_setting();

// CppFFmpegWrapper의 콜백 명령 처리
void ffmpeg_processing_thread()
{
    while (_ffmpeg_processing_flag)
    {
        bool ffmpeg_processing_command_queue_is_empty = false;

        {
            std::lock_guard<std::mutex> lk(_ffmpeg_processing_mutex);
            ffmpeg_processing_command_queue_is_empty = _ffmpeg_processing_command_queue.empty();
        }

        if (ffmpeg_processing_command_queue_is_empty)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_ffmpeg_processing));
            continue;
        }

        FFmpegProcessingCommand data_command;
        {
            std::lock_guard<std::mutex> lk(_ffmpeg_processing_mutex);
            data_command = _ffmpeg_processing_command_queue.front();
            _ffmpeg_processing_command_queue.pop_front();
        }

        switch ((command_type)data_command.command)
        {
        case command_type::play:
        {
            // Decoding이 완료되면 수신함.

            for (auto it = _graphics_insert_list.begin(); it != _graphics_insert_list.end();)
            {
                if ((*it)->scene_index == data_command.scene_index)
                {
                    _graphics_scene_list.push_back(*it);
                    _graphics_insert_list.erase(it);
                    break;
                }

                it++;
            }

            if (data_command.connection != nullptr)
            {
                cppsocket_struct_server_send_play data{};
                data.scene_index = data_command.scene_index;
                data.result = data_command.result;
                data.left = data_command.left;
                data.top = data_command.top;
                data.width = data_command.width;
                data.height = data_command.height;
                data.url_size = data_command.url_size;
                memcpy(data.url, data_command.url, data_command.url_size);

                cppsocket_server_send_play(_server, data_command.connection, data);
            }
        }
        break;
        case command_type::pause:
        {
            if (data_command.connection != nullptr)
            {
                cppsocket_struct_server_send_pause data{};
                data.scene_index = data_command.scene_index;
                data.result = data_command.result;

                cppsocket_server_send_pause(_server, data_command.connection, data);
            }
        }
        break;
        case command_type::stop:
        {
            void* ffmpeg_instance = nullptr;
            {
                std::lock_guard<std::mutex> lock(_ffmpeg_data_mutex);
                ffmpeg_instance = _ffmpeg_data_map.find(data_command.scene_index)->second.ffmpeg_instance;
                cpp_ffmpeg_wrapper_shutdown(ffmpeg_instance);
                cpp_ffmpeg_wrapper_delete(ffmpeg_instance);
                _ffmpeg_data_map.erase(data_command.scene_index);
            }
            delete_scene_data(data_command.scene_index);

            if (data_command.connection != nullptr)
            {
                cppsocket_struct_server_send_stop data{};
                data.scene_index = data_command.scene_index;
                data.result = data_command.result;

                cppsocket_server_send_stop(_server, data_command.connection, data);
            }
        }
        break;
        case command_type::move:
        {

        }
        break;
        case command_type::jump_forward:
        {
            if (data_command.connection != nullptr)
            {
                cppsocket_struct_server_send_jump_forward data{};
                data.scene_index = data_command.scene_index;
                data.result = data_command.result;

                cppsocket_server_send_jump_forward(_server, data_command.connection, data);
            }
        }
        break;
        case command_type::jump_backwards:
        {
            if (data_command.connection != nullptr)
            {
                cppsocket_struct_server_send_jump_backwards data{};
                data.scene_index = data_command.scene_index;
                data.result = data_command.result;

                cppsocket_server_send_jump_backwards(_server, data_command.connection, data);
            }
        }
        break;
        case command_type::play_sync_group:
        {
            SyncGroupCounter sync_group_counter{};

            std::map<u32, SyncGroupCounter>::iterator it = _sync_group_counter_map_play.find(data_command.sync_group_index);
            if (it == _sync_group_counter_map_play.end())
            {
                sync_group_counter.sync_group_count = data_command.sync_group_count;
                sync_group_counter.sync_group_input_count++;

                _sync_group_counter_map_play.insert({ data_command.sync_group_index, sync_group_counter });
            }
            else
            {
                it->second.sync_group_input_count++;
                sync_group_counter = it->second;
            }

            if (sync_group_counter.sync_group_count == sync_group_counter.sync_group_input_count)
            {
                for (auto it = _graphics_insert_list.begin(); it != _graphics_insert_list.end();)
                {
                    if ((*it)->sync_group_index == data_command.sync_group_index)
                    {
                        std::lock_guard<std::mutex> lock(_ffmpeg_data_mutex);

                        void* ffmpeg_instance = _ffmpeg_data_map.find((*it)->scene_index)->second.ffmpeg_instance;
                        cpp_ffmpeg_wrapper_set_sync_group_time_started(ffmpeg_instance);

                        _graphics_scene_list.push_back(*it);
                        it = _graphics_insert_list.erase(it);
                    }
                    else
                    {
                        it++;
                    }
                }

                _sync_group_counter_map_play.erase(data_command.sync_group_index);
            }

            if (data_command.connection != nullptr)
            {
                cppsocket_struct_server_send_play_sync_group data{};
                data.scene_index = data_command.scene_index;
                data.result = data_command.result;
                data.left = data_command.left;
                data.top = data_command.top;
                data.width = data_command.width;
                data.height = data_command.height;
                data.sync_group_index = data_command.sync_group_index;
                data.sync_group_count = sync_group_counter.sync_group_input_count;
                data.url_size = data_command.url_size;
                memcpy(data.url, data_command.url, data_command.url_size);

                cppsocket_server_send_play_sync_group(_server, data_command.connection, data);
            }
        }
        break;
        case command_type::pause_sync_group:
        {

            if (data_command.connection != nullptr)
            {
                cppsocket_struct_server_send_pause_sync_group data{};
                data.result = data_command.result;
                data.sync_group_index = data_command.sync_group_index;

                cppsocket_server_send_pause_sync_group(_server, data_command.connection, data);
            }
        }
        break;
        case command_type::stop_sync_group:
        {
            std::lock_guard<std::mutex> lk(_sync_group_counter_mutex_stop);

            SyncGroupCounter sync_group_counter{};

            std::map<u32, SyncGroupCounter>::iterator it = _sync_group_counter_map_stop.find(data_command.sync_group_index);
            if (it == _sync_group_counter_map_stop.end())
            {
                break;
            }
            
            it->second.sync_group_input_count++;
            sync_group_counter = it->second;

            if (sync_group_counter.sync_group_count == sync_group_counter.sync_group_input_count)
            {
                if (data_command.connection != nullptr)
                {
                    cppsocket_struct_server_send_stop_sync_group data{};
                    data.result = data_command.result;
                    data.sync_group_index = data_command.sync_group_index;

                    cppsocket_server_send_stop_sync_group(_server, data_command.connection, data);
                }

                void* ffmpeg_instance = nullptr;
                {
                    std::lock_guard<std::mutex> lock(_ffmpeg_data_mutex);

                    auto it = _ffmpeg_data_map.begin();
                    for (; it != _ffmpeg_data_map.end();)
                    {
                        if (it->second.sync_group_index == data_command.sync_group_index)
                        {
                            ffmpeg_instance = it->second.ffmpeg_instance;
                            cpp_ffmpeg_wrapper_shutdown(ffmpeg_instance);
                            cpp_ffmpeg_wrapper_delete(ffmpeg_instance);
                            delete_scene_data(it->first);
                            it = _ffmpeg_data_map.erase(it);
                        }
                        else
                        {
                            it++;
                        }
                    }
                }

                _sync_group_counter_map_stop.erase(data_command.sync_group_index);
            }

            {
                std::lock_guard<std::mutex> lk(_sync_group_counter_mutex_frame_numbering);
                _sync_group_counter_map_frame_numbering.erase(data_command.sync_group_index);
            }
        }
        break;
        case command_type::sync_group_frame_numbering:
        {
            std::lock_guard<std::mutex> lk(_sync_group_counter_mutex_frame_numbering);

            std::map<u32, FrameSyncData>::iterator it = _sync_group_counter_map_frame_numbering.find(data_command.sync_group_index);
            if (it == _sync_group_counter_map_frame_numbering.end())
            {
                FrameSyncData frame_sync_data;
                frame_sync_data.frame_number = 0;
                frame_sync_data.inner_map.insert({ data_command.scene_index, 0 });
                _sync_group_counter_map_frame_numbering.insert({ data_command.sync_group_index, frame_sync_data });
            }
            else
            {
                FrameSyncData* frame_sync_data = &it->second;
                std::map<u32, u32>::iterator it_inner_map = frame_sync_data->inner_map.find(data_command.scene_index);
                if (it_inner_map == frame_sync_data->inner_map.end())
                {
                    frame_sync_data->inner_map.insert({ data_command.scene_index, 0 });
                }
                else
                {
                    it_inner_map->second++;
                }

                u32 temp_count = 0;

                // key: scene_index, value: scene에 대한 frame 번호
                auto it_counter = frame_sync_data->inner_map.begin();
                for (; it_counter != frame_sync_data->inner_map.end(); it_counter++)
                {
                    if (frame_sync_data->frame_number <= it_counter->second)
                    {
                        temp_count++;
                    }
                }

                if (temp_count == data_command.sync_group_count)
                {
                    frame_sync_data->frame_number++;

                    std::lock_guard<std::mutex> lock(_ffmpeg_data_mutex);

                    auto it_ffmpeg_data = _ffmpeg_data_map.begin();
                    for (; it_ffmpeg_data != _ffmpeg_data_map.end(); it_ffmpeg_data++)
                    {
                        if (it_ffmpeg_data->second.sync_group_index == data_command.sync_group_index)
                        {
                            cpp_ffmpeg_wrapper_frame_numbering(it_ffmpeg_data->second.ffmpeg_instance);
                        }
                    }
                }
            }
        }
        break;
        case command_type::seek_repeat_self_sync_group:
        {
            SyncGroupCounter sync_group_counter{};

            std::map<u32, SyncGroupCounter>::iterator it = _sync_group_counter_map_repeat_completed.find(data_command.sync_group_index);
            if (it == _sync_group_counter_map_repeat_completed.end())
            {
                sync_group_counter.sync_group_count = data_command.sync_group_count;
                sync_group_counter.sync_group_input_count++;

                _sync_group_counter_map_repeat_completed.insert({ data_command.sync_group_index, sync_group_counter });
            }
            else
            {
                it->second.sync_group_input_count++;
                sync_group_counter = it->second;
            }

            if (sync_group_counter.sync_group_input_count == sync_group_counter.sync_group_count)
            {
                std::lock_guard<std::mutex> lk(_ffmpeg_data_mutex);

                auto it_ffmpeg_data = _ffmpeg_data_map.begin();
                for (; it_ffmpeg_data != _ffmpeg_data_map.end(); it_ffmpeg_data++)
                {
                    if (it_ffmpeg_data->second.sync_group_index == data_command.sync_group_index)
                    {
                        cpp_ffmpeg_wrapper_set_sync_group_time_started(it_ffmpeg_data->second.ffmpeg_instance);
                        cpp_ffmpeg_wrapper_unset_repeat_flag(it_ffmpeg_data->second.ffmpeg_instance);
                    }
                }

                _sync_group_counter_map_repeat_completed.erase(data_command.sync_group_index);
            }
        }
        break;
        default:
            break;
        }
    }
}

// CppSocket의 콜백 명령 처리
void tcp_processing_thread()
{
    while (_tcp_processing_flag)
    {

        bool tcp_processing_command_is_empty = false;
        {
            std::lock_guard<std::mutex> lk(_tcp_processing_mutex);
            tcp_processing_command_is_empty = _tcp_processing_command_queue.empty();
        }

        if (tcp_processing_command_is_empty)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_tcp_processing));
            continue;
        }

        // first : packet_data, second : connection
        std::pair<void*, void*> data_pair;
        {
            std::lock_guard<std::mutex> lk(_tcp_processing_mutex);
            data_pair = _tcp_processing_command_queue.front();
            _tcp_processing_command_queue.pop_front();
        }

        packet_header* header = (packet_header*)data_pair.first;

        void* ffmpeg_instance = nullptr;

        switch (header->cmd)
        {
        case command_type::play:
        {
            packet_play_from_client* packet = (packet_play_from_client*)data_pair.first;
            ffmpeg_instance = cpp_ffmpeg_wrapper_create();

            cpp_ffmpeg_wrapper_initialize(ffmpeg_instance, callback_ffmpeg_wrapper_ptr);

            if (_hw_accel == true)
            {
                cpp_ffmpeg_wrapper_set_hw_decode(ffmpeg_instance);
                cpp_ffmpeg_wrapper_set_hw_device_type(ffmpeg_instance, _hw_accel_device_type);
            }

            cpp_ffmpeg_wrapper_set_file_path(ffmpeg_instance, packet->url);
            if (cpp_ffmpeg_wrapper_open_file(ffmpeg_instance) != 0)
            {
                // TODO: open_file 실패
                // 1, file_not_exist
                // 7, file_path_unsetted

                cpp_ffmpeg_wrapper_shutdown(ffmpeg_instance);
                cpp_ffmpeg_wrapper_delete(ffmpeg_instance);

                cppsocket_struct_server_send_play data{};
                data.scene_index = u32_invalid_id;
                data.result = (u16)packet_result::fail;
                data.left = packet->left;
                data.top = packet->top;
                data.width = packet->width;
                data.height = packet->height;
                data.url_size = packet->url_size;
                memcpy(data.url, packet->url, packet->url_size);

                cppsocket_server_send_play(_server, data_pair.second, data);
                break;
            }

            RECT rect = { packet->left, packet->top, packet->left + packet->width, packet->top + packet->height };

            u32 scene_index = create_scene_data(rect);

            FFmpegInstanceData ffmpeg_instance_data;
            ffmpeg_instance_data.ffmpeg_instance = ffmpeg_instance;

            _ffmpeg_data_map.insert({ scene_index, ffmpeg_instance_data });
            cpp_ffmpeg_wrapper_set_scene_index(ffmpeg_instance, scene_index);

            cpp_ffmpeg_wrapper_set_rect(ffmpeg_instance, rect);

            cpp_ffmpeg_wrapper_play_start(ffmpeg_instance, data_pair.second);
        }
        break;
        case command_type::pause:
        {
            packet_pause_from_client* packet = (packet_pause_from_client*)data_pair.first;

            auto it = _ffmpeg_data_map.find(packet->scene_index);
            if (it == _ffmpeg_data_map.end())
            {
                cppsocket_struct_server_send_pause data{};
                data.scene_index = packet->scene_index;
                data.result = (u16)packet_result::fail;

                cppsocket_server_send_pause(_server, data_pair.second, data);
                break;
            }

            ffmpeg_instance = it->second.ffmpeg_instance;

            cpp_ffmpeg_wrapper_play_pause(ffmpeg_instance, data_pair.second);
        }
        break;
        case command_type::stop:
        {
            packet_stop_from_client* packet = (packet_stop_from_client*)data_pair.first;

            auto it = _ffmpeg_data_map.find(packet->scene_index);
            if (it == _ffmpeg_data_map.end())
            {
                cppsocket_struct_server_send_stop data{};
                data.scene_index = packet->scene_index;
                data.result = (u16)packet_result::fail;

                cppsocket_server_send_stop(_server, data_pair.second, data);
                break;
            }

            ffmpeg_instance = it->second.ffmpeg_instance;

            cpp_ffmpeg_wrapper_play_stop(ffmpeg_instance, data_pair.second);
        }
        break;
        case command_type::move:
        {
            packet_move_from_client* packet = (packet_move_from_client*)data_pair.first;

            auto it = _ffmpeg_data_map.find(packet->scene_index);
            if (it == _ffmpeg_data_map.end())
            {
                cppsocket_struct_server_send_move data{};
                data.scene_index = packet->scene_index;
                data.result = (u16)packet_result::fail;

                cppsocket_server_send_move(_server, data_pair.second, data);
                break;
            }

            ffmpeg_instance = it->second.ffmpeg_instance;
        }
        break;
        case command_type::jump_forward:
        {
            packet_jump_forward_from_client* packet = (packet_jump_forward_from_client*)data_pair.first;

            auto it = _ffmpeg_data_map.find(packet->scene_index);
            if (it == _ffmpeg_data_map.end())
            {
                cppsocket_struct_server_send_jump_forward data{};
                data.scene_index = packet->scene_index;
                data.result = (u16)packet_result::fail;

                cppsocket_server_send_jump_forward(_server, data_pair.second, data);
                break;
            }

            ffmpeg_instance = it->second.ffmpeg_instance;

            cpp_ffmpeg_wrapper_jump_forward(ffmpeg_instance, data_pair.second);
        }
        break;
        case command_type::jump_backwards:
        {
            packet_jump_backwards_from_client* packet = (packet_jump_backwards_from_client*)data_pair.first;

            auto it = _ffmpeg_data_map.find(packet->scene_index);
            if (it == _ffmpeg_data_map.end())
            {
                cppsocket_struct_server_send_jump_backwards data{};
                data.scene_index = packet->scene_index;
                data.result = (u16)packet_result::fail;

                cppsocket_server_send_jump_backwards(_server, data_pair.second, data);
                break;
            }

            ffmpeg_instance = it->second.ffmpeg_instance;

            cpp_ffmpeg_wrapper_jump_backwards(ffmpeg_instance, data_pair.second);
        }
        break;
        case command_type::seek_repeat_self:
        {
            packet_seek_repeat_self* packet = (packet_seek_repeat_self*)data_pair.first;

            auto it = _ffmpeg_data_map.find(packet->scene_index);
            if (it == _ffmpeg_data_map.end())
            {
                break;
            }

            if (it->second.sync_group_index != u32_invalid_id)
            {
                SyncGroupCounter sync_group_counter{};

                std::map<u32, SyncGroupCounter>::iterator it_sync_group_repeat = _sync_group_counter_map_repeat.find(it->second.sync_group_index);
                if (it_sync_group_repeat == _sync_group_counter_map_repeat.end())
                {
                    sync_group_counter.sync_group_count = it->second.sync_group_count;
                    sync_group_counter.sync_group_input_count++;

                    _sync_group_counter_map_repeat.insert({ it->second.sync_group_index, sync_group_counter });
                }
                else
                {
                    it_sync_group_repeat->second.sync_group_input_count++;
                    sync_group_counter = it_sync_group_repeat->second;
                }

                cpp_ffmpeg_wrapper_set_repeat_flag(it->second.ffmpeg_instance);

                if (sync_group_counter.sync_group_input_count == sync_group_counter.sync_group_count)
                {
                    {
                        std::lock_guard<std::mutex> lk(_sync_group_counter_mutex_frame_numbering);
                        _sync_group_counter_map_frame_numbering.erase(it->second.sync_group_index);
                    }

                    auto it_ffmpeg_data = _ffmpeg_data_map.begin();
                    for (; it_ffmpeg_data != _ffmpeg_data_map.end(); it_ffmpeg_data++)
                    {
                        if (it_ffmpeg_data->second.sync_group_index == it->second.sync_group_index)
                        {
                            cpp_ffmpeg_wrapper_repeat_sync_group(it_ffmpeg_data->second.ffmpeg_instance);

                            it_ffmpeg_data->second.repeat_command_send_flag = false;
                        }
                    }

                    _sync_group_counter_map_repeat.erase(it->second.sync_group_index);
                }
            }
            else
            {
                ffmpeg_instance = it->second.ffmpeg_instance;

                cpp_ffmpeg_wrapper_seek_pts(ffmpeg_instance, 0);

                it->second.repeat_command_send_flag = false;
            }
        }
        break;
        case command_type::play_sync_group:
        {
            packet_play_sync_group_from_client* packet = (packet_play_sync_group_from_client*)data_pair.first;
            ffmpeg_instance = cpp_ffmpeg_wrapper_create();

            cpp_ffmpeg_wrapper_initialize(ffmpeg_instance, callback_ffmpeg_wrapper_ptr);

            if (_hw_accel == true)
            {
                cpp_ffmpeg_wrapper_set_hw_decode(ffmpeg_instance);
                cpp_ffmpeg_wrapper_set_hw_device_type(ffmpeg_instance, _hw_accel_device_type);
                cpp_ffmpeg_wrapper_set_hw_decode_adapter_index(ffmpeg_instance, _hw_accel_adapter_index);
            }

            cpp_ffmpeg_wrapper_set_file_path(ffmpeg_instance, packet->url);
            if (cpp_ffmpeg_wrapper_open_file(ffmpeg_instance) != 0)
            {
                // TODO: open_file 실패
                // 1, file_not_exist
                // 7, file_path_unsetted

                cpp_ffmpeg_wrapper_shutdown(ffmpeg_instance);
                cpp_ffmpeg_wrapper_delete(ffmpeg_instance);

                cppsocket_struct_server_send_play_sync_group data{};
                data.scene_index = u32_invalid_id;
                data.result = (u16)packet_result::fail;
                data.left = packet->left;
                data.top = packet->top;
                data.width = packet->width;
                data.height = packet->height;
                data.url_size = packet->url_size;
                memcpy(data.url, packet->url, packet->url_size);

                data.sync_group_index = packet->sync_group_index;
                data.sync_group_count = packet->sync_group_count;

                cppsocket_server_send_play_sync_group(_server, data_pair.second, data);
                break;
            }

            RECT rect = { packet->left, packet->top, packet->left + packet->width, packet->top + packet->height };

            u32 scene_index = create_scene_data(rect, packet->sync_group_index, packet->sync_group_count);

            FFmpegInstanceData ffmpeg_instance_data;
            ffmpeg_instance_data.ffmpeg_instance = ffmpeg_instance;
            ffmpeg_instance_data.sync_group_index = packet->sync_group_index;
            ffmpeg_instance_data.sync_group_count = packet->sync_group_count;

            _ffmpeg_data_map.insert({ scene_index, ffmpeg_instance_data });
            cpp_ffmpeg_wrapper_set_scene_index(ffmpeg_instance, scene_index);

            // client에 돌려주기 위한 정보를 cpp_ffmpeg_wrapper에 세팅
            cpp_ffmpeg_wrapper_set_rect(ffmpeg_instance, rect);
            cpp_ffmpeg_wrapper_set_sync_group_index(ffmpeg_instance, packet->sync_group_index);
            cpp_ffmpeg_wrapper_set_sync_group_count(ffmpeg_instance, packet->sync_group_count);

            if (_sync_group_frame_numbering == true)
            {
                cpp_ffmpeg_wrapper_set_sync_group_frame_numbering(ffmpeg_instance);
            }

            cpp_ffmpeg_wrapper_play_start(ffmpeg_instance, data_pair.second);
        }
        break;
        case command_type::pause_sync_group:
        {

        }
        break;
        case command_type::stop_sync_group:
        {
            std::lock_guard<std::mutex> lock(_ffmpeg_data_mutex);

            packet_stop_sync_group_from_client* packet = (packet_stop_sync_group_from_client*)data_pair.first;

            SyncGroupCounter sync_group_counter{};

            auto it = _ffmpeg_data_map.begin();
            for (; it != _ffmpeg_data_map.end(); it++)
            {
                if (it->second.sync_group_index == packet->sync_group_index)
                {
                    std::lock_guard<std::mutex> lk(_sync_group_counter_mutex_stop);

                    auto it_sync_group_stop = _sync_group_counter_map_stop.find(packet->sync_group_index);
                    if (it_sync_group_stop == _sync_group_counter_map_stop.end())
                    {
                        sync_group_counter.sync_group_count = it->second.sync_group_count;
                        _sync_group_counter_map_stop.insert({ packet->sync_group_index, sync_group_counter });
                    }
                    
                    ffmpeg_instance = it->second.ffmpeg_instance;

                    cpp_ffmpeg_wrapper_play_stop(ffmpeg_instance, data_pair.second);
                }
            }

            if (sync_group_counter.sync_group_count == 0)
            {
                cppsocket_struct_server_send_stop_sync_group data{};
                data.sync_group_index = packet->sync_group_index;
                data.result = (u16)packet_result::fail;

                cppsocket_server_send_stop_sync_group(_server, data_pair.second, data);
                break;
            }
        }
        break;
        default:
            break;
        }

        delete data_pair.first;
    }
}

void get_asset_path(wchar_t * path, u32 path_size)
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

u32 enum_adapters()
{
    if (!_factory)
    {
        return u32();
    }

    HRESULT hr = S_OK;

    IDXGIAdapter1* adapter = nullptr;
    DXGI_ADAPTER_DESC1 adapter_desc{};

    for (u32 i = 0; ; i++)
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
            graphics_data* data = new graphics_data();
            data->adapter = adapter;
            data->adapter_index = i;
            _graphics_data_list.push_back(data);
        }
        else
        {
            adapter->Release();
        }
    }

    size_t adapter_count = _graphics_data_list.size();
    if (_hw_accel_adapter_index > adapter_count)
    {
        _hw_accel_adapter_index = adapter_count;
    }

    return u32();
}

u32 delete_adapters()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        data->adapter->Release();
        data->adapter = nullptr;
    }

    for (auto it = _graphics_data_list.begin(); it != _graphics_data_list.end();)
    {
        delete *it;
        it = _graphics_data_list.erase(it);
    }

    return u32();
}

u32 enum_output_list()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    HRESULT hr = S_OK;

    IDXGIOutput* output = nullptr;

    s32 output_index = 0;

    for (auto data : _graphics_data_list)
    {
        if (data->adapter == nullptr)
        {
            continue;
        }

        int count_create_one_swapchain_for_each_adapter_without_control_output = 0;
        int temp_left = 0;
        int temp_top = 0;
        int temp_right = 0;
        int temp_bottom = 0;

        for (u32 i = 0; ; i++)
        {
            hr = data->adapter->EnumOutputs(i, &output);
            if (hr != S_OK)
            {
                break;
            }

            output_data* o_data = nullptr;

            if (count_create_one_swapchain_for_each_adapter_without_control_output != 0)
            {
            }
            else
            {
                o_data = new output_data();
            
                o_data->output = output;
                output->GetDesc(&o_data->output_desc);
                o_data->output_index = output_index;
                output_index++;

                data->output_list.push_back(o_data);
            }

            if (_create_one_swapchain_for_each_adapter == true)
            {
                for (size_t i = 1; ; i++)
                {
                    if (o_data->output_desc.DesktopCoordinates.left < _create_one_swapchain_for_each_adapter_window_width * i)
                    {
                        o_data->create_one_swapchain_for_each_adapter_rect.left = _create_one_swapchain_for_each_adapter_window_width * (i - 1);
                        break;
                    }
                }

                for (size_t i = 1; ; i++)
                {
                    if (o_data->output_desc.DesktopCoordinates.top < _create_one_swapchain_for_each_adapter_window_height * i)
                    {
                        o_data->create_one_swapchain_for_each_adapter_rect.top = _create_one_swapchain_for_each_adapter_window_height * (i - 1);
                        break;
                    }
                }

                for (size_t i = 1; ; i++)
                {
                    if (o_data->output_desc.DesktopCoordinates.right <= _create_one_swapchain_for_each_adapter_window_width * i)
                    {
                        o_data->create_one_swapchain_for_each_adapter_rect.right = _create_one_swapchain_for_each_adapter_window_width * i;
                        break;
                    }
                }

                for (size_t i = 1; ; i++)
                {
                    if (o_data->output_desc.DesktopCoordinates.bottom < _create_one_swapchain_for_each_adapter_window_height * i)
                    {
                        o_data->create_one_swapchain_for_each_adapter_rect.bottom = _create_one_swapchain_for_each_adapter_window_height * i;
                        break;
                    }
                }

                break;
            }
            else if (_create_one_swapchain_for_each_adapter_without_control_output == true)
            {
                DXGI_OUTPUT_DESC output_desc;
                output->GetDesc(&output_desc);

                if (output_desc.DesktopCoordinates.left == _create_one_swapchain_for_each_adapter_without_control_output_excluded_window_left
                    && output_desc.DesktopCoordinates.top == _create_one_swapchain_for_each_adapter_without_control_output_excluded_window_top
                    && output_desc.DesktopCoordinates.right == _create_one_swapchain_for_each_adapter_without_control_output_excluded_window_right
                    && output_desc.DesktopCoordinates.bottom == _create_one_swapchain_for_each_adapter_without_control_output_excluded_window_bottom
                    )
                {
                    output->Release();
                    output = nullptr;
                    continue;
                }
                
                if (count_create_one_swapchain_for_each_adapter_without_control_output == 0)
                {
                    temp_left = output_desc.DesktopCoordinates.left;
                    temp_top = output_desc.DesktopCoordinates.top;
                    temp_right = output_desc.DesktopCoordinates.right;
                    temp_bottom = output_desc.DesktopCoordinates.bottom;
                }

                if (temp_left > output_desc.DesktopCoordinates.left)
                {
                    temp_left = output_desc.DesktopCoordinates.left;
                }

                if (temp_top > output_desc.DesktopCoordinates.top)
                {
                    temp_top = output_desc.DesktopCoordinates.top;
                }
                
                if (temp_right < output_desc.DesktopCoordinates.right)
                {
                    temp_right = output_desc.DesktopCoordinates.right;
                }

                if (temp_bottom < output_desc.DesktopCoordinates.bottom)
                {
                    temp_bottom = output_desc.DesktopCoordinates.bottom;
                }

                if (count_create_one_swapchain_for_each_adapter_without_control_output != 0)
                {
                    output->Release();
                    output = nullptr;
                }

                count_create_one_swapchain_for_each_adapter_without_control_output++;
            }
        }

        if (_create_one_swapchain_for_each_adapter_without_control_output == true)
        {
            auto it = data->output_list.begin();
            if (it != data->output_list.end())
            {
                output_data* o_data = *it;

                o_data->create_one_swapchain_for_each_adapter_without_control_output_rect.left = temp_left;
                o_data->create_one_swapchain_for_each_adapter_without_control_output_rect.top = temp_top;
                o_data->create_one_swapchain_for_each_adapter_without_control_output_rect.right = temp_right;
                o_data->create_one_swapchain_for_each_adapter_without_control_output_rect.bottom = temp_bottom;
            }
        }
    }

    return u32();
}

u32 delete_output_list()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        for (auto output : data->output_list)
        {
            output->output->Release();
            output->output = nullptr;
        }

        for (auto it = data->output_list.begin(); it != data->output_list.end();)
        {
            delete *it;
            it = data->output_list.erase(it);
        }
    }

    return u32();
}

u32 create_factory()
{
    HRESULT hr = S_OK;

    u32 dxgi_factory_flags = 0;

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

    return u32();
}

u32 delete_factory()
{
    if (_factory)
    {
        _factory->Release();
        _factory = nullptr;
    }

    return u32();
}

u32 create_devices()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    HRESULT hr = S_OK;

    for (auto data : _graphics_data_list)
    {
        if (data->adapter == nullptr)
        {
            continue;
        }

        ID3D12Device* device = nullptr;

        hr = D3D12CreateDevice(data->adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));

        if (hr == S_OK)
        {
            data->device = device;
            NAME_D3D12_OBJECT_INDEXED(device, data->adapter_index, L"ID3D12Device");
        }
    }

    return u32();
}

u32 delete_devices()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        data->device->Release();
        data->device = nullptr;
    }

    return u32();
}

u32 create_command_queues()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    HRESULT hr = S_OK;

    for (auto data : _graphics_data_list)
    {
        if (data->device == nullptr)
        {
            continue;
        }

        ID3D12CommandQueue* cmd_queue = nullptr;

        D3D12_COMMAND_QUEUE_DESC queue_desc{};
        queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

        hr = data->device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&cmd_queue));

        if (hr == S_OK)
        {
            data->cmd_queue = cmd_queue;
            NAME_D3D12_OBJECT_INDEXED(cmd_queue, data->adapter_index, L"ID3D12CommandQueue");
        }
    }

    return u32();
}

u32 delete_command_queues()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        if (data->device == nullptr)
        {
            continue;
        }

        data->cmd_queue->Release();
        data->cmd_queue = nullptr;
    }

    return u32();
}

u32 create_window(WCHAR* window_class, WCHAR* title, HINSTANCE instance, RECT rect, void * data /* = nullptr */, HWND & handle)
{
    AdjustWindowRect(&rect, WS_POPUP, FALSE);

    int left = 0;
    int top = 0;
    int width = 0;
    int height = 0;

    if (_window_create_position_shift_up == true)
    {
        left = rect.left;
        top = rect.top - 1;
        width = rect.right - rect.left;
        height = rect.bottom - rect.top + 1;
    }
    else
    {
        left = rect.left;
        top = rect.top;
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }

    handle = CreateWindow(window_class, title, WS_POPUP,
        left, top, width, height,
        nullptr, nullptr, instance, data);

    if (handle == nullptr)
    {
        return u32();
    }

    ShowWindow(handle, SW_SHOW);
    UpdateWindow(handle);

    return u32();
}

u32 create_swap_chains()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    HRESULT hr = S_OK;

    for (auto data : _graphics_data_list)
    {
        for (auto output : data->output_list)
        {
            if (output->handle == nullptr)
            {
                continue;
            }

            DXGI_SWAP_CHAIN_DESC1 desc{};
            if (_create_one_swapchain_for_each_adapter == true)
            {
                desc.Width = output->create_one_swapchain_for_each_adapter_rect.right - output->create_one_swapchain_for_each_adapter_rect.left;
                desc.Height = output->create_one_swapchain_for_each_adapter_rect.bottom - output->create_one_swapchain_for_each_adapter_rect.top;
            }
            else if (_create_one_swapchain_for_each_adapter_without_control_output == true)
            {
                desc.Width = output->create_one_swapchain_for_each_adapter_without_control_output_rect.right - output->create_one_swapchain_for_each_adapter_without_control_output_rect.left;
                desc.Height = output->create_one_swapchain_for_each_adapter_without_control_output_rect.bottom - output->create_one_swapchain_for_each_adapter_without_control_output_rect.top;
            }
            else
            {
                desc.Width = output->output_desc.DesktopCoordinates.right - output->output_desc.DesktopCoordinates.left;
                desc.Height = output->output_desc.DesktopCoordinates.bottom - output->output_desc.DesktopCoordinates.top;
            }
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            desc.BufferCount = frame_buffer_count;
            desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

            IDXGISwapChain1* swap_chain = nullptr;

            hr = _factory->CreateSwapChainForHwnd(
                data->cmd_queue,
                output->handle,
                &desc,
                nullptr,
                nullptr,
                &swap_chain
            );

            if (hr != S_OK)
            {
                continue;
            }

            hr = _factory->MakeWindowAssociation(output->handle, DXGI_MWA_NO_ALT_ENTER);
            hr = swap_chain->QueryInterface(&output->swap_chain);

            swap_chain->Release();
            swap_chain = nullptr;

            if (_disable_present_barrier == false)
            {
                if (output->present_barrier_client != nullptr)
                {
                    if (output->present_barrier_joined == true)
                    {
                        _nvapi_status = NvAPI_LeavePresentBarrier(output->present_barrier_client);
                        output->present_barrier_joined = false;
                    }

                    _nvapi_status = NvAPI_DestroyPresentBarrierClient(output->present_barrier_client);
                    output->present_barrier_client = nullptr;
                }

                _nvapi_status = NvAPI_D3D12_CreatePresentBarrierClient(data->device, output->swap_chain, &output->present_barrier_client);
            }

            output->frame_index = output->swap_chain->GetCurrentBackBufferIndex();
        }
    }

    return u32();
}

u32 delete_swap_chains()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        for (auto output : data->output_list)
        {
            if (output->handle == nullptr)
            {
                continue;
            }

            output->swap_chain->Release();
            output->swap_chain = nullptr;
        }
    }

    return u32();
}

u32 create_rtv_heaps()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    HRESULT hr = S_OK;

    for (auto data : _graphics_data_list)
    {
        if (data->output_list.empty())
        {
            continue;
        }

        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{};
        rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_heap_desc.NumDescriptors = frame_buffer_count * data->output_list.size();
        rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        hr = data->device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&data->rtv_heaps));
        data->rtv_descriptor_size = data->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        NAME_D3D12_OBJECT_INDEXED(data->rtv_heaps, data->adapter_index, L"ID3D12DescriptorHeap_rtv");

        u32 i = 0;

        for (auto it = data->output_list.begin(); it != data->output_list.end(); it++)
        {
            output_data* o_data = *it;

            if (o_data->handle == nullptr)
            {
                continue;
            }

            o_data->rtv_handle = data->rtv_heaps->GetCPUDescriptorHandleForHeapStart();

            o_data->rtv_handle.ptr = o_data->rtv_handle.ptr + (i * data->rtv_descriptor_size * frame_buffer_count);

            D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = o_data->rtv_handle;

            for (u32 j = 0; j < frame_buffer_count; j++)
            {
                o_data->rtv_view_list.emplace_back();

                hr = o_data->swap_chain->GetBuffer(j, IID_PPV_ARGS(&o_data->rtv_view_list[j]));
                data->device->CreateRenderTargetView(o_data->rtv_view_list[j], nullptr, cpu_handle);
                cpu_handle.ptr = cpu_handle.ptr + data->rtv_descriptor_size;

                NAME_D3D12_OBJECT_INDEXED_3(o_data->rtv_view_list[j], data->adapter_index, i, j, L"ID3D12Resource_rtv_view");
            }

            i++;
        }

        for (u32 j = 0; j < frame_buffer_count; j++)
        {
            data->cmd_allocator_list.emplace_back();
            hr = data->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&data->cmd_allocator_list[j]));

            NAME_D3D12_OBJECT_INDEXED_2(data->cmd_allocator_list[j], data->adapter_index, j, L"ID3D12CommandAllocator");
        }
    }

    return u32();
}

u32 delete_rtv_heaps()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        if (data->output_list.empty())
        {
            continue;
        }

        data->rtv_heaps->Release();
        data->rtv_heaps = nullptr;

        for (u32 i = 0; i < frame_buffer_count; i++)
        {
            data->cmd_allocator_list[i]->Release();
            data->cmd_allocator_list[i] = nullptr;
        }

        for (auto o_data : data->output_list)
        {
            if (o_data->handle == nullptr)
            {
                continue;
            }
            for (u32 n = 0; n < frame_buffer_count; n++)
            {
                o_data->rtv_view_list[n]->Release();
                o_data->rtv_view_list[n] = nullptr;
            }
        }
    }

    return u32();
}

u32 create_srv_heaps()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    HRESULT hr = S_OK;

    for (auto data : _graphics_data_list)
    {
        if (data->output_list.empty())
        {
            continue;
        }

        D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc{};
        srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srv_heap_desc.NumDescriptors = srv_descriptor_count;
        srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        hr = data->device->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&data->srv_heaps));
        data->srv_descriptor_size = data->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        NAME_D3D12_OBJECT_INDEXED(data->srv_heaps, data->adapter_index, L"ID3D12DescriptorHeap_srv");
    }

    return u32();
}

u32 delete_srv_heaps()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        if (data->output_list.empty())
        {
            continue;
        }

        data->srv_heaps->Release();
        data->srv_heaps = nullptr;
    }

    return u32();
}

u32 create_root_signatures()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    HRESULT hr = S_OK;

    for (auto data : _graphics_data_list)
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data{};

        feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(data->device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &feature_data, sizeof(feature_data))))
        {
            feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[frame_buffer_count]{};
        CD3DX12_ROOT_PARAMETER1 root_parameters[frame_buffer_count]{};

        // texture 1
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
        root_parameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        // texture 2
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
        root_parameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

        // texture 1
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

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        hr = D3D12SerializeVersionedRootSignature(&root_sig_desc, &signature, &error);
        hr = data->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&data->root_sig));

        NAME_D3D12_OBJECT_INDEXED(data->root_sig, data->adapter_index, L"ID3D12RootSignature");
    }

    return u32();
}

u32 delete_root_signatures()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        data->root_sig->Release();
        data->root_sig = nullptr;
    }

    return u32();
}

u32 create_psos()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

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
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    for (auto data : _graphics_data_list)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
        pso_desc.pRootSignature = data->root_sig;
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
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso_desc.SampleDesc.Count = 1;
        pso_desc.NodeMask = 0;
        hr = data->device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&data->pso));

        NAME_D3D12_OBJECT_INDEXED(data->pso, data->adapter_index, L"ID3D12PipelineState");
    }

    return u32();
}

u32 delete_psos()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        data->pso->Release();
        data->pso = nullptr;
    }

    return u32();
}

u32 create_command_lists()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    HRESULT hr = S_OK;

    for (auto data : _graphics_data_list)
    {
        for (auto output : data->output_list)
        {
            hr = data->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, data->cmd_allocator_list[0], data->pso, IID_PPV_ARGS(&data->cmd_list));
            data->cmd_list->Close();

            NAME_D3D12_OBJECT_INDEXED(data->cmd_list, data->adapter_index, L"ID3D12GraphicsCommandList");

            break;
        }
    }

    return u32();
}

u32 delete_command_lists()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        if (data->cmd_list == nullptr)
        {
            continue;
        }
        data->cmd_list->Release();
        data->cmd_list = nullptr;
    }

    return u32();
}

u32 create_index_buffer(graphics_data* data)
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    HRESULT hr = S_OK;

    u32 indices[] =
    {
        0, 1, 2,
        1, 3, 2
    };

    const u32 index_buffer_size = sizeof(indices);

    CD3DX12_HEAP_PROPERTIES index_buffer_properties(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_HEAP_PROPERTIES index_upload_buffer_properties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC index_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(index_buffer_size);

    D3D12_SUBRESOURCE_DATA index_data{};
    index_data.pData = indices;
    index_data.RowPitch = sizeof(u32) * 3;
    index_data.SlicePitch = sizeof(indices);

    hr = data->device->CreateCommittedResource(
        &index_buffer_properties,
        D3D12_HEAP_FLAG_NONE,
        &index_buffer_desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&data->index_buffer)
    );
    NAME_D3D12_OBJECT_INDEXED(data->index_buffer, data->adapter_index, L"ID3D12Resource_index_buffer");

    hr = data->device->CreateCommittedResource(
        &index_upload_buffer_properties,
        D3D12_HEAP_FLAG_NONE,
        &index_buffer_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&data->index_upload_buffer)
    );
    NAME_D3D12_OBJECT_INDEXED(data->index_upload_buffer, data->adapter_index, L"ID3D12Resource_index_upload_buffer");

    CD3DX12_RESOURCE_BARRIER transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(data->index_buffer, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    data->cmd_list->ResourceBarrier(1, &transition_barrier);
    UpdateSubresources(data->cmd_list, data->index_buffer, data->index_upload_buffer, 0, 0, 1, &index_data);
    transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(data->index_buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    data->cmd_list->ResourceBarrier(1, &transition_barrier);

    data->index_buffer_view.BufferLocation = data->index_buffer->GetGPUVirtualAddress();
    data->index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
    data->index_buffer_view.SizeInBytes = index_buffer_size;

    data->index_count = index_buffer_size / 4;

    return u32();
}

u32 delete_index_buffer()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        if (data->output_list.empty())
        {
            continue;
        }

        data->index_buffer->Release();
        data->index_buffer = nullptr;

        data->index_upload_buffer->Release();
        data->index_upload_buffer = nullptr;
    }

    return u32();
}

// 
// Target Adapter
// Coordinate
// 
u32 create_vertex_buffer(graphics_data* data, s32 vertex_index, NormalizedRect normalized_rect, NormalizedRect normalized_uv, u32 scene_index)
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    HRESULT hr = S_OK;

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

    if (_texture_create_each_panel == false)
    {
        vertices[0] = { { normalized_rect.left, normalized_rect.top, 0.0f }, { normalized_uv.left, normalized_uv.top } };
        vertices[1] = { { normalized_rect.right, normalized_rect.top, 0.0f }, { normalized_uv.right, normalized_uv.top } };
        vertices[2] = { { normalized_rect.left, normalized_rect.bottom, 0.0f }, { normalized_uv.left, normalized_uv.bottom } };
        vertices[3] = { { normalized_rect.right, normalized_rect.bottom, 0.0f }, { normalized_uv.right, normalized_uv.bottom } };
    }
    else
    {
        vertices[0] = { { normalized_rect.left, normalized_rect.top, 0.0f }, { 0.0f, 0.0f } };
        vertices[1] = { { normalized_rect.right, normalized_rect.top, 0.0f }, { 1.0f, 0.0f } };
        vertices[2] = { { normalized_rect.left, normalized_rect.bottom, 0.0f }, { 0.0f, 1.0f } };
        vertices[3] = { { normalized_rect.right, normalized_rect.bottom, 0.0f }, { 1.0f, 1.0f } };
    }

    const u32 vertex_buffer_size = sizeof(vertices);
    CD3DX12_HEAP_PROPERTIES vertex_buffer_properties(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_HEAP_PROPERTIES vertex_upload_buffer_properties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC vertex_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size);

    hr = data->device->CreateCommittedResource(
        &vertex_buffer_properties,
        D3D12_HEAP_FLAG_NONE,
        &vertex_buffer_desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&vertex_buffer)
    );
    NAME_D3D12_OBJECT_INDEXED_2(vertex_buffer, scene_index, vertex_index, L"ID3D12Resource_vertex_buffer");

    hr = data->device->CreateCommittedResource(
        &vertex_upload_buffer_properties,
        D3D12_HEAP_FLAG_NONE,
        &vertex_buffer_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertex_upload_buffer)
    );
    NAME_D3D12_OBJECT_INDEXED_2(vertex_upload_buffer, scene_index, vertex_index, L"ID3D12Resource_vertex_upload_buffer");

    D3D12_SUBRESOURCE_DATA vertex_data{};
    vertex_data.pData = vertices;
    vertex_data.RowPitch = sizeof(Vertex);
    vertex_data.SlicePitch = sizeof(vertices);

    CD3DX12_RESOURCE_BARRIER transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertex_buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    data->cmd_list->ResourceBarrier(1, &transition_barrier);
    UpdateSubresources(data->cmd_list, vertex_buffer, vertex_upload_buffer, 0, 0, 1, &vertex_data);
    transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertex_buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    data->cmd_list->ResourceBarrier(1, &transition_barrier);

    vertex_buffer_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
    vertex_buffer_view.StrideInBytes = sizeof(Vertex);
    vertex_buffer_view.SizeInBytes = vertex_buffer_size;

    data->vertex_buffer_map.insert({ vertex_index, vertex_buffer });
    data->vertex_upload_buffer_map.insert({ vertex_index, vertex_upload_buffer });
    data->vertex_buffer_view_map.insert({ vertex_index, vertex_buffer_view });

    return u32();
}

u32 delete_vertex_buffer_list()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        for (auto it = data->vertex_buffer_map.begin(); it != data->vertex_buffer_map.end();)
        {
            ID3D12Resource* o = it->second;
            o->Release();
            o = nullptr;
            it = data->vertex_buffer_map.erase(it);
        }

        for (auto it = data->vertex_upload_buffer_map.begin(); it != data->vertex_upload_buffer_map.end();)
        {
            ID3D12Resource* o = it->second;
            o->Release();
            o = nullptr;
            it = data->vertex_upload_buffer_map.erase(it);
        }

        for (auto it = data->vertex_buffer_view_map.begin(); it != data->vertex_buffer_view_map.end();)
        {
            it = data->vertex_buffer_view_map.erase(it);
        }
    }

    return u32();
}

//
// Target Adapter
// Size
//
u32 create_texture(graphics_data* data, u32 width, u32 height, s32 texture_index, u32 scene_index)
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    HRESULT hr = S_OK;

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

    for (u32 i = 0; i < frame_buffer_count; i++)
    {
        ID3D12Resource* texture_y = nullptr;
        hr = data->device->CreateCommittedResource(
            &texture_properties,
            D3D12_HEAP_FLAG_NONE,
            &texture_desc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr,
            IID_PPV_ARGS(&texture_y)
        );
        data->texture_map_y[i].insert({ texture_index, texture_y });

        NAME_D3D12_OBJECT_INDEXED_2(texture_y, scene_index, i, L"ID3D12Resource_texture_y");
    }

    texture_desc.Width /= 2;
    texture_desc.Height /= 2;

    for (u32 i = 0; i < frame_buffer_count; i++)
    {
        ID3D12Resource* texture_u = nullptr;
        hr = data->device->CreateCommittedResource(
            &texture_properties,
            D3D12_HEAP_FLAG_NONE,
            &texture_desc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr,
            IID_PPV_ARGS(&texture_u)
        );
        data->texture_map_u[i].insert({ texture_index, texture_u });

        NAME_D3D12_OBJECT_INDEXED_2(texture_u, scene_index, i, L"ID3D12Resource_texture_u");
    }
    for (u32 i = 0; i < frame_buffer_count; i++)
    {
        ID3D12Resource* texture_v = nullptr;
        hr = data->device->CreateCommittedResource(
            &texture_properties,
            D3D12_HEAP_FLAG_NONE,
            &texture_desc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr,
            IID_PPV_ARGS(&texture_v)
        );
        data->texture_map_v[i].insert({ texture_index, texture_v });

        NAME_D3D12_OBJECT_INDEXED_2(texture_v, scene_index, i, L"ID3D12Resource_texture_v");
    }

    u64 upload_buffer_size = 0;
    texture_desc.Format = DXGI_FORMAT_R8_TYPELESS;
    texture_desc.Width = width;
    texture_desc.Height = height;
    data->device->GetCopyableFootprints(&texture_desc, 0, 1, 0, nullptr, nullptr, nullptr, &upload_buffer_size);

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

    for (u32 i = 0; i < frame_buffer_count; i++)
    {
        ID3D12Resource* upload_texture_y = nullptr;
        hr = data->device->CreateCommittedResource(
            &texture_upload_heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &upload_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&upload_texture_y)
        );
        data->upload_texture_map_y[i].insert({ texture_index, upload_texture_y });

        NAME_D3D12_OBJECT_INDEXED_2(upload_texture_y, scene_index, i, L"ID3D12Resource_upload_texture_y");
    }

    texture_desc.Width /= 2;
    texture_desc.Height /= 2;
    data->device->GetCopyableFootprints(&texture_desc, 0, 1, 0, nullptr, nullptr, nullptr, &upload_buffer_size);
    upload_desc.Width = upload_buffer_size;

    for (u32 i = 0; i < frame_buffer_count; i++)
    {
        ID3D12Resource* upload_texture_u = nullptr;
        hr = data->device->CreateCommittedResource(
            &texture_upload_heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &upload_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&upload_texture_u)
        );
        data->upload_texture_map_u[i].insert({ texture_index, upload_texture_u });

        NAME_D3D12_OBJECT_INDEXED_2(upload_texture_u, scene_index, i, L"ID3D12Resource_upload_texture_u");
    }
    for (u32 i = 0; i < frame_buffer_count; i++)
    {
        ID3D12Resource* upload_texture_v = nullptr;
        hr = data->device->CreateCommittedResource(
            &texture_upload_heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &upload_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&upload_texture_v)
        );
        data->upload_texture_map_v[i].insert({ texture_index, upload_texture_v });

        NAME_D3D12_OBJECT_INDEXED_2(upload_texture_v, scene_index, i, L"ID3D12Resource_upload_texture_v");
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.Format = DXGI_FORMAT_R8_UNORM;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Texture2D.PlaneSlice = 0;
    srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

    const u32 srv_descriptor_size = data->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle(data->srv_heaps->GetCPUDescriptorHandleForHeapStart());
    srv_handle.ptr = SIZE_T(INT64(srv_handle.ptr) + INT64(srv_descriptor_size * (frame_buffer_count * texture_resource_count * texture_index)));

    for (u32 i = 0; i < frame_buffer_count; i++)
    {
        ID3D12Resource* texture_y = data->texture_map_y[i][texture_index];
        ID3D12Resource* texture_u = data->texture_map_u[i][texture_index];
        ID3D12Resource* texture_v = data->texture_map_v[i][texture_index];

        data->device->CreateShaderResourceView(texture_y, &srv_desc, srv_handle);
        srv_handle.ptr = SIZE_T(INT64(srv_handle.ptr) + INT64(srv_descriptor_size));
        data->device->CreateShaderResourceView(texture_u, &srv_desc, srv_handle);
        srv_handle.ptr = SIZE_T(INT64(srv_handle.ptr) + INT64(srv_descriptor_size));
        data->device->CreateShaderResourceView(texture_v, &srv_desc, srv_handle);
        srv_handle.ptr = SIZE_T(INT64(srv_handle.ptr) + INT64(srv_descriptor_size));
    }

    return u32();
}

u32 delete_textures()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        for (u32 i = 0; i < frame_buffer_count; i++)
        {
            for (auto it = data->texture_map_y[i].begin(); it != data->texture_map_y[i].end();)
            {
                ID3D12Resource* o = it->second;
                o->Release();
                o = nullptr;

                it = data->texture_map_y[i].erase(it);
            }

            for (auto it = data->texture_map_u[i].begin(); it != data->texture_map_u[i].end();)
            {
                ID3D12Resource* o = it->second;
                o->Release();
                o = nullptr;

                it = data->texture_map_u[i].erase(it);
            }

            for (auto it = data->texture_map_v[i].begin(); it != data->texture_map_v[i].end();)
            {
                ID3D12Resource* o = it->second;
                o->Release();
                o = nullptr;

                it = data->texture_map_v[i].erase(it);
            }

            for (auto it = data->upload_texture_map_y[i].begin(); it != data->upload_texture_map_y[i].end();)
            {
                ID3D12Resource* o = it->second;
                o->Release();
                o = nullptr;

                it = data->upload_texture_map_y[i].erase(it);
            }

            for (auto it = data->upload_texture_map_u[i].begin(); it != data->upload_texture_map_u[i].end();)
            {
                ID3D12Resource* o = it->second;
                o->Release();
                o = nullptr;

                it = data->upload_texture_map_u[i].erase(it);
            }

            for (auto it = data->upload_texture_map_v[i].begin(); it != data->upload_texture_map_v[i].end();)
            {
                ID3D12Resource* o = it->second;
                o->Release();
                o = nullptr;

                it = data->upload_texture_map_v[i].erase(it);
            }
        }
    }

    return u32();
}

u32 create_fences()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    HRESULT hr = S_OK;

    for (auto data : _graphics_data_list)
    {
        int n = 0;

        for (auto output : data->output_list)
        {
            if (output->handle == nullptr)
            {
                continue;
            }

            hr = data->device->CreateFence(output->fence_values[output->frame_index], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&output->fence));

            NAME_D3D12_OBJECT_INDEXED_2(output->fence, data->adapter_index, n, L"ID3D12Fence");

            output->fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (output->fence_event == nullptr)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }

            n++;
        }
    }

    return u32();
}

u32 delete_fences()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        for (auto output : data->output_list)
        {
            if (output->handle == nullptr)
            {
                continue;
            }

            CloseHandle(output->fence_event);

            output->fence->Release();
            output->fence = nullptr;
        }
    }

    return u32();
}

u32 create_viewports()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    for (auto data : _graphics_data_list)
    {
        for (auto output : data->output_list)
        {
            if (output->handle == nullptr)
            {
                continue;
            }

            if (_create_one_swapchain_for_each_adapter == true)
            {
                output->viewport = { 0.0f, 0.0f, (f32)(output->create_one_swapchain_for_each_adapter_rect.right - output->create_one_swapchain_for_each_adapter_rect.left), (f32)(output->create_one_swapchain_for_each_adapter_rect.bottom - output->create_one_swapchain_for_each_adapter_rect.top) };
                output->scissor_rect = { 0, 0, (s32)(output->create_one_swapchain_for_each_adapter_rect.right - output->create_one_swapchain_for_each_adapter_rect.left), (s32)(output->create_one_swapchain_for_each_adapter_rect.bottom - output->create_one_swapchain_for_each_adapter_rect.top) };
            }
            else if (_create_one_swapchain_for_each_adapter_without_control_output == true)
            {
                output->viewport = { 0.0f, 0.0f, (f32)(output->create_one_swapchain_for_each_adapter_without_control_output_rect.right - output->create_one_swapchain_for_each_adapter_without_control_output_rect.left), (f32)(output->create_one_swapchain_for_each_adapter_without_control_output_rect.bottom - output->create_one_swapchain_for_each_adapter_without_control_output_rect.top) };
                output->scissor_rect = { 0, 0, (s32)(output->create_one_swapchain_for_each_adapter_without_control_output_rect.right - output->create_one_swapchain_for_each_adapter_without_control_output_rect.left), (s32)(output->create_one_swapchain_for_each_adapter_without_control_output_rect.bottom - output->create_one_swapchain_for_each_adapter_without_control_output_rect.top) };
            }
            else
            {
                output->viewport = { 0.0f, 0.0f, (f32)(output->output_desc.DesktopCoordinates.right - output->output_desc.DesktopCoordinates.left), (f32)(output->output_desc.DesktopCoordinates.bottom - output->output_desc.DesktopCoordinates.top) };
                output->scissor_rect = { 0, 0, (s32)(output->output_desc.DesktopCoordinates.right - output->output_desc.DesktopCoordinates.left), (s32)(output->output_desc.DesktopCoordinates.bottom - output->output_desc.DesktopCoordinates.top) };
            }
        }
    }

    return u32();
}

u32 wait_for_gpu(ID3D12CommandQueue* cmd_queue, output_data* data)
{
    HRESULT hr = S_OK;

    hr = cmd_queue->Signal(data->fence, data->fence_values[data->frame_index]);

    hr = data->fence->SetEventOnCompletion(data->fence_values[data->frame_index], data->fence_event);
    WaitForSingleObject(data->fence_event, INFINITE);

    data->fence_values[data->frame_index]++;

    return u32();
}

u32 wait_for_gpus()
{
    for (auto data : _graphics_data_list)
    {
        for (auto output : data->output_list)
        {
            if (output->handle == nullptr)
            {
                continue;
            }

            wait_for_gpu(data->cmd_queue, output);
        }
    }

    return u32();
}

u32 move_to_next_frame(ID3D12CommandQueue* cmd_queue, output_data* data)
{
    HRESULT hr = S_OK;

    const u64 current_fence_value = data->fence_values[data->frame_index];
    hr = cmd_queue->Signal(data->fence, current_fence_value);

    data->frame_index = data->swap_chain->GetCurrentBackBufferIndex();

    if (data->fence->GetCompletedValue() < data->fence_values[data->frame_index])
    {
        hr = data->fence->SetEventOnCompletion(data->fence_values[data->frame_index], data->fence_event);
        WaitForSingleObject(data->fence_event, INFINITE);
    }

    data->fence_values[data->frame_index] = current_fence_value + 1;

    return u32();
}

u32 populate_command_list(graphics_data* data)
{
    HRESULT hr = S_OK;

    u32 current_back_buffer_index = ((output_data*)*data->output_list.begin())->frame_index;

    hr = data->cmd_allocator_list[current_back_buffer_index]->Reset();

    data->cmd_list->Reset(data->cmd_allocator_list[current_back_buffer_index], data->pso);

    if (data->deferred_free_flag[current_back_buffer_index] == true)
    {
        data->deferred_free_flag[current_back_buffer_index] = false;

        deferred_free_processing(current_back_buffer_index);
    }

    AVFrame* frame = av_frame_alloc();
    // get texture
    for (auto scene : _graphics_scene_list)
    {
        int32_t output_frame_index = -1;

        {
            std::lock_guard<std::mutex> lock(_ffmpeg_data_mutex);
            auto it_ffmpeg_data = _ffmpeg_data_map.find(scene->scene_index);
            if (it_ffmpeg_data != _ffmpeg_data_map.end())
            {
                output_frame_index = cpp_ffmpeg_wrapper_get_frame(it_ffmpeg_data->second.ffmpeg_instance, frame);

                if (output_frame_index == -2)
                {
                    // eof
                    if (_repeat_play_flag == true)
                    {
                        if (it_ffmpeg_data->second.repeat_command_send_flag == false)
                        {
                            it_ffmpeg_data->second.repeat_command_send_flag = true;

                            packet_seek_repeat_self temp_packet{};
                            temp_packet.scene_index = scene->scene_index;
                            temp_packet.header.cmd = command_type::seek_repeat_self;
                            temp_packet.header.size = sizeof(packet_seek_repeat_self);

                            void* packet = new char[temp_packet.header.size];
                            memcpy(packet, &temp_packet, temp_packet.header.size);

                            {
                                std::lock_guard<std::mutex> lk(_tcp_processing_mutex);
                                _tcp_processing_command_queue.push_back(std::pair<void*, void*>(packet, nullptr));
                            }
                        }
                    }
                }
            }
        }

        if (output_frame_index < 0)
        {
            continue;
        }

        scene->using_flag = true;
        scene->pts = frame->pts;

        for (auto panel : scene->panel_list)
        {
            if (data->adapter_index != panel->adapter_index)
            {
                continue;
            }

            if (_texture_create_each_panel == false)
            {
                if (data->texture_map_y[0].find(panel->texture_index) == data->texture_map_y[0].end())
                {
                    create_texture(data, frame->width, frame->height, panel->texture_index, scene->scene_index);
                }
            }

            for (auto output : data->output_list)
            {
                if (output->handle == nullptr)
                {
                    continue;
                }
                if (output->output_index != panel->output_index)
                {
                    continue;
                }

                if (panel->normalize_uv_flag == false)
                {
                    panel->normalize_uv_flag = true;
                    normalize_uv(scene->rect, panel->rect, panel->normalized_uv);
                }

                if (_texture_create_each_panel == true)
                {
                    if (data->texture_map_y[0].find(panel->texture_index) == data->texture_map_y[0].end())
                    {
                        create_texture(data, frame->width * (panel->normalized_uv.right - panel->normalized_uv.left), frame->height * (panel->normalized_uv.bottom - panel->normalized_uv.top), panel->texture_index, scene->scene_index);
                    }
                }
            }

            if (output_frame_index != -1)
            {
                if (_texture_create_each_panel == true)
                {
                    upload_texture_each_panel(data, frame, output_frame_index, panel);
                }
                else
                {
                    auto it_upload_flag = scene->texture_upload_to_adapter_flag_map.find(panel->adapter_index);
                    if (it_upload_flag != scene->texture_upload_to_adapter_flag_map.end())
                    {
                        if (it_upload_flag->second == false)
                        {
                            upload_texture(data, frame, panel->texture_index, output_frame_index);
                            it_upload_flag->second = true;
                        }
                    }
                }

                panel->output_frame_index = output_frame_index;
            }
        }

        av_frame_unref(frame);
    }
    av_frame_free(&frame);

    // create vertex
    for (auto scene : _graphics_scene_list)
    {
        for (auto panel : scene->panel_list)
        {
            if (data->adapter_index != panel->adapter_index)
            {
                continue;
            }

            if (data->vertex_buffer_map.find(panel->vertex_index) == data->vertex_buffer_map.end())
            {
                create_vertex_buffer(data, panel->vertex_index, panel->normalized_rect, panel->normalized_uv, scene->scene_index);
            }
        }
    }

    if (data->create_index_buffer_flag)
    {
        data->create_index_buffer_flag = false;
        create_index_buffer(data);
    }

    // remove data input to deferred_free_maps
    {
        std::lock_guard<std::mutex> lock(_graphics_remove_mutex);
        for (auto it_remove_scene = _graphics_remove_queue.begin(); it_remove_scene != _graphics_remove_queue.end();)
        {
            RemoveScene remove_scene = *it_remove_scene;

            for (auto it_scene = _graphics_scene_list.begin(); it_scene != _graphics_scene_list.end();)
            {
                Scene* scene = *it_scene;

                if (scene->scene_index != remove_scene.scene_index)
                {
                    it_scene++;
                    continue;
                }

                for (auto it_panel = scene->panel_list.begin(); it_panel != scene->panel_list.end();)
                {
                    Panel* panel = *it_panel;

                    if (data->adapter_index != panel->adapter_index)
                    {
                        it_panel++;
                        continue;
                    }
                    
                    // remove vertex
                    {
                        auto it_vertex = data->vertex_buffer_map.find(panel->vertex_index);
                        if (it_vertex != data->vertex_buffer_map.end())
                        {
                            deferred_free_object object;
                            object.resource = (*it_vertex).second;
                            object.index = (*it_vertex).first;
                            object.type = deferred_type::vertex_buffer;
                            data->deferred_free_object_list[current_back_buffer_index].push_back(object);
                            
                            data->vertex_buffer_map.erase(panel->vertex_index);
                        }

                        auto it_vertex_upload = data->vertex_upload_buffer_map.find(panel->vertex_index);
                        if (it_vertex_upload != data->vertex_upload_buffer_map.end())
                        {
                            deferred_free_object object;
                            object.resource = (*it_vertex_upload).second;
                            object.index = (*it_vertex_upload).first;
                            object.type = deferred_type::vertex_upload_buffer;
                            data->deferred_free_object_list[current_back_buffer_index].push_back(object);

                            data->vertex_upload_buffer_map.erase(panel->vertex_index);
                        }

                        auto it_vertex_view = data->vertex_buffer_view_map.find(panel->vertex_index);
                        if (it_vertex_view != data->vertex_buffer_view_map.end())
                        {
                            data->vertex_buffer_view_map.erase(panel->vertex_index);
                        }
                    }

                    // remove texture
                    if (data->texture_map_y[0].find(panel->texture_index) != data->texture_map_y[0].end())
                    {
                        for (u32 i = 0; i < frame_buffer_count; i++)
                        {
                            auto it_texture_y = data->texture_map_y[i].find(panel->texture_index);
                            if (it_texture_y != data->texture_map_y[i].end())
                            {
                                deferred_free_object object;
                                object.resource = (*it_texture_y).second;
                                object.index = (*it_texture_y).first;
                                object.type = deferred_type::texture_y;
                                data->deferred_free_object_list[current_back_buffer_index].push_back(object);

                                data->texture_map_y[i].erase(panel->texture_index);
                            }

                            auto it_texture_u = data->texture_map_u[i].find(panel->texture_index);
                            if (it_texture_u != data->texture_map_u[i].end())
                            {
                                deferred_free_object object;
                                object.resource = (*it_texture_u).second;
                                object.index = (*it_texture_u).first;
                                object.type = deferred_type::texture_u;
                                data->deferred_free_object_list[current_back_buffer_index].push_back(object);

                                data->texture_map_u[i].erase(panel->texture_index);
                            }

                            auto it_texture_v = data->texture_map_v[i].find(panel->texture_index);
                            if (it_texture_v != data->texture_map_v[i].end())
                            {
                                deferred_free_object object;
                                object.resource = (*it_texture_v).second;
                                object.index = (*it_texture_v).first;
                                object.type = deferred_type::texture_v;
                                data->deferred_free_object_list[current_back_buffer_index].push_back(object);

                                data->texture_map_v[i].erase(panel->texture_index);
                            }

                            auto it_upload_texture_y = data->upload_texture_map_y[i].find(panel->texture_index);
                            if (it_upload_texture_y != data->upload_texture_map_y[i].end())
                            {
                                deferred_free_object object;
                                object.resource = (*it_upload_texture_y).second;
                                object.index = (*it_upload_texture_y).first;
                                object.type = deferred_type::upload_texture_y;
                                data->deferred_free_object_list[current_back_buffer_index].push_back(object);

                                data->upload_texture_map_y[i].erase(panel->texture_index);
                            }

                            auto it_upload_texture_u = data->upload_texture_map_u[i].find(panel->texture_index);
                            if (it_upload_texture_u != data->upload_texture_map_u[i].end())
                            {
                                deferred_free_object object;
                                object.resource = (*it_upload_texture_u).second;
                                object.index = (*it_upload_texture_u).first;
                                object.type = deferred_type::upload_texture_u;
                                data->deferred_free_object_list[current_back_buffer_index].push_back(object);

                                data->upload_texture_map_u[i].erase(panel->texture_index);
                            }

                            auto it_upload_texture_v = data->upload_texture_map_v[i].find(panel->texture_index);
                            if (it_upload_texture_v != data->upload_texture_map_v[i].end())
                            {
                                deferred_free_object object;
                                object.resource = (*it_upload_texture_v).second;
                                object.index = (*it_upload_texture_v).first;
                                object.type = deferred_type::upload_texture_v;
                                if (i == frame_buffer_count - 1)
                                {
                                    object.free_flag = true;
                                }

                                data->deferred_free_object_list[current_back_buffer_index].push_back(object);

                                data->upload_texture_map_v[i].erase(panel->texture_index);
                            }
                        }
                    }

                    data->deferred_free_flag[current_back_buffer_index] = true;

                    if (_texture_create_each_panel == true)
                    {
                        delete[] panel->separated_frame_data_y;
                        panel->separated_frame_data_y = nullptr;

                        delete[] panel->separated_frame_data_u;
                        panel->separated_frame_data_u = nullptr;
                        
                        delete[] panel->separated_frame_data_v;
                        panel->separated_frame_data_v = nullptr;
                    }

                    delete panel;
                    it_panel = scene->panel_list.erase(it_panel);
                }

                if (scene->panel_list.empty())
                {
                    remove_scene.all_panel_removed_flag = true;
                    delete scene;
                    it_scene = _graphics_scene_list.erase(it_scene);
                }
                else
                {
                    it_scene++;
                }
            }

            if (remove_scene.all_panel_removed_flag)
            {
                _free_scene_queue.push_back(remove_scene.scene_index);
                it_remove_scene = _graphics_remove_queue.erase(it_remove_scene);
            }
            else
            {
                it_remove_scene++;
            }
        }
    }

    data->cmd_list->SetGraphicsRootSignature(data->root_sig);

    ID3D12DescriptorHeap* pp_heaps[] = { data->srv_heaps };
    data->cmd_list->SetDescriptorHeaps(_countof(pp_heaps), pp_heaps);

    for (auto output : data->output_list)
    {
        if (output->handle == nullptr)
        {
            continue;
        }
        data->cmd_list->RSSetViewports(1, &output->viewport);
        data->cmd_list->RSSetScissorRects(1, &output->scissor_rect);

        CD3DX12_RESOURCE_BARRIER transition_barrier_before = CD3DX12_RESOURCE_BARRIER::Transition(output->rtv_view_list[output->frame_index], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        data->cmd_list->ResourceBarrier(1, &transition_barrier_before);

        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = data->rtv_heaps->GetCPUDescriptorHandleForHeapStart();
        rtv_handle.ptr = SIZE_T(INT64(output->rtv_handle.ptr) + INT64(data->rtv_descriptor_size * output->frame_index));

        data->cmd_list->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);

        //float clear_color[] = { _clear_color[0] * output->frame_index, _clear_color[1] * output->frame_index, _clear_color[2] * output->frame_index, _clear_color[3] * output->frame_index };

        data->cmd_list->ClearRenderTargetView(rtv_handle, _clear_color, 0, nullptr);
        //data->cmd_list->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);
        data->cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


        const u32 srv_descriptor_size = data->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        data->cmd_list->IASetIndexBuffer(&data->index_buffer_view);

        for (auto scene : _graphics_scene_list)
        {
            for (auto panel : scene->panel_list)
            {
                if (data->adapter_index != panel->adapter_index)
                {
                    continue;
                }

                if (panel->output_index != output->output_index)
                {
                    continue;
                }

                D3D12_GPU_DESCRIPTOR_HANDLE srv_handle = data->srv_heaps->GetGPUDescriptorHandleForHeapStart();
                srv_handle.ptr = SIZE_T(INT64(srv_handle.ptr) + INT64(srv_descriptor_size * (frame_buffer_count * texture_resource_count * panel->texture_index)));
                srv_handle.ptr = SIZE_T(INT64(srv_handle.ptr) + INT64(srv_descriptor_size * (texture_resource_count * panel->output_frame_index)));

                data->cmd_list->SetGraphicsRootDescriptorTable(0, srv_handle);
                srv_handle.ptr = SIZE_T(INT64(srv_handle.ptr) + INT64(srv_descriptor_size));
                data->cmd_list->SetGraphicsRootDescriptorTable(1, srv_handle);
                srv_handle.ptr = SIZE_T(INT64(srv_handle.ptr) + INT64(srv_descriptor_size));
                data->cmd_list->SetGraphicsRootDescriptorTable(2, srv_handle);
                srv_handle.ptr = SIZE_T(INT64(srv_handle.ptr) + INT64(srv_descriptor_size));

                data->cmd_list->IASetVertexBuffers(0, 1, &data->vertex_buffer_view_map[panel->vertex_index]);
                data->cmd_list->DrawIndexedInstanced(data->index_count, 1, 0, 0, 0);
            }
        }

        CD3DX12_RESOURCE_BARRIER transition_barrier_after = CD3DX12_RESOURCE_BARRIER::Transition(output->rtv_view_list[output->frame_index], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        data->cmd_list->ResourceBarrier(1, &transition_barrier_after);
    }

    hr = data->cmd_list->Close();

    return u32();
}

u32 render()
{
    HRESULT hr = S_OK;

    for (auto data : _graphics_data_list)
    {
        if (data->output_list.empty())
        {
            continue;
        }

        populate_command_list(data);

        ID3D12CommandList* pp_cmd_lists[] = { data->cmd_list };
        data->cmd_queue->ExecuteCommandLists(_countof(pp_cmd_lists), pp_cmd_lists);
    }

    for (auto data : _graphics_data_list)
    {
        if (data->output_list.empty())
        {
            continue;
        }

        for (auto output : data->output_list)
        {
            if (output->handle == nullptr)
            {
                continue;
            }
            hr = output->swap_chain->Present(1, 0);

            if (_disable_present_barrier == false)
            {
                _nvapi_status = NvAPI_QueryPresentBarrierFrameStatistics(output->present_barrier_client, &output->present_barrier_frame_stats);

                switch (output->present_barrier_frame_stats.SyncMode)
                {
                case NV_PRESENT_BARRIER_SYNC_MODE::PRESENT_BARRIER_NOT_JOINED:
                {
                    OutputDebugString(L"PRESENT_BARRIER_NOT_JOINED\n");
                }
                break;
                case NV_PRESENT_BARRIER_SYNC_MODE::PRESENT_BARRIER_SYNC_CLIENT:
                {
                    OutputDebugString(L"PRESENT_BARRIER_SYNC_CLIENT\n");
                }
                break;
                case NV_PRESENT_BARRIER_SYNC_MODE::PRESENT_BARRIER_SYNC_SYSTEM:
                {
                    OutputDebugString(L"PRESENT_BARRIER_SYNC_SYSTEM\n");
                }
                break;
                case NV_PRESENT_BARRIER_SYNC_MODE::PRESENT_BARRIER_SYNC_CLUSTER:
                {
                    OutputDebugString(L"PRESENT_BARRIER_SYNC_CLUSTER\n");
                }
                break;
                default:
                    break;
                }
            }
        }
    }

    for (auto data : _graphics_data_list)
    {
        if (data->output_list.empty())
        {
            continue;
        }

        for (auto output : data->output_list)
        {
            if (output->handle == nullptr)
            {
                continue;
            }

            move_to_next_frame(data->cmd_queue, output);
        }
    }

    {
        std::lock_guard<std::mutex> lock(_ffmpeg_data_mutex);

        std::map<u32, SyncGroupCounter> frame_to_next_sync_group_map;

        for (auto scene : _graphics_scene_list)
        {
            if (scene->using_flag == false)
            {
                continue;
            }
            else
            {
                scene->using_flag = false;
            }

            for (auto it_upload_flag = scene->texture_upload_to_adapter_flag_map.begin(); it_upload_flag != scene->texture_upload_to_adapter_flag_map.end();)
            {
                it_upload_flag->second = false;
                it_upload_flag++;
            }

            auto it_ffmpeg_data = _ffmpeg_data_map.find(scene->scene_index);
            if (it_ffmpeg_data != _ffmpeg_data_map.end())
            {
                if (scene->sync_group_index != u32_invalid_id)
                {
                    s32 ret = cpp_ffmpeg_wrapper_check_frame_to_next_sync_group(it_ffmpeg_data->second.ffmpeg_instance);
                    if (ret == 0)
                    {
                        SyncGroupCounter sync_group_counter;

                        auto it_sync_group_counter = frame_to_next_sync_group_map.find(scene->sync_group_index);
                        if (it_sync_group_counter == frame_to_next_sync_group_map.end())
                        {
                            sync_group_counter.sync_group_count = scene->sync_group_count;
                            sync_group_counter.sync_group_input_count++;

                            frame_to_next_sync_group_map.insert({ scene->sync_group_index, sync_group_counter });
                        }
                        else
                        {
                            it_sync_group_counter->second.sync_group_input_count++;
                        }
                    }

                    continue;
                }

                cpp_ffmpeg_wrapper_frame_to_next(it_ffmpeg_data->second.ffmpeg_instance);
            }
        }

        std::map<u32, SyncGroupCounter>::iterator it_frame_to_next_sync_group = frame_to_next_sync_group_map.begin();
        for (; it_frame_to_next_sync_group != frame_to_next_sync_group_map.end(); it_frame_to_next_sync_group++)
        {
            if (it_frame_to_next_sync_group->second.sync_group_count == it_frame_to_next_sync_group->second.sync_group_input_count)
            {
                auto it_ffmpeg_data = _ffmpeg_data_map.begin();
                for (; it_ffmpeg_data != _ffmpeg_data_map.end(); it_ffmpeg_data++)
                {
                    if (it_ffmpeg_data->second.sync_group_index == it_frame_to_next_sync_group->first)
                    {
                        cpp_ffmpeg_wrapper_frame_to_next(it_ffmpeg_data->second.ffmpeg_instance);
                    }
                }
            }
        }

        frame_to_next_sync_group_map.clear();
    }

    return u32();
}

/// <summary>
/// 
/// </summary>
/// <param name="rect"></param>
/// <returns> created scene_index </returns>
u32 create_scene_data(RECT rect, u32 sync_group_index, u16 sync_group_count)
{
    Scene* scene = new Scene();

    scene->rect = rect;

    scene->sync_group_index = sync_group_index;
    scene->sync_group_count = sync_group_count;

    if (_free_scene_queue.empty())
    {
        scene->scene_index = _next_scene_index;
        _next_scene_index++;
    }
    else
    {
        scene->scene_index = _free_scene_queue.front();
        _free_scene_queue.pop_front();
    }

    for (auto data : _graphics_data_list)
    {
        s32 texture_selected_index = -1;

        for (auto output : data->output_list)
        {
            //
            // base right - target left > 0 : base right > target left
            // base bottom - target top > 0 : base bottom > target top
            // base left - target right < 0 : base left < target right
            // base top - target bottom < 0 : base top < target bottom
            //

            if (_create_one_swapchain_for_each_adapter == true)
            {
                if (!(output->create_one_swapchain_for_each_adapter_rect.right > rect.left &&
                    output->create_one_swapchain_for_each_adapter_rect.bottom > rect.top &&
                    output->create_one_swapchain_for_each_adapter_rect.left < rect.right &&
                    output->create_one_swapchain_for_each_adapter_rect.top < rect.bottom))
                {
                    continue;
                }
            }
            else if (_create_one_swapchain_for_each_adapter_without_control_output == true)
            {
                if (!(output->create_one_swapchain_for_each_adapter_without_control_output_rect.right > rect.left &&
                    output->create_one_swapchain_for_each_adapter_without_control_output_rect.bottom > rect.top &&
                    output->create_one_swapchain_for_each_adapter_without_control_output_rect.left < rect.right &&
                    output->create_one_swapchain_for_each_adapter_without_control_output_rect.top < rect.bottom))
                {
                    continue;
                }
            }
            else
            {
                if (!(output->output_desc.DesktopCoordinates.right > rect.left &&
                    output->output_desc.DesktopCoordinates.bottom > rect.top &&
                    output->output_desc.DesktopCoordinates.left < rect.right &&
                    output->output_desc.DesktopCoordinates.top < rect.bottom))
                {
                    continue;
                }
            }

            scene->texture_upload_to_adapter_flag_map.insert({ data->adapter_index, false });

            Panel* panel = new Panel();
            scene->panel_list.push_back(panel);

            panel->adapter_index = data->adapter_index;

            if (data->free_vertex_queue.empty())
            {
                panel->vertex_index = data->next_vertex_index;
                data->next_vertex_index++;
            }
            else
            {
                panel->vertex_index = data->free_vertex_queue.front();
                data->free_vertex_queue.pop_front();
            }

            if (texture_selected_index == -1)
            {
                if (data->free_texture_queue.empty())
                {
                    panel->texture_index = data->next_texture_index;
                    data->next_texture_index++;
                }
                else
                {
                    panel->texture_index = data->free_texture_queue.front();
                    data->free_texture_queue.pop_front();
                }

                texture_selected_index = panel->texture_index;

                if (_texture_create_each_panel == true)
                {
                    texture_selected_index = -1;
                }
            }
            else
            {
                panel->texture_index = texture_selected_index;
            }

            panel->output_index = output->output_index;

            //
            // rect 계산
            // Output : O, Target : T, Left : L, Right : R, Top : T, Bottom : B
            // 
            // O.L < T.L : T.L
            // O.L = T.L : O.L - 1
            // O.L > T.L : O.L - 1
            // 
            // O.R < T.R : O.R + 1
            // O.R = T.R : O.R + 1
            // O.R > T.R : T.R
            // 
            // O.T < T.T : T.T
            // O.T = T.T : O.T - 1
            // O.T > T.T : O.T - 1
            // 
            // O.B < T.B : O.B + 1
            // O.B = T.B : O.B + 1
            // O.B > T.B : T.B
            //

            if (_create_one_swapchain_for_each_adapter == true)
            {
                if (output->create_one_swapchain_for_each_adapter_rect.left < rect.left)
                {
                    panel->rect.left = rect.left;
                }
                else
                {
                    if (_scene_panel_coordinate_correction == true)
                    {
                        panel->rect.left = output->create_one_swapchain_for_each_adapter_rect.left - 1;
                    }
                    else
                    {
                        panel->rect.left = output->create_one_swapchain_for_each_adapter_rect.left;
                    }
                }

                if (output->create_one_swapchain_for_each_adapter_rect.right > rect.right)
                {
                    panel->rect.right = rect.right;
                }
                else
                {
                    if (_scene_panel_coordinate_correction == true)
                    {
                        panel->rect.right = output->create_one_swapchain_for_each_adapter_rect.right + 1;
                    }
                    else
                    {
                        panel->rect.right = output->create_one_swapchain_for_each_adapter_rect.right;
                    }
                }

                if (output->create_one_swapchain_for_each_adapter_rect.top < rect.top)
                {
                    panel->rect.top = rect.top;
                }
                else
                {
                    if (_scene_panel_coordinate_correction == true)
                    {
                        panel->rect.top = output->create_one_swapchain_for_each_adapter_rect.top - 1;
                    }
                    else
                    {
                        panel->rect.top = output->create_one_swapchain_for_each_adapter_rect.top;
                    }
                }

                if (output->create_one_swapchain_for_each_adapter_rect.bottom > rect.bottom)
                {
                    panel->rect.bottom = rect.bottom;
                }
                else
                {
                    if (_scene_panel_coordinate_correction == true)
                    {
                        panel->rect.bottom = output->create_one_swapchain_for_each_adapter_rect.bottom + 1;
                    }
                    else
                    {
                        panel->rect.bottom = output->create_one_swapchain_for_each_adapter_rect.bottom;
                    }
                }

                normalize_rect(output->create_one_swapchain_for_each_adapter_rect, panel->rect, panel->normalized_rect);
            }
            else if (_create_one_swapchain_for_each_adapter_without_control_output == true)
            {
                if (output->create_one_swapchain_for_each_adapter_without_control_output_rect.left < rect.left)
                {
                    panel->rect.left = rect.left;
                }
                else
                {
                    if (_scene_panel_coordinate_correction == true)
                    {
                        panel->rect.left = output->create_one_swapchain_for_each_adapter_without_control_output_rect.left - 1;
                    }
                    else
                    {
                        panel->rect.left = output->create_one_swapchain_for_each_adapter_without_control_output_rect.left;
                    }
                }

                if (output->create_one_swapchain_for_each_adapter_without_control_output_rect.right > rect.right)
                {
                    panel->rect.right = rect.right;
                }
                else
                {
                    if (_scene_panel_coordinate_correction == true)
                    {
                        panel->rect.right = output->create_one_swapchain_for_each_adapter_without_control_output_rect.right + 1;
                    }
                    else
                    {
                        panel->rect.right = output->create_one_swapchain_for_each_adapter_without_control_output_rect.right;
                    }
                }

                if (output->create_one_swapchain_for_each_adapter_without_control_output_rect.top < rect.top)
                {
                    panel->rect.top = rect.top;
                }
                else
                {
                    if (_scene_panel_coordinate_correction == true)
                    {
                        panel->rect.top = output->create_one_swapchain_for_each_adapter_without_control_output_rect.top - 1;

                    }
                    else
                    {
                        panel->rect.top = output->create_one_swapchain_for_each_adapter_without_control_output_rect.top;
                    }
                }

                if (output->create_one_swapchain_for_each_adapter_without_control_output_rect.bottom > rect.bottom)
                {
                    panel->rect.bottom = rect.bottom;
                }
                else
                {
                    if (_scene_panel_coordinate_correction == true)
                    {
                        panel->rect.bottom = output->create_one_swapchain_for_each_adapter_without_control_output_rect.bottom + 1;
                    }
                    else
                    {
                        panel->rect.bottom = output->create_one_swapchain_for_each_adapter_without_control_output_rect.bottom;
                    }
                }

                normalize_rect(output->create_one_swapchain_for_each_adapter_without_control_output_rect, panel->rect, panel->normalized_rect);
            }
            else
            {
                if (output->output_desc.DesktopCoordinates.left < rect.left)
                {
                    panel->rect.left = rect.left;
                }
                else
                {
                    if (_scene_panel_coordinate_correction == true)
                    {
                        panel->rect.left = output->output_desc.DesktopCoordinates.left - 1;
                    }
                    else
                    {
                        panel->rect.left = output->output_desc.DesktopCoordinates.left;
                    }
                }

                if (output->output_desc.DesktopCoordinates.right > rect.right)
                {
                    panel->rect.right = rect.right;
                }
                else
                {
                    if (_scene_panel_coordinate_correction == true)
                    {
                        panel->rect.right = output->output_desc.DesktopCoordinates.right + 1;
                    }
                    else
                    {
                        panel->rect.right = output->output_desc.DesktopCoordinates.right;
                    }
                }

                if (output->output_desc.DesktopCoordinates.top < rect.top)
                {
                    panel->rect.top = rect.top;
                }
                else
                {
                    if (_scene_panel_coordinate_correction == true)
                    {
                        panel->rect.top = output->output_desc.DesktopCoordinates.top - 1;
                    }
                    else
                    {
                        panel->rect.top = output->output_desc.DesktopCoordinates.top;
                    }
                }

                if (output->output_desc.DesktopCoordinates.bottom > rect.bottom)
                {
                    panel->rect.bottom = rect.bottom;
                }
                else
                {
                    if (_scene_panel_coordinate_correction == true)
                    {
                        panel->rect.bottom = output->output_desc.DesktopCoordinates.bottom + 1;
                    }
                    else
                    {
                        panel->rect.bottom = output->output_desc.DesktopCoordinates.bottom;
                    }
                }

                normalize_rect(output->output_desc.DesktopCoordinates, panel->rect, panel->normalized_rect);
            }
        }
    }

    _graphics_insert_list.push_back(scene);

    return scene->scene_index;
}

float normalize_min_max(int min, int max, int target, int normalized_min = 0, int normalized_max = 1)
{
    return (float(target - min) / float(max - min) * (normalized_max - normalized_min)) + (normalized_min);
}

u32 normalize_rect(RECT base_rect, RECT target_rect, NormalizedRect& normalized_rect)
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

    return u32();
}

u32 normalize_uv(RECT base_rect, RECT target_rect, NormalizedRect& normalized_uv)
{
    normalized_uv.left = normalize_min_max(base_rect.left, base_rect.right, target_rect.left, 0, 1);
    normalized_uv.right = normalize_min_max(base_rect.left, base_rect.right, target_rect.right, 0, 1);
    normalized_uv.top = normalize_min_max(base_rect.top, base_rect.bottom, target_rect.top, 0, 1);
    normalized_uv.bottom = normalize_min_max(base_rect.top, base_rect.bottom, target_rect.bottom, 0, 1);

    if (normalized_uv.left < 0.0f)
    {
        normalized_uv.left = 0.0f;
    }

    if (normalized_uv.top < 0.0f)
    {
        normalized_uv.top = 0.0f;
    }

    if (normalized_uv.right > 1.0f)
    {
        normalized_uv.right = 1.0f;
    }

    if (normalized_uv.bottom > 1.0f)
    {
        normalized_uv.bottom = 1.0f;
    }

    return u32();
}

u32 delete_scene_data(u32 scene_index)
{
    std::lock_guard<std::mutex> lock(_graphics_remove_mutex);

    _graphics_remove_queue.push_back({ scene_index, false });

    return u32();
}

u32 delete_scene_datas()
{
    std::lock_guard<std::mutex> lock(_graphics_remove_mutex);

    for (auto it_scene = _graphics_scene_list.begin(); it_scene != _graphics_scene_list.end();)
    {
        Scene* scene = *it_scene;

        for (auto it_panel = scene->panel_list.begin(); it_panel != scene->panel_list.end();)
        {
            Panel* panel = *it_panel;

            if (_texture_create_each_panel == true)
            {
                delete[] panel->separated_frame_data_y;
                panel->separated_frame_data_y = nullptr;

                delete[] panel->separated_frame_data_u;
                panel->separated_frame_data_u = nullptr;

                delete[] panel->separated_frame_data_v;
                panel->separated_frame_data_v = nullptr;
            }

            delete panel;
            it_panel = scene->panel_list.erase(it_panel);
        }

        if (scene->panel_list.empty())
        {
            delete scene;
            it_scene = _graphics_scene_list.erase(it_scene);
        }
        else
        {
            it_scene++;
        }
    }

    return u32();
}

void callback_data_connection_server(void* data, void* connection)
{
    packet_header* header = (packet_header*)data;

    void* packet = new char[header->size];
    memcpy(packet, data, header->size);

    {
        std::lock_guard<std::mutex> lk(_tcp_processing_mutex);
        _tcp_processing_command_queue.push_back(std::pair<void*, void*>(packet, connection));
    }
}

void server_thread()
{
    cppsocket_network_initialize();

    _server = cppsocket_server_create();

    cppsocket_server_set_callback_data_connection(_server, callback_data_connection_server);

    if (cppsocket_server_initialize(_server, _ip.c_str(), _port))
    {
        while (_tcp_server_flag)
        {
            cppsocket_server_frame(_server);
        }
    }

    cppsocket_server_delete(_server);

    cppsocket_network_shutdown();
}

/// <summary>
/// 
/// </summary>
/// <param name="index"> scene index </param>
void callback_ffmpeg_wrapper_ptr(void* param)
{
    ffmpeg_wrapper_callback_data* param_data = (ffmpeg_wrapper_callback_data*)param;

    FFmpegProcessingCommand data{};

    data.scene_index = param_data->scene_index;
    data.command = param_data->command;
    data.connection = param_data->connection;
    data.result = param_data->result;

    data.left = param_data->left;
    data.top = param_data->top;
    data.width = param_data->width;
    data.height = param_data->height;
    if (param_data->url_size != u16_invalid_id)
    {
        memcpy(data.url, param_data->url, param_data->url_size);
    }
    data.url_size = param_data->url_size;

    data.sync_group_index = param_data->sync_group_index;
    data.sync_group_count = param_data->sync_group_count;

    std::lock_guard<std::mutex> lk(_ffmpeg_processing_mutex);
    _ffmpeg_processing_command_queue.push_back(data);
}

void delete_ffmpeg_instances()
{   
    std::vector<FFmpegInstanceData> ffmpeg_instances;

    for (auto it = _ffmpeg_data_map.begin(); it != _ffmpeg_data_map.end(); it++)
    {
        ffmpeg_instances.push_back(it->second);
    }

    for (auto it : ffmpeg_instances)
    {
        cpp_ffmpeg_wrapper_play_stop(it.ffmpeg_instance, nullptr);
    }
}

u32 upload_texture(graphics_data* data, AVFrame* frame, s32 target_texture_index, s32 output_frame_index)
{
    D3D12_SUBRESOURCE_DATA texture_data_y{};
    texture_data_y.pData = frame->data[0];
    texture_data_y.RowPitch = frame->linesize[0];
    texture_data_y.SlicePitch = texture_data_y.RowPitch * frame->height;

    CD3DX12_RESOURCE_BARRIER transition_barrier_y = CD3DX12_RESOURCE_BARRIER::Transition(data->texture_map_y[output_frame_index][target_texture_index], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    data->cmd_list->ResourceBarrier(1, &transition_barrier_y);
    UpdateSubresources(data->cmd_list, data->texture_map_y[output_frame_index][target_texture_index], data->upload_texture_map_y[output_frame_index][target_texture_index], 0, 0, 1, &texture_data_y);
    transition_barrier_y = CD3DX12_RESOURCE_BARRIER::Transition(data->texture_map_y[output_frame_index][target_texture_index], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    data->cmd_list->ResourceBarrier(1, &transition_barrier_y);

    D3D12_SUBRESOURCE_DATA texture_data_u{};
    texture_data_u.pData = frame->data[1];
    texture_data_u.RowPitch = frame->linesize[1];
    texture_data_u.SlicePitch = texture_data_u.RowPitch * frame->height / 2;

    CD3DX12_RESOURCE_BARRIER transition_barrier_u = CD3DX12_RESOURCE_BARRIER::Transition(data->texture_map_u[output_frame_index][target_texture_index], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    data->cmd_list->ResourceBarrier(1, &transition_barrier_u);
    UpdateSubresources(data->cmd_list, data->texture_map_u[output_frame_index][target_texture_index], data->upload_texture_map_u[output_frame_index][target_texture_index], 0, 0, 1, &texture_data_u);
    transition_barrier_u = CD3DX12_RESOURCE_BARRIER::Transition(data->texture_map_u[output_frame_index][target_texture_index], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    data->cmd_list->ResourceBarrier(1, &transition_barrier_u);

    D3D12_SUBRESOURCE_DATA texture_data_v{};
    texture_data_v.pData = frame->data[2];
    texture_data_v.RowPitch = frame->linesize[2];
    texture_data_v.SlicePitch = texture_data_v.RowPitch * frame->height / 2;

    CD3DX12_RESOURCE_BARRIER transition_barrier_v = CD3DX12_RESOURCE_BARRIER::Transition(data->texture_map_v[output_frame_index][target_texture_index], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    data->cmd_list->ResourceBarrier(1, &transition_barrier_v);
    UpdateSubresources(data->cmd_list, data->texture_map_v[output_frame_index][target_texture_index], data->upload_texture_map_v[output_frame_index][target_texture_index], 0, 0, 1, &texture_data_v);
    transition_barrier_v = CD3DX12_RESOURCE_BARRIER::Transition(data->texture_map_v[output_frame_index][target_texture_index], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    data->cmd_list->ResourceBarrier(1, &transition_barrier_v);

    return u32();
}

u32 upload_texture_each_panel(graphics_data* data, AVFrame* frame, s32 output_frame_index, Panel* panel)
{
    if (panel->separated_frame_data_y == nullptr)
    {
        // yuv420의 linesize
        panel->separated_frame_linesize_y = frame->width * (panel->normalized_uv.right - panel->normalized_uv.left);
        panel->separated_frame_linesize_u = panel->separated_frame_linesize_y / 2;
        panel->separated_frame_linesize_v = panel->separated_frame_linesize_y / 2;
    
        panel->separated_frame_height = frame->height * (panel->normalized_uv.bottom - panel->normalized_uv.top);

        panel->separated_frame_data_y = new uint8_t[panel->separated_frame_linesize_y * panel->separated_frame_height];
        panel->separated_frame_data_u = new uint8_t[panel->separated_frame_linesize_u * panel->separated_frame_height / 2];
        panel->separated_frame_data_v = new uint8_t[panel->separated_frame_linesize_v * panel->separated_frame_height / 2];
    }

    int offset = 0;

    int offset_start_y = frame->linesize[0] * (frame->height * panel->normalized_uv.top) + (frame->linesize[0] * panel->normalized_uv.left);
    for (size_t i = 0; i < panel->separated_frame_height; i++)
    {
        memcpy(panel->separated_frame_data_y + offset, frame->data[0] + offset_start_y + (frame->linesize[0] * i), panel->separated_frame_linesize_y);
        offset += panel->separated_frame_linesize_y;
    }

    offset = 0;
    int offset_start_u = frame->linesize[1] * ((frame->height * panel->normalized_uv.top) / 2) + (frame->linesize[1] * panel->normalized_uv.left);
    for (size_t i = 0; i < panel->separated_frame_height / 2; i++)
    {
        memcpy(panel->separated_frame_data_u + offset, frame->data[1] + offset_start_u + (frame->linesize[1] * i), panel->separated_frame_linesize_u);
        offset += panel->separated_frame_linesize_u;
    }

    offset = 0;
    int offset_start_v = frame->linesize[2] * ((frame->height * panel->normalized_uv.top) / 2) + (frame->linesize[2] * panel->normalized_uv.left);
    for (size_t i = 0; i < panel->separated_frame_height / 2; i++)
    {
        memcpy(panel->separated_frame_data_v + offset, frame->data[2] + offset_start_v + (frame->linesize[2] * i), panel->separated_frame_linesize_v);
        offset += panel->separated_frame_linesize_v;
    }

    D3D12_SUBRESOURCE_DATA texture_data_y{};
    texture_data_y.pData = panel->separated_frame_data_y;
    texture_data_y.RowPitch = panel->separated_frame_linesize_y;
    texture_data_y.SlicePitch = texture_data_y.RowPitch * panel->separated_frame_height;

    CD3DX12_RESOURCE_BARRIER transition_barrier_y = CD3DX12_RESOURCE_BARRIER::Transition(data->texture_map_y[output_frame_index][panel->texture_index], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    data->cmd_list->ResourceBarrier(1, &transition_barrier_y);
    UpdateSubresources(data->cmd_list, data->texture_map_y[output_frame_index][panel->texture_index], data->upload_texture_map_y[output_frame_index][panel->texture_index], 0, 0, 1, &texture_data_y);
    transition_barrier_y = CD3DX12_RESOURCE_BARRIER::Transition(data->texture_map_y[output_frame_index][panel->texture_index], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    data->cmd_list->ResourceBarrier(1, &transition_barrier_y);

    D3D12_SUBRESOURCE_DATA texture_data_u{};
    texture_data_u.pData = panel->separated_frame_data_u;
    texture_data_u.RowPitch = panel->separated_frame_linesize_u;
    texture_data_u.SlicePitch = texture_data_u.RowPitch * panel->separated_frame_height / 2;

    CD3DX12_RESOURCE_BARRIER transition_barrier_u = CD3DX12_RESOURCE_BARRIER::Transition(data->texture_map_u[output_frame_index][panel->texture_index], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    data->cmd_list->ResourceBarrier(1, &transition_barrier_u);
    UpdateSubresources(data->cmd_list, data->texture_map_u[output_frame_index][panel->texture_index], data->upload_texture_map_u[output_frame_index][panel->texture_index], 0, 0, 1, &texture_data_u);
    transition_barrier_u = CD3DX12_RESOURCE_BARRIER::Transition(data->texture_map_u[output_frame_index][panel->texture_index], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    data->cmd_list->ResourceBarrier(1, &transition_barrier_u);

    D3D12_SUBRESOURCE_DATA texture_data_v{};
    texture_data_v.pData = panel->separated_frame_data_v;
    texture_data_v.RowPitch = panel->separated_frame_linesize_v;
    texture_data_v.SlicePitch = texture_data_v.RowPitch * panel->separated_frame_height / 2;

    CD3DX12_RESOURCE_BARRIER transition_barrier_v = CD3DX12_RESOURCE_BARRIER::Transition(data->texture_map_v[output_frame_index][panel->texture_index], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    data->cmd_list->ResourceBarrier(1, &transition_barrier_v);
    UpdateSubresources(data->cmd_list, data->texture_map_v[output_frame_index][panel->texture_index], data->upload_texture_map_v[output_frame_index][panel->texture_index], 0, 0, 1, &texture_data_v);
    transition_barrier_v = CD3DX12_RESOURCE_BARRIER::Transition(data->texture_map_v[output_frame_index][panel->texture_index], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    data->cmd_list->ResourceBarrier(1, &transition_barrier_v);

    return u32();
}

u32 deferred_free_processing(u32 back_buffer_index)
{
    for (auto data : _graphics_data_list)
    {
        for (auto it = data->deferred_free_object_list[back_buffer_index].begin(); it != data->deferred_free_object_list[back_buffer_index].end();)
        {
            deferred_free_object object = (*it);

            object.resource->Release();

            switch (object.type)
            {
            case deferred_type::vertex_upload_buffer:
            {
                data->free_vertex_queue.push_back(object.index);
            }
            break;
            case deferred_type::upload_texture_v:
            {
                if (object.free_flag)
                {
                    data->free_texture_queue.push_back(object.index);
                }
            }
            break;
            default:
                break;
            }

            it = data->deferred_free_object_list[back_buffer_index].erase(it);
        }
    }

    return u32();
}

u32 delete_present_barriers()
{
    if (_graphics_data_list.empty())
    {
        return u32();
    }

    if (_disable_present_barrier == false)
    {
        for (auto data : _graphics_data_list)
        {
            for (auto output : data->output_list)
            {
                if (output->handle == nullptr)
                {
                    continue;
                }

                if (output->present_barrier_joined == true)
                {
                    _nvapi_status = NvAPI_LeavePresentBarrier(output->present_barrier_client);
                    output->present_barrier_joined = false;
                }

                _nvapi_status = NvAPI_DestroyPresentBarrierClient(output->present_barrier_client);
                output->present_barrier_client = nullptr;

                output->present_barrier_fence->Release();
                output->present_barrier_fence = nullptr;
            }
        }
    }

    return u32();
}

void config_setting()
{
    wchar_t path_w[260] = { 0, };
    GetModuleFileName(nullptr, path_w, 260);
    std::wstring str_path_w = path_w;
    str_path_w = str_path_w.substr(0, str_path_w.find_last_of(L"\\/"));
    std::wstring str_ini_path_w = str_path_w + L"\\WPlayer.ini";

    char path_a[260] = { 0, };
    GetModuleFileNameA(nullptr, path_a, 260);
    std::string str_path_a = path_a;
    str_path_a = str_path_a.substr(0, str_path_a.find_last_of("\\/"));
    std::string str_ini_path_a = str_path_a + "\\WPlayer.ini";

    char result_a[255];
    wchar_t result_w[255];
    int result_i = 0;

    GetPrivateProfileStringA("WPlayer", "IP", "", result_a, 255, str_ini_path_a.c_str());
    _ip = result_a;

    GetPrivateProfileString(L"WPlayer", L"PORT", L"0", result_w, 255, str_ini_path_w.c_str());
    _port = _ttoi(result_w);

    GetPrivateProfileString(L"WPlayer", L"texture_create_each_panel", L"1", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _texture_create_each_panel = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"disable_present_barrier", L"1", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _disable_present_barrier = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"test_window_count", L"0", result_w, 255, str_ini_path_w.c_str());
    _test_window_count = _ttoi(result_w);

    GetPrivateProfileString(L"WPlayer", L"window_create_position_shift_up", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _window_create_position_shift_up = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"create_one_swapchain_for_each_adapter", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _create_one_swapchain_for_each_adapter = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"create_one_swapchain_for_each_adapter_window_width", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _create_one_swapchain_for_each_adapter_window_width = result_i;

    GetPrivateProfileString(L"WPlayer", L"create_one_swapchain_for_each_adapter_window_height", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _create_one_swapchain_for_each_adapter_window_height = result_i;

    GetPrivateProfileString(L"WPlayer", L"create_one_swapchain_for_each_adapter_without_control_output", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _create_one_swapchain_for_each_adapter_without_control_output = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"create_one_swapchain_for_each_adapter_without_control_output_excluded_window_left", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _create_one_swapchain_for_each_adapter_without_control_output_excluded_window_left = result_i;

    GetPrivateProfileString(L"WPlayer", L"create_one_swapchain_for_each_adapter_without_control_output_excluded_window_top", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _create_one_swapchain_for_each_adapter_without_control_output_excluded_window_top = result_i;

    GetPrivateProfileString(L"WPlayer", L"create_one_swapchain_for_each_adapter_without_control_output_excluded_window_right", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _create_one_swapchain_for_each_adapter_without_control_output_excluded_window_right = result_i;

    GetPrivateProfileString(L"WPlayer", L"create_one_swapchain_for_each_adapter_without_control_output_excluded_window_bottom", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _create_one_swapchain_for_each_adapter_without_control_output_excluded_window_bottom = result_i;

    GetPrivateProfileString(L"WPlayer", L"sync_group_frame_numbering", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _sync_group_frame_numbering = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"hw_accel", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _hw_accel = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"hw_accel_device_type", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _hw_accel_device_type = result_i;

    GetPrivateProfileString(L"WPlayer", L"hw_accel_adapter_index", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _hw_accel_adapter_index = result_i;

    GetPrivateProfileString(L"WPlayer", L"scene_panel_coordinate_correction", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _scene_panel_coordinate_correction = result_i == 0 ? false : true;
}

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

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

    _tcp_processing_thread = std::thread(tcp_processing_thread);
    _tcp_thread = std::thread(server_thread);
    _ffmpeg_processing_thread = std::thread(ffmpeg_processing_thread);

    wchar_t asset_path[512];
    get_asset_path(asset_path, _countof(asset_path));
    _asset_path = asset_path;

    create_factory();
    enum_adapters();
    enum_output_list();

    for (auto data : _graphics_data_list)
    {
        if (data->output_list.empty())
        {
            continue;
        }

        int n = 0;

        for (auto output : data->output_list)
        {
            if (_create_one_swapchain_for_each_adapter == true)
            {
                create_window(szWindowClass, szTitle, hInst, output->create_one_swapchain_for_each_adapter_rect, nullptr, output->handle);
            }
            else if (_create_one_swapchain_for_each_adapter_without_control_output == true)
            {
                create_window(szWindowClass, szTitle, hInst, output->create_one_swapchain_for_each_adapter_without_control_output_rect, nullptr, output->handle);
            }
            else
            {
                create_window(szWindowClass, szTitle, hInst, output->output_desc.DesktopCoordinates, nullptr, output->handle);
            }
            
            n++;

            if (n == _test_window_count)
            {
                break;
            }
        }
    }

    create_viewports();

    create_devices();
    create_command_queues();

    if (_disable_present_barrier == false)
    {
        _nvapi_status = NvAPI_Initialize();
        // Check whether the system supports present barrier (Quadro + driver with support)
        for (auto data : _graphics_data_list)
        {
            _nvapi_status = NvAPI_D3D12_QueryPresentBarrierSupport(data->device, &data->present_barrier_supported);
            if (data->present_barrier_supported == false)
            {
                _present_barrier_supported = false;
            }

            if (_nvapi_status != NVAPI_OK || data->present_barrier_supported == false)
            {
                OutputDebugStringA("Present barrier is not supported on this system\n");
                continue;
                // return false;
            }

            for (auto output : data->output_list)
            {
                if (output->handle == nullptr)
                {
                    continue;
                }

                data->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&output->present_barrier_fence));
                NAME_D3D12_OBJECT_INDEXED_2(output->present_barrier_fence, data->adapter_index, output->output_index, L"ID3D12Fence_Nv");
            }
        }
    }

    create_swap_chains();
    create_rtv_heaps();
    create_srv_heaps();

    if (_disable_present_barrier == false)
    {
        for (auto data : _graphics_data_list)
        {
            for (auto output : data->output_list)
            {
                if (output->handle == nullptr)
                {
                    continue;
                }

                _nvapi_status = NvAPI_D3D12_RegisterPresentBarrierResources(output->present_barrier_client, output->present_barrier_fence, output->rtv_view_list.data(), static_cast<NvU32>(output->rtv_view_list.size()));
            }
        }
    }

    create_root_signatures();
    create_psos();
    create_command_lists();

    create_fences();

    if (_disable_present_barrier == false)
    {
        for (auto data : _graphics_data_list)
        {
            for (auto output : data->output_list)
            {
                if (output->handle == nullptr)
                {
                    continue;
                }

                if (output->present_barrier_joined == false)
                {
                    NV_JOIN_PRESENT_BARRIER_PARAMS params = {};
                    params.dwVersion = NV_JOIN_PRESENT_BARRIER_PARAMS_VER1;
                    _nvapi_status = NvAPI_JoinPresentBarrier(output->present_barrier_client, &params);
                    output->present_barrier_joined = true;
                }

                output->present_barrier_frame_stats.dwVersion = NV_PRESENT_BARRIER_FRAME_STATICS_VER1;
            }
        }
    }
    ready = true;

    MSG msg;
    bool is_running = true;
    // 기본 메시지 루프입니다:
    while (is_running)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            is_running &= (msg.message != WM_QUIT);
        }
        if (ready)
        {
            render();
        }
    }

    wait_for_gpus();

    if (_tcp_thread.joinable())
    {
        _tcp_server_flag = false;
        _tcp_thread.join();
    }

    if (_tcp_processing_thread.joinable())
    {
        _tcp_processing_flag = false;
        _tcp_processing_thread.join();
    }

    delete_ffmpeg_instances();

    if (_ffmpeg_processing_thread.joinable())
    {
        while (_ffmpeg_processing_command_queue.empty() == false)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_ffmpeg_processing));
        }

        _ffmpeg_processing_flag = false;
        _ffmpeg_processing_thread.join();
    }
    
    delete_scene_datas();

    delete_textures();
    delete_vertex_buffer_list();
    delete_index_buffer();

    delete_present_barriers();

    delete_fences();

    delete_command_lists();
    delete_psos();
    delete_root_signatures();

    delete_srv_heaps();
    delete_rtv_heaps();
    delete_swap_chains();
    delete_command_queues();
    delete_devices();
    delete_output_list();
    delete_adapters();
    delete_factory();

    if (_disable_present_barrier == false)
    {
        _nvapi_status = NvAPI_Unload();
    }

#if _DEBUG
    d3d_memory_check();
#endif

    return (int)msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    //wcex.style = CS_HREDRAW | CS_VREDRAW;
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

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
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
