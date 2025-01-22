// WPlayer.cpp : ���ø����̼ǿ� ���� �������� �����մϴ�.
//

#include "framework.h"
#include "WPlayer.h"

// --------------------------------

#if _DEBUG
#include <crtdbg.h>
#endif // _DEBUG

#include <thread>
#include <mutex>
#include <deque>
#include <map>
#include <shared_mutex>

#if _DEBUG
#include "dxgidebug.h"
#endif // _DEBUG

#include "GraphicsHeaders.h"

#include <d2d1_3.h>
#pragma comment(lib, "D2d1")

#include <dwrite.h>
#pragma comment(lib, "Dwrite")

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
#include "libswscale/swscale.h"
}

#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

#include "StringConverter.h"

#include "CppCefWrapperAPI.h"
#pragma comment(lib, "CppCefWrapper.lib")

// --------------------------------

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"

#pragma comment(lib, "spdlog.lib")

// --------------------------------

#define MAX_LOADSTRING 100

#define DEFAULT_TEXT_STRING L"Test Text"
#define DEFAULT_TEXT_COLOR 1.0f
#define DEFAULT_TEXT_SIZE 32
#define DEFAULT_TEXT_FONT_FAMILY L"Arial"
#define DEFAULT_TEXT_COLOR_BACKGROUND 0.0f
#define DEFAULT_TEXT_BACKGROUND_RECTANGLE 0.0f
#define DEFAULT_TEXT_WEIGHT e_dwrite_font_weight::DWRITE_FONT_WEIGHT_NORMAL
#define DEFAULT_TEXT_STYLE e_dwrite_font_style::DWRITE_FONT_STYLE_NORMAL
#define DEFAULT_TEXT_STRETCH e_dwrite_font_stretch::DWRITE_FONT_STRETCH_NORMAL
#define DEFAULT_TEXT_MOVEMENT_TYPE 0
#define DEFAULT_TEXT_MOVEMENT_SPEED 0.0f
#define DEFAULT_TEXT_MOVEMENT_THRESHOLD FLT_MIN
#define DEFAULT_TEXT_MOVEMENT_THRESHOLD_PACKET INT_MIN
#define DEFAULT_TEXT_MOVEMENT_TRANSLATION 0.0f

#define PRESENT_COUNT_PER_SECOND 60.0f

// --------------------------------

#pragma region Enumerations

/// <summary>
/// ��������� ���� ������ �����ϱ� ���� ��ü���� Ÿ�� ����
/// </summary>
enum class e_object_type
{
    none = 0,
    object_event = 1,
    object_condition_variable = 2,
    object_mutex = 3,
    object_flag = 4,

};

/// <summary>
/// ��������� ���� ���� ������ �����ϴ� enum
/// from - to �������� ������
/// </summary>
enum class e_wait_type
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

/// <summary>
/// int ���� Rect�� float ������ ����ȭ�Ͽ� ������ �� ����� ����ü
/// </summary>
struct NormalizedRect
{
    float left;
    float top;
    float right;
    float bottom;
};

/// <summary>
/// Vertex Shader�� �ǳ��� ������ ���� ���� ����ü
/// </summary>
struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 uv;

};

/// <summary>
/// ������ ���� ������ �����ϴ� ��ü�� ����ü
/// </summary>
struct st_input_object
{
    void* object = nullptr;

    e_object_type object_type = e_object_type::none;
    e_wait_type wait_type = e_wait_type::none;

};

/// <summary>
/// int ���� color�� float ������ ����ȭ�Ͽ� ������ �� ����� ����ü
/// </summary>
typedef struct st_color
{
    float r;
    float g;
    float b;
    float a;

}*pst_color;

typedef struct st_cef_data_paint
{
    void* buffer;
    int width;
    int height;
    void* nv12_buffer;
}*pst_cef_data_paint;

typedef struct st_texture_cef
{
    std::map<UINT, ID3D12Resource*> map_texture;
    std::map<UINT, ID3D12Resource*> map_upload_heap_luminance;
    std::map<UINT, ID3D12Resource*> map_upload_heap_chrominance;
}*pst_texture_cef;

/// <summary>
/// �׷��� ī�� �߻�ȭ ��ü
/// </summary>
typedef struct st_adapter
{
    IDXGIAdapter1* adapter = nullptr;
    DXGI_ADAPTER_DESC1 adapter_desc{};

    UINT adapter_index = UINT_MAX;

}*pst_adapter;

/// <summary>
/// �׷��� ī���� ��� ���� ��ġ �߻�ȭ ��ü
/// </summary>
typedef struct st_device
{
    ID3D12Device* device = nullptr;

    UINT device_index = UINT_MAX;

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

    bool flag_ready_to_device_use = false;

    bool flag_use_last_frame = false;

    ID3D12Resource* texture_default = nullptr;
    ID3D12Resource* upload_heap_texture_default_luminance = nullptr;
    ID3D12Resource* upload_heap_texture_default_chrominance = nullptr;

    /// <summary>
    /// �ڸ� �������� ���� �ʿ��� ��ü
    /// </summary>
    ID3D11Device* device_11 = nullptr;
    /// <summary>
    /// �ڸ� �������� ���� �ʿ��� ��ü
    /// </summary>
    ID3D11DeviceContext* device_context_11 = nullptr;
    /// <summary>
    /// �ڸ� �������� ���� �ʿ��� ��ü
    /// </summary>
    ID3D11On12Device* device_11_on_12 = nullptr;
    /// <summary>
    /// �ڸ� �������� ���� �ʿ��� ��ü
    /// </summary>
    ID2D1Device2* device_2d = nullptr;
    /// <summary>
    /// �ڸ� �������� ���� �ʿ��� ��ü
    /// </summary>
    ID2D1DeviceContext2* device_context_2d = nullptr;

    RECT rect_connected = { INT_MAX, INT_MAX, INT_MIN, INT_MIN };

    std::shared_mutex* mutex_map_text_internal = nullptr;

    std::map<UINT, pst_texture_cef> map_texture_cef;

}*pst_device;

/// <summary>
/// �׷��� �����ġ(�����) �߻�ȭ ��ü
/// </summary>
typedef struct st_output
{
    IDXGIOutput* output = nullptr;
    DXGI_OUTPUT_DESC output_desc{};

    UINT output_index = UINT_MAX;
    UINT device_index = UINT_MAX;

}*pst_output;

/// <summary>
/// window �߻�ȭ ��ü
/// </summary>
typedef struct st_window
{
    HWND handle = nullptr;
    RECT rect{};

    UINT window_index = UINT_MAX;
    UINT output_index = UINT_MAX;
    UINT device_index = UINT_MAX;

    bool flag_thread_window = true;

    HANDLE event_window_to_scene = nullptr;

    std::condition_variable* condition_variable_device_to_window = nullptr;
    std::mutex* mutex_device_to_window = nullptr;
    bool flag_device_to_window = false;

    bool flag_first_entry = true;

    int counter_cef_create = 0;

}*pst_window;

/// <summary>
/// Device �� 1���� command queue�� ����
/// �׷��� ī��� ���� ����� ��� ������ �߻�ȭ ��ü
/// </summary>
typedef struct st_command_queue
{
    ID3D12CommandQueue* command_queue = nullptr;

    UINT device_index = UINT_MAX;

}*pst_command_queue;

/// <summary>
/// window �� 1���� swap chain�� ����
/// </summary>
typedef struct st_swap_chain
{
    IDXGISwapChain1* swap_chain = nullptr;

    UINT window_index = UINT_MAX;
    UINT device_index = UINT_MAX;

}*pst_swap_chain;

/// <summary>
/// rtv: render target view
/// �׷��� ī���� �޸𸮿��� rtv�� �Ҵ��� �޸� ���� �߻�ȭ ��ü
/// </summary>
typedef struct st_rtv_heap
{
    ID3D12DescriptorHeap* rtv_heap = nullptr;

    /// <summary>
    /// rtv 1���� ũ��
    /// </summary>
    UINT rtv_descriptor_size = UINT_MAX;

    UINT device_index = UINT_MAX;

}*pst_rtv_heap;

/// <summary>
/// rtv: render target view
/// swap chain�� backbuffer�� ����Ŵ
/// </summary>
typedef struct st_rtv
{
    std::vector<ID3D12Resource*> vector_rtv;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> vector_rtv_handle;

    UINT window_index = UINT_MAX;
    UINT device_index = UINT_MAX;

    /// <summary>
    /// �ڸ� �������� ���� �ʿ��� ��ü
    /// </summary>
    std::vector<ID3D11Resource*> vector_wrapped_back_buffer;
    /// <summary>
    /// �ڸ� �������� ���� �ʿ��� ��ü
    /// </summary>
    std::vector<IDXGISurface*> vector_surface;
    /// <summary>
    /// �ڸ� �������� ���� �ʿ��� ��ü
    /// </summary>
    std::vector<ID2D1Bitmap1*> vector_rtv_2d;

}*pst_rtv;

/// <summary>
/// srv: shader resource view
/// �׷��� ī���� �޸𸮿��� srv�� �Ҵ��� �޸� ���� �߻�ȭ ��ü
/// </summary>
typedef struct st_srv_heap
{
    ID3D12DescriptorHeap* srv_heap = nullptr;

    /// <summary>
    /// srv 1���� ũ��
    /// </summary>
    UINT srv_descriptor_size = UINT_MAX;

    UINT device_index = UINT_MAX;

}*pst_srv_heap;

/// <summary>
/// command queue�� command list ���̿� ��ġ�ϴ� �׷��� ��� �Ҵ��� �߻�ȭ ��ü
/// </summary>
typedef struct st_command_allocator
{
    std::vector<ID3D12CommandAllocator*> vector_command_allocator;

    UINT device_index = UINT_MAX;

}*pst_command_allocator;

/// <summary>
/// �ۼ����� ������ �����Ͽ� ��Ȯ�ϰ� ������ �� ����
/// </summary>
typedef struct st_root_signature
{
    ID3D12RootSignature* root_sig = nullptr;

    UINT device_index = UINT_MAX;

}*pst_root_signature;

/// <summary>
/// �ۼ����� ������ �����Ͽ� ��Ȯ�ϰ� ������ �� ����
/// </summary>
typedef struct st_pipeline_state_object
{
    ID3D12PipelineState* pso = nullptr;

    UINT device_index = UINT_MAX;

}*pst_pipeline_state_object, st_pso, * pst_pso;

/// <summary>
/// command allocator�� �׷��� ����� �����ϴ� �߻�ȭ ��ü
/// </summary>
typedef struct st_command_list
{
    std::vector<ID3D12GraphicsCommandList*> vector_command_list;

    UINT device_index = UINT_MAX;

}*pst_command_list;

/// <summary>
/// �׷��� ī���� ó���� ����Ű�� �߻�ȭ ��ü
/// </summary>
typedef struct st_fence
{
    ID3D12Fence* fence_device = nullptr;
    ID3D12Fence* fence_upload = nullptr;
    HANDLE fence_event_device = nullptr;
    HANDLE fence_event_upload = nullptr;

    uint64_t fence_value_device = 0;
    uint64_t fence_value_upload = 0;

    UINT device_index = UINT_MAX;

}*pst_fence;

/// <summary>
/// �ۼ����� ������ �����Ͽ� ��Ȯ�ϰ� ������ �� ����
/// </summary>
typedef struct st_viewport
{
    D3D12_VIEWPORT viewport{};
    D3D12_RECT scissor_rect{};

    UINT window_index = UINT_MAX;
    UINT device_index = UINT_MAX;

}*pst_viewport;

/// <summary>
/// vertex: �׷��� ��ǥ ���������� �Ѱ��� ���� �ǹ���
/// TopologyType�� ����� ����ϴ��Ŀ� ���� �׷����� ������� �޶���
/// Triangle�� ����� ��쿡�� 3���� vertex�� 1���� �ﰢ���� �׸��� ��
/// 
/// �׷��� ī���� �޸𸮿��� vertex shader�� �Է��� vertex �����͸� �Ҵ��ϸ� CPU���� ������ �� ���� �޸� ������ �߻�ȭ ��ü
/// </summary>
typedef struct st_vertex_buffer
{
    std::vector<ID3D12Resource*> vector_vertex_buffer;

    UINT device_index = UINT_MAX;

}*pst_vertex_buffer;

/// <summary>
/// �׷��� ī���� �޸𸮿� ���������� CPU���� Access�Ͽ� vertex�� ���� ������ �� �ִ� �޸� ������ �߻�ȭ ��ü
/// </summary>
typedef struct st_vertex_upload_buffer
{
    std::vector<ID3D12Resource*> vector_vertex_upload_buffer;

    UINT device_index = UINT_MAX;

}*pst_vertex_upload_buffer;

/// <summary>
/// �׷��� ī���� �޸𸮿��� vertex�� ������ ������ ������ �� ����ϴ� �߻�ȭ ��ü
/// </summary>
typedef struct st_vertex_buffer_view
{
    std::vector<D3D12_VERTEX_BUFFER_VIEW> vector_vertex_buffer_view{};

    UINT device_index = UINT_MAX;

}*pst_vertex_buffer_view;

/// <summary>
/// index: vertex shader���� vertex�� ����� ������ �����ϴ� ��
/// 
/// �׷��� ī���� �޸𸮿��� vertex shader�� �Է��� index �����͸� �Ҵ��ϸ� CPU���� ������ �� ���� �޸� ������ �߻�ȭ ��ü
/// </summary>
typedef struct st_index_buffer
{
    ID3D12Resource* index_buffer = nullptr;

    UINT device_index = UINT_MAX;

}*pst_index_buffer;

/// <summary>
/// �׷��� ī���� �޸𸮿� ���������� CPU���� Access�Ͽ� index�� ���� ������ �� �ִ� �޸� ������ �߻�ȭ ��ü
/// </summary>
typedef struct st_index_upload_buffer
{
    ID3D12Resource* index_upload_buffer = nullptr;

    UINT device_index = UINT_MAX;

}*pst_index_upload_buffer;

/// <summary>
/// �׷��� ī���� �޸𸮿��� index�� ������ ������ ������ �� ����ϴ� �߻�ȭ ��ü
/// </summary>
typedef struct st_index_buffer_view
{
    D3D12_INDEX_BUFFER_VIEW index_buffer_view{};

    UINT device_index = UINT_MAX;

}*pst_index_buffer_view;

/// <summary>
/// ������� ����
/// </summary>
typedef struct st_upload_texture
{
    std::vector<ID3D12Resource*> vector_texture;

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout{};
    UINT numRows = 0;
    UINT64 rowSizeInBytes = 0;
    UINT64 totalBytes = 0;

    UINT device_index = UINT_MAX;

}*pst_upload_texture;

/// <summary>
/// srv�� ������ �޸𸮿� ������ �� �ִ� �ڵ�
/// </summary>
typedef struct st_srv_handle
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> vector_handle_cpu;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> vector_handle_gpu;

    UINT device_index = UINT_MAX;

}*pst_srv_handle;

/// <summary>
/// 1���� �̵�� ��Ʈ���� �߻�ȭ�ϴ� ��ü
/// </summary>
typedef struct st_scene
{
    std::map<int, void*> map_ffmpeg_instance;
    int map_ffmpeg_instance_capacity = 3;
    int index_ffmpeg_instance_current = 0;
    int index_ffmpeg_instance_last = 0;
    int index_ffmpeg_instance_delete = 0;

    std::string url;
    RECT rect{ 0, 0, 0, 0 };
    NormalizedRect normal_rect{ 0.0f, 0.0f, 0.0f, 0.0f };

    UINT scene_index = UINT_MAX;
    UINT device_index = UINT_MAX;
    UINT output_index = UINT_MAX;

    bool flag_thread_scene = true;

    HANDLE event_scene_to_upload = nullptr;

    std::condition_variable* condition_variable_window_to_scene = nullptr;
    std::mutex* mutex_window_to_scene = nullptr;
    bool flag_window_to_scene = false;

    std::vector<AVFrame*> vector_frame;

    bool flag_ready_to_frame_use = false;

    bool flag_frame_unref = false;

    int index_input = 0;
    int index_output = 0;

    int frame_index = 0;

    std::deque<int> deque_index_used;
    std::mutex* mutex_deque_index_used = nullptr;

    // value : 3
    int count_used_frame_store = 3;

    bool flag_use_last_frame = false;

    bool flag_thread_scene_unref = true;

    std::mutex* mutex_deque_index_unref;
    std::deque<int> deque_index_unref;

    int64_t time_now = 0;
    int64_t time_last = 0;
    int64_t pts_in_milliseconds_now = 0;
    int64_t pts_in_milliseconds_last = 0;

}*pst_scene;

/// <summary>
/// 1���� ���ڸ� �߻�ȭ�ϴ� ��ü
/// </summary>
typedef struct st_text_internal
{
    UINT index_text_internal;

    std::wstring text_string;
    pst_color text_color;
    int* text_size;
    std::wstring text_font_family;
    pst_color text_color_background;
    float* text_start_coordinate_left;
    float* text_start_coordinate_top;
    float* text_background_width;
    float* text_background_height;
    e_dwrite_font_weight* text_weight;
    e_dwrite_font_style* text_style;
    e_dwrite_font_stretch* text_stretch;

    IDWriteTextFormat* text_format;
    ID2D1SolidColorBrush* text_brush;

    IDWriteTextLayout* text_layout;
    DWRITE_TEXT_METRICS* text_matrics;
    ID2D1SolidColorBrush* text_brush_background;

    e_movement_type_horizontal* movement_type_horizontal;
    float* movement_speed_horizontal;
    float* movement_threshold_horizontal;
    float* movement_translation_horizontal;

    e_movement_type_horizontal* movement_type_horizontal_background;
    float* movement_speed_horizontal_background;
    float* movement_threshold_horizontal_background;
    float* movement_translation_horizontal_background;

    e_movement_type_vertical* movement_type_vertical;
    float* movement_speed_vertical;
    float* movement_threshold_vertical;
    float* movement_translation_vertical;

    e_movement_type_vertical* movement_type_vertical_background;
    float* movement_speed_vertical_background;
    float* movement_threshold_vertical_background;
    float* movement_translation_vertical_background;

    bool flag_created;
    bool flag_deleted;

    bool flag_blink;

}*pst_text_internal;

/// <summary>
/// 1���� �׷��� ī�尡 1���� ������ ���� �߻�ȭ ��ü
/// </summary>
typedef struct st_text
{
    UINT index_device = UINT_MAX;

    std::map<UINT, pst_text_internal> map_text_internal;

    int interval_blink_in_miliseconds;
    int counter_interval_blink;

    int duration_blink_in_miliseconds;
    int counter_duration_blink;

    bool flag_blink_status;

}*pst_text;

typedef struct st_cef_device
{
    UINT index_cef_device = UINT_MAX;

    RECT rect_target{ 0, 0, 0, 0 };
    NormalizedRect rect_normalized{ 0.0f, 0.0f, 0.0f, 0.0f };

    int index_srv_upload;
    int index_srv_draw;

    bool flag_texture_created;

    int count_cef_device;

}*pst_cef_device;

typedef struct st_cef
{
    UINT index_cef;

    void* instance;

    std::string url;
    
    RECT rect_base{ 0, 0, 0, 0 };

    std::map<UINT, pst_cef_device> map_cef_device;

    HWND handle;

    int index_map_input;
    int index_map_output;

    std::map<UINT, pst_cef_data_paint>* map_cef_data_paint = nullptr;
    std::mutex* mutex_map_cef_data_paint = nullptr;

    std::deque<int>* deque_index_used = nullptr;
    std::mutex* mutex_deque_index_used = nullptr;

    std::thread* thread_cef_unref = nullptr;
    bool flag_thread_cef_unref = false;
    std::mutex* mutex_deque_index_unref = nullptr;
    std::deque<int>* deque_index_unref = nullptr;

    bool flag_use_last_frame = false;

    int count_used_frame_store = 3;

    int index_upload_texture = -1;

}*pst_cef;

#pragma endregion

// --------------------------------

// ���� ����:
HINSTANCE hInst;                                // ���� �ν��Ͻ��Դϴ�.
WCHAR szTitle[MAX_LOADSTRING];                  // ���� ǥ���� �ؽ�Ʈ�Դϴ�.
WCHAR szWindowClass[MAX_LOADSTRING];            // �⺻ â Ŭ���� �̸��Դϴ�.

/// <summary>
/// ���α׷� ������ �÷���
/// </summary>
bool _is_running = true;
/// <summary>
/// ���� ���α׷��� ���
/// </summary>
std::wstring _asset_path;

bool _flag_cef_succeed = false;

/// <summary>
/// ��������� ���� �߿� �߻��� ª�� ���ð�
/// </summary>
constexpr int _sleep_time_main_loop = 1;
/// <summary>
/// ��������� ���� �߿� �߻��� �� ���ð�
/// </summary>
constexpr int _sleep_time_processing = 10;
/// <summary>
/// backbuffer ���� ����
/// </summary>
constexpr UINT _frame_buffer_count = 3;
/// <summary>
/// rtv�� ������ �׷��� ī�� �޸��� �ִ� ũ�� ����
/// </summary>
constexpr UINT _rtv_descriptor_count = 4096;
/// <summary>
/// srv�� ������ �׷��� ī�� �޸��� �ִ� ũ�� ����
/// </summary>
constexpr UINT _srv_descriptor_count = 4096;
/// <summary>
/// NV12 ������ ����� �� 1���� �����ӿ� ����� �ؽ�ó ����
/// </summary>
constexpr int _texture_resource_count_nv12 = 2;
/// <summary>
/// command allocator ����
/// </summary>
constexpr int _count_command_allocator = 3;
/// <summary>
/// command list ����
/// </summary>
constexpr int _count_command_list = 3;

#pragma region Config Values

/// <summary>
/// Server IP
/// </summary>
std::string _ip;
/// <summary>
/// Server PORT
/// </summary>
uint16_t _port = UINT16_MAX;

/// <summary>
/// �α׸� �ۼ����� ���ϴ� �÷���
/// </summary>
bool _flag_set_logger = false;

/// <summary>
/// �α� ����. trace: 0, debug : 1, info : 2, warn : 3, err : 4, critical : 5, off : 6
/// </summary>
int _log_level = 6;

/// <summary>
/// 1�� �α� ���� ũ��. MB ����
/// </summary>
int _log_file_size = 1;

/// <summary>
/// �α� ���� ��ȯ ����.
/// </summary>
int _log_file_rotation_count = 3;

/// <summary>
/// WaitForMultipleObjects �� ����� ���ð�
/// </summary>
DWORD _wait_for_multiple_objects_wait_time = 1000;

/// <summary>
/// repeat �� �� ���� �������� ����ϴ� ���� Ƚ��
/// </summary>
int _count_use_last_frame_at_repeat = 30;

/// <summary>
/// background color red, 0 - 255
/// </summary>
int _background_color_r = 0;
/// <summary>
/// background color green, 0 - 255
/// </summary>
int _background_color_g = 0;
/// <summary>
/// background color blue, 0 - 255
/// </summary>
int _background_color_b = 0;

/// <summary>
/// background color red in float
/// </summary>
float _background_color_r_float = 0.0f;
/// <summary>
/// background color green in float
/// </summary>
float _background_color_g_float = 0.0f;
/// <summary>
/// background color blue in float
/// </summary>
float _background_color_b_float = 0.0f;

/// <summary>
/// �⺻ �̹��� url
/// </summary>
std::string _default_texture_url;

/// <summary>
/// nvapi ���
/// </summary>
bool _use_nvapi = false;

/// <summary>
/// nvapi�� ����� present���� ���
/// </summary>
bool _block_swap_group_present = false;

/// <summary>
/// texture ���� ũ��
/// </summary>
int _count_texture_store = 0;

/// <summary>
/// �ϵ���� ���ڵ� Ÿ��. 2: CUDA, 4: DXVA2, 7: D3D11VA, 12: D3D12VA
/// </summary>
int _hw_device_type = 12;

/// <summary>
/// Chromium Embedded Framework (CEF) ������� ���ϴ� �÷���
/// �̿ϼ��̴� ���� �׽�Ʈ �뵵 ����� ������� ����.
/// </summary>
bool _flag_use_cef = false;

/// <summary>
/// browser_subprocess_path �ɼǿ� ����� ��ġ.
/// cef �ҽ��� tests/cefclient �� �����Ͽ� ���� �� �ִ� cefclient.exe ������ ��ġ�� �����.
/// </summary>
std::string _cef_browser_subprocess_path;

/// <summary>
/// ��Ʈ�� ����� ��� �÷���
/// </summary>
bool _use_control_output = false;
/// <summary>
/// ��Ʈ�� ����� left
/// </summary>
int _control_output_left = 0;
/// <summary>
/// ��Ʈ�� ����� top
/// </summary>
int _control_output_top = 0;
/// <summary>
/// ��Ʈ�� ����� width
/// </summary>
int _control_output_width = 0;
/// <summary>
/// ��Ʈ�� ����� height
/// </summary>
int _control_output_height = 0;

/// <summary>
/// ������ ��ǥ ���� ���� �÷���
/// </summary>
bool _use_manual_window_create = false;
/// <summary>
/// ������ ��ǥ�� �����Ͽ� ������ ������ ����
/// </summary>
int _count_manual_window = 0;

/// <summary>
/// ������ ��ǥ ����. �� ���� ���ڸ� ������ ���� -1 ���� �����ؾ���
/// </summary>
std::map<UINT, int> _map_manual_window_rect_left;
/// <summary>
/// ������ ��ǥ ����. �� ���� ���ڸ� ������ ���� -1 ���� �����ؾ���
/// </summary>
std::map<UINT, int> _map_manual_window_rect_top;
/// <summary>
/// ������ ��ǥ ����. �� ���� ���ڸ� ������ ���� -1 ���� �����ؾ���
/// </summary>
std::map<UINT, int> _map_manual_window_rect_width;
/// <summary>
/// ������ ��ǥ ����. �� ���� ���ڸ� ������ ���� -1 ���� �����ؾ���
/// </summary>
std::map<UINT, int> _map_manual_window_rect_height;

/// <summary>
/// scene ���� ����
/// </summary>
int _count_scene = 0;
/// <summary>
/// scene url
/// </summary>
std::map<int, std::string> _map_scene_url;
/// <summary>
/// scene ǥ�� ��ǥ
/// </summary>
std::map<int, int> _map_scene_rect_left;
/// <summary>
/// scene ǥ�� ��ǥ
/// </summary>
std::map<int, int> _map_scene_rect_top;
/// <summary>
/// scene ǥ�� ��ǥ
/// </summary>
std::map<int, int> _map_scene_rect_width;
/// <summary>
/// scene ǥ�� ��ǥ
/// </summary>
std::map<int, int> _map_scene_rect_height;

#pragma endregion

/// <summary>
/// nvapi�� ����� �غ� �Ϸ� �÷���
/// </summary>
bool _nvapi_initialized = false;
/// <summary>
/// nvapi ��ȯ�� ���� ��ü
/// </summary>
NvAPI_Status _nvapi_status = NVAPI_OK;
/// <summary>
/// 1 �׷��� ī�� �� 1���� swap group ������
/// </summary>
NvU32 _swap_group = 1;
/// <summary>
/// 1 �׷��� ī�� �� 1���� swap barrier ������
/// </summary>
NvU32 _swap_barrier = 1;

/// <summary>
/// ��� ��Ŷ ó�� ������
/// </summary>
std::thread _thread_packet_processing;
/// <summary>
/// ��� ��Ŷ ó�� ������ ������ �÷���
/// </summary>
bool _flag_packet_processing = true;
/// <summary>
/// ��� ��Ŷ ť ���ؽ�
/// </summary>
std::mutex _mutex_packet_processing;
/// <summary>
/// ��� ��Ŷ ť
/// </summary>
std::deque<std::pair<void*, void*>> _queue_packet_processing;

/// <summary>
/// ��� ���� ������
/// </summary>
std::thread _thread_tcp_server;
/// <summary>
/// ��� ���� ������ ������ �÷���
/// </summary>
bool _flag_tcp_server = true;
/// <summary>
/// ��� ���� ��ü
/// </summary>
void* _tcp_server = nullptr;

/// <summary>
/// ������ ���� ������ �����ϴ� ��ü ���� ������
/// </summary>
std::thread _thread_vector_input;
/// <summary>
/// ������ ���� ������ �����ϴ� ��ü ���� ������ ������ �÷���
/// </summary>
bool _flag_vector_input = true;
/// <summary>
/// ������ ���� ������ �����ϴ� ��ü�� �ӽ÷� �����ϴ� ����
/// </summary>
std::vector<st_input_object> _vector_input_object;
/// <summary>
/// ������ ���� ������ �����ϴ� ��ü�� �ӽ÷� �����ϴ� ������ ���ؽ�
/// </summary>
std::mutex _mutex_input_object;


/// <summary>
/// �̺�Ʈ ��ü ���� scene - upload
/// </summary>
std::vector<HANDLE> _vector_event_scene_to_upload;
/// <summary>
/// �̺�Ʈ ��ü ���� ���ؽ� scene - upload
/// </summary>
std::mutex _mutex_vector_event_scene_to_upload;
/// <summary>
/// ����� �踮��� ��ü ���� scene - upload
/// </summary>
std::vector<std::condition_variable*> _vector_condition_variable_scene_to_upload;
/// <summary>
/// ����� �踮��� ��ü ���� ���ؽ� scene - upload
/// </summary>
std::mutex _mutex_vector_condition_variable_scene_to_upload;
/// <summary>
/// ����� �踮��� ���ؽ� ��ü ���� scene - upload
/// </summary>
std::vector<std::mutex*> _vector_mutex_scene_to_upload;
/// <summary>
/// ����� �踮��� ���ؽ� ��ü ���� ���ؽ� scene - upload
/// </summary>
std::mutex _mutex_vector_mutex_scene_to_upload;
/// <summary>
/// ������ ������ �÷��� ��ü ���� scene - upload
/// </summary>
std::vector<bool*> _vector_flag_scene_to_upload;
/// <summary>
/// ������ ������ �÷��� ��ü ���� ���ؽ� scene - upload
/// </summary>
std::mutex _mutex_vector_flag_scene_to_upload;

/// <summary>
/// �̺�Ʈ ��ü ���� upload - device
/// </summary>
std::vector<HANDLE> _vector_event_upload_to_device;
/// <summary>
/// �̺�Ʈ ��ü ���� ���ؽ� upload - device
/// </summary>
std::mutex _mutex_vector_event_upload_to_device;
/// <summary>
/// ����� �踮��� ��ü ���� upload - device
/// </summary>
std::vector<std::condition_variable*> _vector_condition_variable_upload_to_device;
/// <summary>
/// ����� �踮��� ��ü ���� ���ؽ� upload - device
/// </summary>
std::mutex _mutex_vector_condition_variable_upload_to_device;
/// <summary>
/// ����� �踮��� ���ؽ� ��ü ���� upload - device
/// </summary>
std::vector<std::mutex*> _vector_mutex_upload_to_device;
/// <summary>
/// ����� �踮��� ���ؽ� ��ü ���� ���ؽ� upload - device
/// </summary>
std::mutex _mutex_vector_mutex_upload_to_device;
/// <summary>
/// ������ ������ �÷��� ��ü ���� upload - device
/// </summary>
std::vector<bool*> _vector_flag_upload_to_device;
/// <summary>
/// ������ ������ �÷��� ��ü ���� ���ؽ� upload - device
/// </summary>
std::mutex _mutex_vector_flag_upload_to_device;

/// <summary>
/// �̺�Ʈ ��ü ���� device - window
/// </summary>
std::vector<HANDLE> _vector_event_device_to_window;
/// <summary>
/// �̺�Ʈ ��ü ���� ���ؽ� device - window
/// </summary>
std::mutex _mutex_vector_event_device_to_window;
/// <summary>
/// ����� �踮��� ��ü ���� device - window
/// </summary>
std::vector<std::condition_variable*> _vector_condition_variable_device_to_window;
/// <summary>
/// ����� �踮��� ��ü ���� ���ؽ� device - window
/// </summary>
std::mutex _mutex_vector_condition_variable_device_to_window;
/// <summary>
/// ����� �踮��� ���ؽ� ��ü ���� device - window
/// </summary>
std::vector<std::mutex*> _vector_mutex_device_to_window;
/// <summary>
/// ����� �踮��� ���ؽ� ��ü ���� ���ؽ� device - window
/// </summary>
std::mutex _mutex_vector_mutex_device_to_window;
/// <summary>
/// ������ ������ �÷��� ��ü ���� device - window
/// </summary>
std::vector<bool*> _vector_flag_device_to_window;
/// <summary>
/// ������ ������ �÷��� ��ü ���� ���ؽ� device - window
/// </summary>
std::mutex _mutex_vector_flag_device_to_window;

/// <summary>
/// �̺�Ʈ ��ü ���� window - scene
/// </summary>
std::vector<HANDLE> _vector_event_window_to_scene;
/// <summary>
/// �̺�Ʈ ��ü ���� ���ؽ� window - scene
/// </summary>
std::mutex _mutex_vector_event_window_to_scene;
/// <summary>
/// ����� �踮��� ��ü ���� window - scene
/// </summary>
std::vector<std::condition_variable*> _vector_condition_variable_window_to_scene;
/// <summary>
/// ����� �踮��� ��ü ���� ���ؽ� window - scene
/// </summary>
std::mutex _mutex_vector_condition_variable_window_to_scene;
/// <summary>
/// ����� �踮��� ���ؽ� ��ü ���� window - scene
/// </summary>
std::vector<std::mutex*> _vector_mutex_window_to_scene;
/// <summary>
/// ����� �踮��� ���ؽ� ��ü ���� window - scene
/// </summary>
std::mutex _mutex_vector_mutex_window_to_scene;
/// <summary>
/// ������ ������ �÷��� ��ü ���� window - scene
/// </summary>
std::vector<bool*> _vector_flag_window_to_scene;
/// <summary>
/// ������ ������ �÷��� ��ü ���� ���ؽ� window - scene
/// </summary>
std::mutex _mutex_vector_flag_window_to_scene;

/// <summary>
/// wait for multiple objects ������ scene - upload
/// </summary>
std::thread _thread_wait_for_multiple_objects_scene_to_upload;
/// <summary>
/// wait for multiple objects ������ ������ �÷��� scene - upload
/// </summary>
bool _flag_wait_for_multiple_objects_scene_to_upload = true;

/// <summary>
/// wait for multiple objects ������ upload - device
/// </summary>
std::thread _thread_wait_for_multiple_objects_upload_to_device;
/// <summary>
/// wait for multiple objects ������ ������ �÷��� upload - device
/// </summary>
bool _flag_wait_for_multiple_objects_upload_to_device = true;

/// <summary>
/// wait for multiple objects ������ device - window
/// </summary>
std::thread _thread_wait_for_multiple_objects_device_to_window;
/// <summary>
/// wait for multiple objects ������ ������ �÷��� device - window
/// </summary>
bool _flag_wait_for_multiple_objects_device_to_window = true;

/// <summary>
/// wait for multiple objects ������ window - scene
/// </summary>
std::thread _thread_wait_for_multiple_objects_window_to_scene;
/// <summary>
/// wait for multiple objects ������ ������ �÷��� window - scene
/// </summary>
bool _flag_wait_for_multiple_objects_window_to_scene = true;

/// <summary>
/// scene ������ ��
/// </summary>
std::map<UINT, std::thread*> _map_thread_scene;
/// <summary>
/// scene �� �������� unref�ϴ� ������ ��
/// </summary>
std::map<UINT, std::thread*> _map_thread_scene_unref;
/// <summary>
/// upload ������ ��
/// </summary>
std::map<UINT, std::thread*> _map_thread_upload;
/// <summary>
/// device ������ ��
/// </summary>
std::map<UINT, std::thread*> _map_thread_device;
/// <summary>
/// window ������ ��
/// </summary>
std::map<UINT, std::thread*> _map_thread_window;

/// <summary>
/// scene ��
/// </summary>
std::map<UINT, pst_scene> _map_scene;

/// <summary>
/// ���� scene�� index
/// </summary>
UINT _next_scene_index = 0;

std::map<UINT, pst_cef> _map_cef;

std::deque<int> _deque_cef_delete;
std::mutex _mutex_deque_cef_delete;

// --------------------------------

/// <summary>
/// DirectX�� ���丮
/// </summary>
IDXGIFactory2* _factory = nullptr;

/// <summary>
/// adapter ��
/// </summary>
std::map<UINT, pst_adapter> _map_adapter;
/// <summary>
/// device ��
/// </summary>
std::map<UINT, pst_device> _map_device;
/// <summary>
/// output ��
/// </summary>
std::map<UINT, pst_output> _map_output;

/// <summary>
/// command queue ��
/// </summary>
std::map<UINT, pst_command_queue> _map_command_queue;
/// <summary>
/// command allocator ��
/// </summary>
std::map<UINT, pst_command_allocator> _map_command_allocator;
/// <summary>
/// command list ��
/// </summary>
std::map<UINT, pst_command_list> _map_command_list;
/// <summary>
/// root signature ��
/// </summary>
std::map<UINT, pst_root_signature> _map_root_sig;
/// <summary>
/// pipeline state object ��
/// </summary>
std::map<UINT, pst_pso> _map_pso;
/// <summary>
/// fence ��
/// </summary>
std::map<UINT, pst_fence> _map_fence;

/// <summary>
/// render target view heap ��
/// </summary>
std::map<UINT, pst_rtv_heap> _map_rtv_heap;
/// <summary>
/// render target view ��
/// </summary>
std::map<UINT, pst_rtv> _map_rtv;

/// <summary>
/// shader resource view heap ��
/// </summary>
std::map<UINT, pst_srv_heap> _map_srv_heap;

/// <summary>
/// vertex buffer ��
/// </summary>
std::map<UINT, pst_vertex_buffer> _map_vertex_buffer;
/// <summary>
/// vertex buffer �� ���ؽ�
/// </summary>
std::mutex* _mutex_map_vertex_buffer = nullptr;
/// <summary>
/// vertex upload buffer ��
/// </summary>
std::map<UINT, pst_vertex_upload_buffer> _map_vertex_upload_buffer;
/// <summary>
/// vertex upload buffer �� ���ؽ�
/// </summary>
std::mutex* _mutex_map_vertex_upload_buffer = nullptr;
/// <summary>
/// vertex buffer view ��
/// </summary>
std::map<UINT, pst_vertex_buffer_view> _map_vertex_buffer_view;
/// <summary>
/// vertex buffer view �� ���ؽ�
/// </summary>
std::mutex* _mutex_map_vertex_buffer_view = nullptr;

/// <summary>
/// index buffer ��
/// </summary>
std::map<UINT, pst_index_buffer> _map_index_buffer;
/// <summary>
/// index buffer �� ���ؽ�
/// </summary>
std::mutex* _mutex_map_index_buffer = nullptr;
/// <summary>
/// index upload buffer ��
/// </summary>
std::map<UINT, pst_index_upload_buffer> _map_index_upload_buffer;
/// <summary>
/// index upload buffer �� ���ؽ�
/// </summary>
std::mutex* _mutex_map_index_upload_buffer = nullptr;
/// <summary>
/// index buffer view ��
/// </summary>
std::map<UINT, pst_index_buffer_view> _map_index_buffer_view;
/// <summary>
/// index buffer view �� ���ؽ�
/// </summary>
std::mutex* _mutex_map_index_buffer_view = nullptr;

/// <summary>
/// shader resource view handle Y ä�� ��
/// </summary>
std::map<UINT, pst_srv_handle> _map_srv_handle_luminance;
/// <summary>
/// shader resource view handle Y ä�� �� ���ؽ�
/// </summary>
std::mutex* _mutex_map_srv_handle_luminance = nullptr;
/// <summary>
/// shader resource view handle UV ä�� ��
/// </summary>
std::map<UINT, pst_srv_handle> _map_srv_handle_chrominance;
/// <summary>
/// shader resource view handle UV ä�� �� ���ؽ�
/// </summary>
std::mutex* _mutex_map_srv_handle_chrominance = nullptr;

/// <summary>
/// window ��
/// </summary>
std::map<UINT, pst_window> _map_window;
/// <summary>
/// swap chain ��
/// </summary>
std::map<UINT, pst_swap_chain> _map_swap_chain;
/// <summary>
/// viewport ��
/// </summary>
std::map<UINT, pst_viewport> _map_viewport;

/// <summary>
/// �⺻ �̹����� ���ڵ��� ffmpeg �ν��Ͻ�
/// </summary>
void* _ffmpeg_instance_default_image = nullptr;
/// <summary>
/// �⺻ �̹��� ������
/// </summary>
AVFrame* _frame_default_image = nullptr;

/// <summary>
/// �⺻ �̹��� ��� �÷���
/// </summary>
bool _flag_use_default_image = false;

/// <summary>
/// ���ڸ� ����ϱ� ���� D2D1 ���丮
/// </summary>
ID2D1Factory7* _factory_2d = nullptr;
/// <summary>
/// ���ڸ� ����ϱ� ���� Direct Write ���丮
/// </summary>
IDWriteFactory* _factory_dwrite = nullptr;

/// <summary>
/// ���� ��
/// </summary>
std::map<UINT, pst_text> _map_text;

// --------------------------------

// �� �ڵ� ��⿡ ���Ե� �Լ��� ������ �����մϴ�:
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

/// <summary>
/// ���丮 ����
/// </summary>
void create_factory();
/// <summary>
/// ����� ����
/// </summary>
void enum_adapters();
/// <summary>
/// �����ġ ����
/// </summary>
void enum_outputs();
/// <summary>
/// window ����
/// </summary>
/// <param name="window_class"></param>
/// <param name="title"></param>
/// <param name="instance"></param>
/// <param name="rect"></param>
/// <param name="data"></param>
/// <param name="handle"> ������ window�� �ڵ� </param>
void create_window(WCHAR* window_class, WCHAR* title, HINSTANCE instance, RECT rect, void* data, HWND& handle);
/// <summary>
/// window �� ����,
/// ������� ����
/// </summary>
void create_windows();
/// <summary>
/// window���� ini�� ������ rect�� ����
/// </summary>
void create_windows_manual();
/// <summary>
/// window�� �������� ���� ����͸� �ʿ��� ����
/// </summary>
void delete_adapter_has_none_window();
/// <summary>
/// ����̽� ����
/// </summary>
void create_devices();
/// <summary>
/// Ŀ�ǵ� ť ����
/// </summary>
void create_command_queues();
/// <summary>
/// Ŀ�ǵ� �Ҵ��� ����
/// </summary>
void create_command_allocators();
/// <summary>
/// Ŀ�ǵ� ����Ʈ ����
/// </summary>
void create_command_lists();
/// <summary>
/// ���� Ÿ�� �� �� ����
/// </summary>
void create_rtv_heaps();
/// <summary>
/// ���̴� ���ҽ� �� �� ����
/// </summary>
void create_srv_heaps();
/// <summary>
/// ��Ʈ �ñ״�ó ����.
/// ��Ʈ �ñ״�ó�� ���������� �ۼ��ڵ� ��,
/// ���� �ʿ���. �Ǵ� �׳� ����ϸ� ��.
/// </summary>
void create_root_sigs();
/// <summary>
/// ���������� ������Ʈ ������Ʈ ����.
/// pso�� �������� �ۼ��ڵ� �ڼ��� ���� ����.
/// ���� �ʿ���. �Ǵ� �׳� ����ϸ� ��.
/// </summary>
void create_pipeline_state_objects();
/// <summary>
/// �潺 ����
/// </summary>
void create_fences();
/// <summary>
/// ���� ü�� ����
/// </summary>
void create_swap_chains();
/// <summary>
/// ���� Ÿ�� �� ����
/// </summary>
void create_rtvs();
/// <summary>
/// ����Ʈ ����
/// </summary>
void create_viewports();
/// <summary>
/// 11 on 12 ����̽� ����
/// </summary>
void create_device_11_on_12();


/// <summary>
/// 11 on 12 ����̽� ������
/// </summary>
void delete_device_11_on_12();
/// <summary>
/// ����Ʈ ����
/// </summary>
void delete_viewports();
/// <summary>
/// ���� Ÿ�� �� ������
/// </summary>
void delete_rtvs();
/// <summary>
/// ���� ü�� ������
/// </summary>
void delete_swap_chains();
/// <summary>
/// ���̴� ���ҽ� �� �� ������
/// </summary>
void delete_srv_heaps();
/// <summary>
/// ���� Ÿ�� �� �� ������
/// </summary>
void delete_rtv_heaps();
/// <summary>
/// �潺 ������
/// </summary>
void delete_fences();
/// <summary>
/// Ŀ�ǵ� ����Ʈ ������
/// </summary>
void delete_command_lists();
/// <summary>
/// ���������� ������Ʈ ������Ʈ ������
/// </summary>
void delete_pipeline_state_objects();
/// <summary>
/// ��Ʈ �ñ״�ó ������
/// </summary>
void delete_root_sigs();
/// <summary>
/// Ŀ�ǵ� �Ҵ��� ������
/// </summary>
void delete_command_allocators();
/// <summary>
/// Ŀ�ǵ� ť ������
/// </summary>
void delete_command_queues();
/// <summary>
/// ����̽� ������
/// </summary>
void delete_devices();
/// <summary>
/// window ����
/// </summary>
void delete_windows();
/// <summary>
/// �����ġ ������
/// </summary>
void delete_outputs();
/// <summary>
/// ����� ������
/// </summary>
void delete_adapters();
/// <summary>
/// ���丮 ������
/// </summary>
void delete_factory();

/// <summary>
/// ���ؽ� ���� ����
/// </summary>
/// <param name="data_device"> ���ؽ� ���۸� ������ ����̽� </param>
/// <param name="index_command_list"> ��� ����̽����� ������ Ŀ�ǵ� ����Ʈ �ε��� </param>
void create_vertex_buffer(pst_device data_device, int index_command_list);
void create_vertex_buffer_cef(pst_device data_device, int index_command_list, int counter_scene, NormalizedRect rect_normalized);
/// <summary>
/// ���ؽ� ���� ������
/// </summary>
void delete_vertex_buffers();
/// <summary>
/// �ε��� ���� ����
/// </summary>
/// <param name="data_device"> �ε��� ���۸� ������ ����̽� </param>
/// <param name="index_command_list"> ��� ����̽����� ������ Ŀ�ǵ� ����Ʈ �ε��� </param>
void create_index_buffer(pst_device data_device, int index_command_list);
/// <summary>
/// �ε��� ���� ������
/// </summary>
void delete_index_buffers();
/// <summary>
/// ���̴� ���ҽ� �� �ڵ�(�ؽ�ó) ����
/// </summary>
/// <param name="data_device"> srv handle�� ������ ����̽� </param>
/// <param name="counter_texture"> srv �ڵ��� ��ȣ�� ����� �ؽ�ó ī��Ʈ </param>
void create_srv_handles(pst_device data_device, int counter_texture);
/// <summary>
/// ���̴� ���ҽ� �� �ڵ�(�ؽ�ó) ����
/// </summary>
/// <param name="data_device"> srv handle�� ������ ����̽� </param>
/// <param name="counter_texture"> �̹� �����Ǿ��ִ� srv ���� </param>
/// <param name="counter_cef"> �̹� �����Ǿ��ִ� cef�� srv ���� </param>
void create_srv_handles_cef_paint(pst_device data_device, int counter_texture, int counter_cef);
/// <summary>
/// �⺻ �̹����� ���̴� ���ҽ� �� �ڵ�(�ؽ�ó) ����
/// </summary>
/// <param name="data_device"> srv handle�� ������ ����̽� </param>
void create_srv_handles_texture_default(pst_device data_device);
/// <summary>
/// �ؽ�ó ������
/// </summary>
void delete_textures();

/// <summary>
/// �ؽ�ó�� srv�� ����
/// </summary>
/// <param name="data_device"> srv�� ������ ����̽� </param>
/// <param name="frame"> ���ڵ��� ������ </param>
/// <param name="counter_texture"> srv �ڵ��� ��ȣ�� ����� �ؽ�ó ī��Ʈ </param>
/// <param name="srv_index"> �ؽ�ó�� ��ȣ </param>
void upload_texture(pst_device data_device, AVFrame* frame, int counter_texture, int srv_index);


/// <summary>
/// nvapi�� ����� ����ȭ ��ü ����
/// </summary>
/// <param name="device"></param>
/// <param name="swap_chain"></param>
void initialize_swap_lock(ID3D12Device* device, IDXGISwapChain1* swap_chain);
/// <summary>
/// nvapi�� ����� ����ȭ ��ü ����
/// </summary>
void initialize_swap_locks();
/// <summary>
/// nvapi�� ����� ����ȭ ��ü ������
/// </summary>
/// <param name="device"></param>
/// <param name="swap_chain"></param>
void delete_swap_lock(ID3D12Device* device, IDXGISwapChain1* swap_chain);
/// <summary>
/// nvapi�� ����� ����ȭ ��ü ������
/// </summary>
void delete_swap_locks();


/// <summary>
/// scene ����
/// </summary>
/// <returns> 0: ����, 1: ��Ʈ�� ���� ���� </returns>
int create_scenes();
/// <summary>
/// scene ����
/// </summary>
void delete_scenes();

/// <summary>
/// ffmpeg �ν��Ͻ��� �ݹ� �Լ�
/// </summary>
/// <param name="param"></param>
void callback_ffmpeg_wrapper_ptr(void* param);

#if _DEBUG
/// <summary>
/// d3d ��ü���� ������ ���� Ȯ��
/// </summary>
void d3d_memory_check();
#endif

/// <summary>
/// ���� ���α׷� ��� ��������
/// </summary>
/// <param name="path"></param>
/// <param name="path_size"></param>
void get_asset_path(wchar_t* path, uint32_t path_size);
/// <summary>
/// ���� ���α׷� ��ο� param�� �ٿ��� ������
/// </summary>
/// <param name="asset_name"> ���� ��ο� �߰��� ���� ��� </param>
/// <returns></returns>
std::wstring get_asset_full_path(LPCWSTR asset_name);
/// <summary>
/// ini ���� �б�
/// </summary>
void config_setting();


/// <summary>
/// ��� ���� ������ �Լ�
/// </summary>
void thread_tcp_server();

/// <summary>
/// ��� ��Ŷ ó�� ������ �Լ�
/// </summary>
void thread_packet_processing();

/// <summary>
/// ��� ���� ��ü�� �ݹ� �Լ�
/// </summary>
/// <param name="data"></param>
/// <param name="connection"></param>
void callback_data_connection_server(void* data, void* connection);


/// <summary>
/// ������ ���� ������ �����ϴ� ��ü ���� ������ �Լ�
/// </summary>
void thread_vector_input();

/// <summary>
/// ������ ���� ������ �����ϴ� ��ü�� �ӽ÷� �����ϴ� ���Ϳ� ��ü input
/// </summary>
/// <param name="data"></param>
void vector_input(st_input_object data);

/// <summary>
/// ��� ������ �Լ�
/// </summary>
/// <param name="wait_type"> ��� ���� ���� </param>
/// <param name="flag_thread"> wait for multiple objects ������ ������ �÷��� </param>
void thread_wait_for_multiple_objects(e_wait_type wait_type, bool* flag_thread);

/// <summary>
/// device ������ �Լ�
/// </summary>
/// <param name="data_device"></param>
void thread_device(pst_device data_device);
/// <summary>
/// upload ������ �Լ�
/// </summary>
/// <param name="data_device"></param>
void thread_upload(pst_device data_device);
/// <summary>
/// window ������ �Լ�
/// </summary>
/// <param name="data_window"></param>
void thread_window(pst_window data_window);
/// <summary>
/// scene ������ �Լ�
/// </summary>
/// <param name="data_scene"></param>
void thread_scene(pst_scene data_scene);

/// <summary>
/// scene �� ������ unref ������ �Լ�
/// </summary>
/// <param name="data_scene"></param>
void thread_scene_unref(pst_scene data_scene);

/// <summary>
/// ���� �� gpu �۾� �Ϸ� ��� �Լ�
/// </summary>
void wait_gpus_end();

/// <summary>
/// min, max ������ �� target�� normalized_min, normalized_max ���̷� ����ȭ
/// </summary>
/// <param name="min"> ���� min </param>
/// <param name="max"> ���� max </param>
/// <param name="target"> ���� min, max ������ �� </param>
/// <param name="normalized_min"> ����ȭ �� min </param>
/// <param name="normalized_max"> ����ȭ �� max </param>
/// <returns> ����ȭ�� target �� ��</returns>
float normalize_min_max(int min, int max, int target, int normalized_min, int normalized_max);
/// <summary>
/// int �� rect�� float �� NormalizedRect �� ����ȭ
/// </summary>
/// <param name="base_rect"> min, max ������ �Ǵ� rect </param>
/// <param name="target_rect"> base_rect �� ��ġ�� �κ��� �ִ� rect </param>
/// <param name="normalized_rect"> ����ȭ ��� </param>
void normalize_rect(RECT base_rect, RECT target_rect, NormalizedRect& normalized_rect);

/// <summary>
/// �̵�� ��Ʈ�� ��� ���� �Լ�
/// </summary>
/// <returns></returns>
int start_playback();

/// <summary>
/// index�� queue�� ����á���� Ȯ���ϴ� �Լ�
/// </summary>
/// <param name="index_input"> ��ǲ �ε��� </param>
/// <param name="index_output"> �ƿ�ǲ �ε��� </param>
/// <param name="queue_size"> ť�� ũ�� </param>
/// <returns> true: ť�� ���� ��, false: ť�� ���� ���� ���� </returns>
bool is_queue_full(int index_input, int index_output, int queue_size);
/// <summary>
/// index�� queue�� ������� Ȯ���ϴ� �Լ�
/// </summary>
/// <param name="index_input"> ��ǲ �ε��� </param>
/// <param name="index_output"> �ƿ�ǲ �ε��� </param>
/// <returns> true: ť�� �����, false: ť�� ������� ���� </returns>
bool is_queue_empty(int index_input, int index_output);
/// <summary>
/// index�� queue�� �� ���� �� �ִ� ũ�⸦ Ȯ���ϴ� �Լ�
/// </summary>
/// <param name="index_input"> ��ǲ �ε��� </param>
/// <param name="index_output"> �ƿ�ǲ �ε��� </param>
/// <param name="queue_size"> ť�� ũ�� </param>
/// <returns> ���� ������ ũ�� </returns>
int get_queue_size(int index_input, int index_output, int queue_size);

/// <summary>
/// ffmpeg�� ����ϴ� instance�� �����ϴ� �Լ�. create_ffmpeg_instance_play_start()�� ���� ����Ŀ� ���� �� �ִ� index_scene�� �� �Լ��� scene_index�� �����.
/// </summary>
/// <param name="instance"> ������ ffmpeg ��� instance�� ���� ������ </param>
/// <param name="device_index"> �ϵ���� ���ڵ��� ����� Device�� index </param>
/// <param name="url"> ���ῡ ����� ��Ʈ���� url </param>
/// <param name="scene_index"> ������ pst_scene�� �Ҵ��� scene_index �� </param>
/// <param name="rect"> instance�� ǥ��Ǿ���� �簢�� ��ġ </param>
/// <returns> 0: ����, 1: ���� </returns>
int create_ffmpeg_instance_with_scene_index(void*& instance, UINT device_index, std::string url, UINT scene_index, RECT rect);

/// <summary>
/// instance�� ����� ó�� ���� ��ġ�� ������ �Լ�
/// </summary>
/// <param name="data_scene"> �ݺ� ��� ��� instance�� ���� pst_scene ��ü </param>
/// <returns> �׻� 0 return </returns>
int check_map_ffmpeg_instance_repeat(pst_scene data_scene);

/// <summary>
/// ffmpeg�� ����ϴ� instance���� ��Ʈ�� ���� �õ� �Լ�
/// </summary>
/// <param name="instance"> ffmpeg ��� instance�� ������ </param>
/// <returns> 0: ����, 1: ���� </returns>
int create_ffmpeg_instance_check_open_file(void* instance);
/// <summary>
/// ffmpeg�� ����ϴ� instance���� playback ���� �Լ�
/// </summary>
/// <param name="instance"> ffmpeg ��� instance�� ������ </param>
/// <param name="index_scene"> ������ pst_scene�� �Ҵ��� index_scene�� ���� ���� ref </param>
/// <param name="rect"> instance�� ǥ��Ǿ���� �簢�� ��ġ </param>
/// <returns> �׻� 0 return </returns>
int create_ffmpeg_instance_play_start(void* instance, UINT& index_scene, RECT rect);

/// <summary>
/// default�� ����� texture ���� �Լ�
/// </summary>
/// <param name="data_device"> texture�� �����ϰ� ������ device ��ü </param>
/// <returns> �׻� 0 return </returns>
int create_texture_default(pst_device data_device);
/// <summary>
/// default�� ����ϴ� texture ���� �Լ�
/// </summary>
/// <param name="data_device"> default texture�� ���� device ��ü </param>
/// <returns> �׻� 0 return </returns>
int delete_texture_default(pst_device data_device);
/// <summary>
/// default�� ����ϴ� texture�� data�� upload�ϴ� �Լ�
/// </summary>
/// <param name="data_device"> default texture�� ���� device ��ü </param>
/// <param name="index_command_list"> GPU ����� �ۼ��� command_list�� index �� </param>
/// <returns> �׻� 0 return </returns>
int upload_texture_default(pst_device data_device, int index_command_list);

int create_nv12_texture(pst_device data_device, int width, int height, int index_cef);
int delete_nv12_texture(pst_device data_device);
int upload_texture_cef(pst_device data_device, pst_cef data_cef, int index_command_list_upload, int counter_texture);

/// <summary>
/// default image�� ���ڵ��� ffmpeg�� ����ϴ� instance ���� �Լ�
/// </summary>
/// <param name="instance"> default image�� ����� ffmpeg�� ����ϴ� instance�� ������ </param>
/// <returns> 0: ����, 1: ���� </returns>
int create_ffmpeg_instance_default_image(void*& instance);

/// <summary>
/// _map_scene�� ���� ������ ���� �Լ�. create_scenes()�� ������ �� �����.
/// </summary>
void delete_scenes_data();

/// <summary>
/// ffmpeg�� ����ϴ� instance�� �����ϰ� �����͵��� �����ϴ� �Լ�.
/// </summary>
/// <param name="instance"> ������ ffmpeg ��� instance�� ���� ������ </param>
/// <param name="index_device"> �ϵ���� ���ڵ��� ����� Device�� index </param>
/// <param name="url"> ���ῡ ����� ��Ʈ���� url </param>
/// <returns> �׻� 0 return </returns>
int create_ffmpeg_instance_set_data(void*& instance, UINT index_device, std::string url);

/// <summary>
/// ���� ��Ʈ���� live stream�� �� ��Ʈ�� ���� �õ� �Լ�
/// </summary>
/// <param name="instance"> ffmpeg�� ����ϴ� instance ������ </param>
/// <returns> 0: ����, 1: ����</returns>
int create_ffmpeg_instance_check_open_file_on_live_stream(void* instance);
/// <summary>
/// ffmpeg�� ����ϴ� instance�� scene_index�� rect�� �����ϴ� �Լ�
/// </summary>
/// <param name="instance"> ffmpeg�� ����ϴ� instance ������ </param>
/// <param name="index_scene"> ������ pst_scene�� �Ҵ��� index_scene�� ���� ���� ref </param>
/// <param name="rect"> instance�� ǥ��Ǿ���� �簢�� ��ġ </param>
/// <returns> �׻� 0 return </returns>
int create_ffmpeg_instance_play_start_set_data(void* instance, UINT& index_scene, RECT rect);
/// <summary>
/// ���� ����� live stream�� �� play�� �����ϴ� �Լ�
/// </summary>
/// <param name="instance"> ffmpeg�� ����ϴ� instance ������ </param>
/// <returns> �׻� 0 return </returns>
int create_ffmpeg_instance_play_start_on_live_stream(void* instance);

/// <summary>
/// Device ���� �ڸ� ��ü ���� �Լ�
/// </summary>
void create_texts();
/// <summary>
/// Device ���� ������ �ڸ� ��ü ���� �Լ�
/// </summary>
void delete_texts();

/// <summary>
/// internal �ڸ� ��ü ���� �Լ�
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
void create_text(int index_text);
/// <summary>
/// internal �ڸ� ��ü ���� �Լ�
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
void delete_text(int index_text);

/// <summary>
/// �ڸ��� �ؽ�Ʈ�� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="text_string"> ������ �ؽ�Ʈ </param>
void set_text_string(int index_text, std::wstring text_string);
/// <summary>
/// �ڸ��� �ؽ�Ʈ�� ���� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="color_r"> �ڸ����� R �� </param>
/// <param name="color_g"> �ڸ����� G �� </param>
/// <param name="color_b"> �ڸ����� B �� </param>
/// <param name="color_a"> �ڸ����� A �� </param>
void set_text_color(int index_text, int color_r, int color_g, int color_b, int color_a);
/// <summary>
/// �ڸ��� �ؽ�Ʈ�� ũ�⸦ ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="font_size"> �ؽ�Ʈ�� ũ�� </param>
void set_text_size(int index_text, int font_size);
/// <summary>
/// �ڸ��� �۲��� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="font_family"> �ؽ�Ʈ�� �۲� </param>
void set_text_font_family(int index_text, std::wstring font_family);
/// <summary>
/// �ڸ��� ������ ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="color_r"> �ڸ��� ������ R �� </param>
/// <param name="color_g"> �ڸ��� ������ G �� </param>
/// <param name="color_b"> �ڸ��� ������ B �� </param>
/// <param name="color_a"> �ڸ��� ������ A �� </param>
void set_text_color_background(int index_text, int color_r, int color_g, int color_b, int color_a);
/// <summary>
/// �ڸ��� ���� ��ġ�� left �� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="left"> �ڸ��� ���� ��ġ�� left �� </param>
void set_text_start_coordinate_left(int index_text, int left);
/// <summary>
/// �ڸ��� ���� ��ġ�� top �� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="top"> �ڸ��� ���� ��ġ�� top �� </param>
void set_text_start_coordinate_top(int index_text, int top);
/// <summary>
/// �ڸ��� ����� �ʺ� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="width"> �ڸ��� ����� �ʺ� �� </param>
void set_text_background_width(int index_text, int width);
/// <summary>
/// �ڸ��� ����� ���̸� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="height"> �ڸ��� ����� ���� �� </param>
void set_text_background_height(int index_text, int height);
/// <summary>
/// �ڸ��� ���⸦ ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="weight"> �ڸ��� ���� ���� </param>
void set_text_weight(int index_text, e_dwrite_font_weight weight);
/// <summary>
/// �ڸ��� style ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="style"> �ڸ��� style �� </param>
void set_text_style(int index_text, e_dwrite_font_style style);
/// <summary>
/// �ڸ��� stretch ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="stretch"> �ڸ��� stretch �� </param>
void set_text_stretch(int index_text, e_dwrite_font_stretch stretch);
/// <summary>
/// �ڸ� ������ ��� ���ο��� ������ �̵� type ��/�� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="type"> �ڸ��� ������ �̵� type ��/�� </param>
void set_text_movement_type_horizontal(int index_text, e_movement_type_horizontal type);
/// <summary>
/// �ڸ� ������ ��� ���ο��� ������ �̵� �ӵ� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="speed"> �ڸ� �̵� �ӵ� </param>
void set_text_movement_speed_horizontal(int index_text, int speed);
/// <summary>
/// �ڸ� ������ ��� ���ο��� ������ �ִ� �̵� ��ġ ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="threshold"> �ڸ� ��� ���ο����� ������ �ִ� �̵� ��ġ </param>
void set_text_movement_threshold_horizontal(int index_text, int threshold);
/// <summary>
/// �ڸ� ����� ������ �̵� type ��/�� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="type"> �ڸ� ����� ������ �̵� type ��/�� </param>
void set_text_movement_type_horizontal_background(int index_text, e_movement_type_horizontal type);
/// <summary>
/// �ڸ� ����� ������ �̵� �ӵ� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="speed"> �ڸ� ����� ������ �̵� �ӵ� </param>
void set_text_movement_speed_horizontal_background(int index_text, int speed);
/// <summary>
/// �ڸ� ����� ������ �ִ� �̵� ��ġ ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="threshold"> �ڸ� ����� ������ �ִ� �̵� ��ġ </param>
void set_text_movement_threshold_horizontal_background(int index_text, int threshold);
/// <summary>
/// �ڸ� ������ ��� ���ο��� ������ �̵� type ��/�� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="type"> �ڸ��� ������ �̵� type ��/�� </param>
void set_text_movement_type_vertical(int index_text, e_movement_type_vertical type);
/// <summary>
/// �ڸ� ������ ��� ���ο��� ������ �̵� �ӵ� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="speed"> �ڸ� �̵� �ӵ� </param>
void set_text_movement_speed_vertical(int index_text, int speed);
/// <summary>
/// �ڸ� ������ ��� ���ο��� ������ �ִ� �̵� ��ġ ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="threshold"> �ڸ� ��� ���ο����� ������ �ִ� �̵� ��ġ </param>
void set_text_movement_threshold_vertical(int index_text, int threshold);
/// <summary>
/// �ڸ� ����� ������ �̵� type ��/�� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="type"> �ڸ� ����� ������ �̵� type ��/�� </param>
void set_text_movement_type_vertical_background(int index_text, e_movement_type_vertical type);
/// <summary>
/// �ڸ� ����� ������ �̵� �ӵ� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="speed"> �ڸ� ����� ������ �̵� �ӵ� </param>
void set_text_movement_speed_vertical_background(int index_text, int speed);
/// <summary>
/// �ڸ� ����� ������ �ִ� �̵� ��ġ ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="threshold"> �ڸ� ����� ������ �ִ� �̵� ��ġ </param>
void set_text_movement_threshold_vertical_background(int index_text, int threshold);

/// <summary>
/// internal �ڸ� ������ ����ؼ� Direct Write�� �ڸ� instance�� ������
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="data_text"> �ڸ� instance�� ������ Device ������ ������ pst_text ��ü </param>
void create_text_instance(int index_text, pst_text data_text);
/// <summary>
/// Direct Write�� ���� �ڸ� instance�� ������
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="data_text"> �ڸ� instance�� ������ Device ������ ������ pst_text ��ü </param>
void delete_text_instance(int index_text, pst_text data_text);

/// <summary>
/// �ڸ� ������ ������ ȿ�� ����
/// </summary>
/// <param name="index_text"> ������ �ڸ��� index </param>
/// <param name="flag_blink_turn_on_off"> �ڸ� ������ ������ ȿ��, true: ȿ�� on, false: ȿ�� off </param>
void set_text_blink_turn_on_off(int index_text, bool flag_blink_turn_on_off);
/// <summary>
/// �ڸ� ������ ������ ȿ���� ���� ��, ���ڰ� �������� �ð��� milisecond�� ����
/// </summary>
/// <param name="interval_blink_in_miliseconds"> ���ڰ� �������� milisecond �ð� �� </param>
void set_text_blink_interval(int interval_blink_in_miliseconds);
/// <summary>
/// �ڸ� ������ ������ ȿ���� ���� ��, ���ڰ� �������� �ʴ� �ð��� milisecond�� ����
/// </summary>
/// <param name="duration_blink_in_miliseconds"> ���ڰ� �������� �ʴ� milisecond �ð� �� </param>
void set_text_blink_duration(int duration_blink_in_miliseconds);

void cef_initialize();
void cef_shutdown();

void create_cef(UINT index_cef, HWND handle, std::string url, RECT rect);
void delete_cef(UINT index_cef);

void delete_cefs();

void delete_cef_internal(pst_cef data_cef);

void thread_cef_unref(pst_cef data_cef);

// --------------------------------

/// <summary>
/// spdlog�� ����� �ΰ� ����
/// </summary>
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
            // GPU ��ȿ�� �˻� ��� ����
            // GPU�� ó���ӵ��� ������
            debug_controller->SetEnableGPUBasedValidation(true);
#endif // 0

            // Enable additional debug layers.
            dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&_factory));

    D2D1_FACTORY_OPTIONS d2d_factory_options = {};
#ifdef _DEBUG
    d2d_factory_options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif //_DEBUG
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory7), &d2d_factory_options, (void**)(&_factory_2d));

    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)(&_factory_dwrite));
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

    for (UINT i = 0; ; i++)
    {
        hr = _factory->EnumAdapters1(i, &adapter);
        if (hr != S_OK)
        {
            break;
        }

        adapter->GetDesc1(&adapter_desc);

        // adapter�� software adapter ��
        if (adapter_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            adapter->Release();
            continue;
        }

        // Device�� Direct3D Feature Level 12�� �����ϴ��� Ȯ����
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

    UINT output_index = 0;

    for (auto it_adapter = _map_adapter.begin(); it_adapter != _map_adapter.end();)
    {
        pst_adapter data_adapter = it_adapter->second;

        uint64_t output_count = 0;

        for (UINT i = 0; ; i++)
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

        // adapter�� ����� output�� ������ _map_adapter���� adapter�� ������
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
    for (auto it_adapter = _map_adapter.begin(); it_adapter != _map_adapter.end();)
    {
        pst_adapter data_adapter = it_adapter->second;

        RECT rect{ 0, 0, 0, 0 };

        if (_use_control_output)
        {

        }
        else
        {

        }
    }
}

void create_windows_manual()
{
    for (UINT i = 0; i < _count_manual_window; i++)
    {
        HWND handle = nullptr;

        RECT rect{ 0, 0, 0, 0 };

        rect.left = _map_manual_window_rect_left.find(i)->second;
        rect.top = _map_manual_window_rect_top.find(i)->second;
        rect.right = _map_manual_window_rect_left.find(i)->second + _map_manual_window_rect_width.find(i)->second;
        rect.bottom = _map_manual_window_rect_top.find(i)->second + _map_manual_window_rect_height.find(i)->second;

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
            event_window_to_scene.object_type = e_object_type::object_event;
            event_window_to_scene.wait_type = e_wait_type::window_to_scene;

            vector_input(event_window_to_scene);


            st_input_object condition_variable_device_to_window{};
            condition_variable_device_to_window.object = data_window->condition_variable_device_to_window;
            condition_variable_device_to_window.object_type = e_object_type::object_condition_variable;
            condition_variable_device_to_window.wait_type = e_wait_type::device_to_window;

            st_input_object mutex_device_to_window{};
            mutex_device_to_window.object = data_window->mutex_device_to_window;
            mutex_device_to_window.object_type = e_object_type::object_mutex;
            mutex_device_to_window.wait_type = e_wait_type::device_to_window;

            st_input_object flag_device_to_window{};
            flag_device_to_window.object = &data_window->flag_device_to_window;
            flag_device_to_window.object_type = e_object_type::object_flag;
            flag_device_to_window.wait_type = e_wait_type::device_to_window;

            vector_input(condition_variable_device_to_window);
            vector_input(mutex_device_to_window);
            vector_input(flag_device_to_window);
            

            for (auto it_output = _map_output.begin(); it_output != _map_output.end(); it_output++)
            {
                pst_output data_output = it_output->second;

                if (!(data_output->output_desc.DesktopCoordinates.left <= rect.left &&
                    data_output->output_desc.DesktopCoordinates.top <= rect.top &&
                    data_output->output_desc.DesktopCoordinates.right > rect.left &&
                    data_output->output_desc.DesktopCoordinates.bottom > rect.top))
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

        //hr = D3D12CreateDevice(data_adapter->adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
        // Direct Write�� ����ϱ� ���ؼ� Device�� Direct3D Feature Level 11 �� ������
        hr = D3D12CreateDevice(data_adapter->adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

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
            event_device_to_window.object_type = e_object_type::object_event;
            event_device_to_window.wait_type = e_wait_type::device_to_window;

            vector_input(event_device_to_window);


            st_input_object condition_variable_upload_to_device{};
            condition_variable_upload_to_device.object = data_device->condition_variable_upload_to_device;
            condition_variable_upload_to_device.object_type = e_object_type::object_condition_variable;
            condition_variable_upload_to_device.wait_type = e_wait_type::upload_to_device;

            st_input_object mutex_upload_to_device{};
            mutex_upload_to_device.object = data_device->mutex_upload_to_device;
            mutex_upload_to_device.object_type = e_object_type::object_mutex;
            mutex_upload_to_device.wait_type = e_wait_type::upload_to_device;

            st_input_object flag_upload_to_device{};
            flag_upload_to_device.object = &data_device->flag_upload_to_device;
            flag_upload_to_device.object_type = e_object_type::object_flag;
            flag_upload_to_device.wait_type = e_wait_type::upload_to_device;

            vector_input(condition_variable_upload_to_device);
            vector_input(mutex_upload_to_device);
            vector_input(flag_upload_to_device);


            data_device->event_upload_to_device = CreateEvent(NULL, FALSE, FALSE, NULL);

            data_device->condition_variable_scene_to_upload = new std::condition_variable();
            data_device->mutex_scene_to_upload = new std::mutex();

            st_input_object event_upload_to_device{};
            event_upload_to_device.object = data_device->event_upload_to_device;
            event_upload_to_device.object_type = e_object_type::object_event;
            event_upload_to_device.wait_type = e_wait_type::upload_to_device;

            vector_input(event_upload_to_device);


            st_input_object condition_variable_scene_to_upload{};
            condition_variable_scene_to_upload.object = data_device->condition_variable_scene_to_upload;
            condition_variable_scene_to_upload.object_type = e_object_type::object_condition_variable;
            condition_variable_scene_to_upload.wait_type = e_wait_type::scene_to_upload;

            st_input_object mutex_scene_to_upload{};
            mutex_scene_to_upload.object = data_device->mutex_scene_to_upload;
            mutex_scene_to_upload.object_type = e_object_type::object_mutex;
            mutex_scene_to_upload.wait_type = e_wait_type::scene_to_upload;

            st_input_object flag_scene_to_upload{};
            flag_scene_to_upload.object = &data_device->flag_scene_to_upload;
            flag_scene_to_upload.object_type = e_object_type::object_flag;
            flag_scene_to_upload.wait_type = e_wait_type::scene_to_upload;

            vector_input(condition_variable_scene_to_upload);
            vector_input(mutex_scene_to_upload);
            vector_input(flag_scene_to_upload);

            for (auto it_output = _map_output.begin(); it_output != _map_output.end(); it_output++)
            {
                pst_output data_output = it_output->second;

                if (data_output->device_index != data_device->device_index)
                {
                    continue;
                }

                if (data_output->output_desc.DesktopCoordinates.left < data_device->rect_connected.left)
                {
                    data_device->rect_connected.left = data_output->output_desc.DesktopCoordinates.left;
                }
                if (data_output->output_desc.DesktopCoordinates.top < data_device->rect_connected.top)
                {
                    data_device->rect_connected.top = data_output->output_desc.DesktopCoordinates.top;
                }

                if (data_output->output_desc.DesktopCoordinates.right > data_device->rect_connected.right)
                {
                    data_device->rect_connected.right = data_output->output_desc.DesktopCoordinates.right;
                }
                if (data_output->output_desc.DesktopCoordinates.bottom > data_device->rect_connected.bottom)
                {
                    data_device->rect_connected.bottom = data_output->output_desc.DesktopCoordinates.bottom;
                }
            }

            data_device->mutex_map_text_internal = new std::shared_mutex();
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

        // ������ thread_device����, ������ ������ thread_upload���� �����
        for (UINT i = 0; i < _count_command_allocator * 2; i++)
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

    // NV12
    hr = D3DCompileFromFile(get_asset_full_path(L"shaders_nv12.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compile_flags, 0, &vertex_shader, nullptr);
    hr = D3DCompileFromFile(get_asset_full_path(L"shaders_nv12.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compile_flags, 0, &pixel_shader, nullptr);

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

        pst_command_list data_command_list = new st_command_list();

        data_command_list->device_index = data_device->device_index;

        // ������ thread_device����, ������ ������ thread_upload���� �����
        for (size_t i = 0; i < _count_command_list * 2; i++)
        {
            ID3D12GraphicsCommandList* command_list = nullptr;

            hr = data_device->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, data_command_allocator->vector_command_allocator[0], data_pso->pso, IID_PPV_ARGS(&command_list));
            command_list->Close();

            NAME_D3D12_OBJECT_INDEXED_2(command_list, data_device->device_index, i, L"ID3D12GraphicsCommandList");

            data_command_list->vector_command_list.push_back(command_list);
        }

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

        // CPU���� ������ ������ rtv heap�� �޸� ���� ��ġ
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = data_rtv_heap->rtv_heap->GetCPUDescriptorHandleForHeapStart();

        pst_rtv data_rtv = new st_rtv();

        for (UINT i = 0; i < _frame_buffer_count; i++)
        {
            ID3D12Resource* rtv = nullptr;

            data_swap_chain->swap_chain->GetBuffer(i, IID_PPV_ARGS(&rtv));
            data_device->device->CreateRenderTargetView(rtv, nullptr, cpu_handle);

            data_rtv->vector_rtv_handle.push_back(cpu_handle);

            cpu_handle.ptr = cpu_handle.ptr + data_rtv_heap->rtv_descriptor_size;

            NAME_D3D12_OBJECT_INDEXED_2(rtv, data_swap_chain->device_index, i, L"ID3D12Resource_rtv");

            data_rtv->vector_rtv.push_back(rtv);

            // --------------------------------
            // �ڸ� �������� ���� ��ü��
            ID3D11Resource* wrapped_back_buffer = nullptr;
            D3D11_RESOURCE_FLAGS flag_d3d11 = { D3D11_BIND_RENDER_TARGET };
            data_device->device_11_on_12->CreateWrappedResource(
                rtv,
                &flag_d3d11,
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_PRESENT,
                IID_PPV_ARGS(&wrapped_back_buffer)
            );
            data_rtv->vector_wrapped_back_buffer.push_back(wrapped_back_buffer);

            IDXGISurface* surface = nullptr;
            wrapped_back_buffer->QueryInterface(IID_PPV_ARGS(&surface));
            data_rtv->vector_surface.push_back(surface);

            D2D1_BITMAP_PROPERTIES1 bitmap_properties = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
            );
            ID2D1Bitmap1* rtv_2d = nullptr;
            data_device->device_context_2d->CreateBitmapFromDxgiSurface(
                surface,
                &bitmap_properties,
                &rtv_2d
            );
            data_rtv->vector_rtv_2d.push_back(rtv_2d);
            // --------------------------------
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

void create_device_11_on_12()
{
    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        auto it_command_queue = _map_command_queue.find(data_device->device_index);
        if (it_command_queue == _map_command_queue.end())
        {
            continue;
        }

        pst_command_queue data_command_queue = it_command_queue->second;

        UINT flag_d3d11 = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
        flag_d3d11 |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

        D3D11On12CreateDevice(
            data_device->device,
            flag_d3d11,
            nullptr, 0,
            (IUnknown**)&data_command_queue->command_queue, 1,
            0,
            &data_device->device_11,
            &data_device->device_context_11,
            nullptr
        );

        data_device->device_11->QueryInterface(IID_PPV_ARGS(&data_device->device_11_on_12));
        IDXGIDevice* device_dxgi = nullptr;
        data_device->device_11_on_12->QueryInterface(&device_dxgi);
        _factory_2d->CreateDevice(device_dxgi, &data_device->device_2d);

        device_dxgi->Release();
        device_dxgi = nullptr;

        D2D1_DEVICE_CONTEXT_OPTIONS device_options = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
        data_device->device_2d->CreateDeviceContext(device_options, &data_device->device_context_2d);
        
    }
}


void delete_device_11_on_12()
{
    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        if (data_device->device_context_2d != nullptr)
        {
            data_device->device_context_2d->Release();
            data_device->device_context_2d = nullptr;
        }

        if (data_device->device_2d != nullptr)
        {
            data_device->device_2d->Release();
            data_device->device_2d = nullptr;
        }

        if (data_device->device_11_on_12 != nullptr)
        {
            data_device->device_11_on_12->Release();
            data_device->device_11_on_12 = nullptr;
        }

        if (data_device->device_11 != nullptr)
        {
            data_device->device_11->Release();
            data_device->device_11 = nullptr;
        }

        if (data_device->device_context_11 != nullptr)
        {
            data_device->device_context_11->Release();
            data_device->device_context_11 = nullptr;
        }
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

        for (auto it_vector = data_rtv->vector_wrapped_back_buffer.begin(); it_vector != data_rtv->vector_wrapped_back_buffer.end();)
        {
            ID3D11Resource* wrapped_back_buffer = *it_vector;

            wrapped_back_buffer->Release();
            wrapped_back_buffer = nullptr;

            it_vector = data_rtv->vector_wrapped_back_buffer.erase(it_vector);
        }

        for (auto it_vector = data_rtv->vector_surface.begin(); it_vector != data_rtv->vector_surface.end();)
        {
            IDXGISurface* surface = *it_vector;

            surface->Release();
            surface = nullptr;

            it_vector = data_rtv->vector_surface.erase(it_vector);
        }

        for (auto it_vector = data_rtv->vector_rtv_2d.begin(); it_vector != data_rtv->vector_rtv_2d.end();)
        {
            ID2D1Bitmap1* rtv_2d = *it_vector;

            rtv_2d->Release();
            rtv_2d = nullptr;

            it_vector = data_rtv->vector_rtv_2d.erase(it_vector);
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

        if (data_swap_chain->swap_chain != nullptr)
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

        if (data_srv_heap->srv_heap != nullptr)
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

        if (data_rtv_heap->rtv_heap != nullptr)
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

        if (data_fence->fence_device != nullptr)
        {
            data_fence->fence_device->Release();
            data_fence->fence_device = nullptr;
        }

        if (data_fence->fence_upload != nullptr)
        {
            data_fence->fence_upload->Release();
            data_fence->fence_upload = nullptr;
        }

        if (data_fence->fence_event_device != nullptr)
        {
            CloseHandle(data_fence->fence_event_device);
            data_fence->fence_event_device = nullptr;
        }

        if (data_fence->fence_event_upload != nullptr)
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

        if (data_command_list->vector_command_list.size() != 0)
        {
            for (auto it_vector = data_command_list->vector_command_list.begin(); it_vector != data_command_list->vector_command_list.end();)
            {
                ID3D12GraphicsCommandList* command_list = *it_vector;

                command_list->Release();
                command_list = nullptr;

                it_vector = data_command_list->vector_command_list.erase(it_vector);
            }
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

        if (data_pso->pso != nullptr)
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

        if (data_root_sig->root_sig != nullptr)
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

        if (data_command_queue->command_queue != nullptr)
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

        if (data_device->device != nullptr)
        {
            data_device->device->Release();
            data_device->device = nullptr;
        }

        if (data_device->event_upload_to_device != nullptr)
        {
            CloseHandle(data_device->event_upload_to_device);
            data_device->event_upload_to_device = nullptr;
        }

        if (data_device->event_device_to_window != nullptr)
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

        delete data_device->mutex_map_text_internal;
        data_device->mutex_map_text_internal = nullptr;

        delete data_device;
        data_device = nullptr;

        it_device = _map_device.erase(it_device);
    }

    _vector_event_upload_to_device.clear();
    _vector_event_device_to_window.clear();
    _vector_condition_variable_scene_to_upload.clear();
    _vector_condition_variable_upload_to_device.clear();
    _vector_mutex_scene_to_upload.clear();
    _vector_mutex_upload_to_device.clear();
    _vector_flag_scene_to_upload.clear();
    _vector_flag_upload_to_device.clear();
}

void delete_windows()
{
    for (auto it_window = _map_window.begin(); it_window != _map_window.end();)
    {
        pst_window data_window = it_window->second;

        if (data_window->handle != nullptr)
        {
            DestroyWindow(data_window->handle);
            data_window->handle = nullptr;
        }

        if (data_window->event_window_to_scene != nullptr)
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

    _vector_event_window_to_scene.clear();
    _vector_condition_variable_device_to_window.clear();
    _vector_mutex_device_to_window.clear();
    _vector_flag_device_to_window.clear();
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
    if (_factory != nullptr)
    {
        _factory->Release();
        _factory = nullptr;
    }

    if (_factory_2d != nullptr)
    {
        _factory_2d->Release();
        _factory_2d = nullptr;
    }

    if (_factory_dwrite != nullptr)
    {
        _factory_dwrite->Release();
        _factory_dwrite = nullptr;
    }
}


void create_vertex_buffer(pst_device data_device, int index_command_list)
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

        // position�� -1.0 - 1.0 ������ ���� ������ uv�� 0.0 - 1.0 ������ ���� ����
        vertices[0] = { { data_scene->normal_rect.left, data_scene->normal_rect.top, 0.0f }, { 0.0f, 0.0f } };  // �»��
        vertices[1] = { { data_scene->normal_rect.right, data_scene->normal_rect.top, 0.0f }, { 1.0f, 0.0f } }; // ����
        vertices[2] = { { data_scene->normal_rect.left, data_scene->normal_rect.bottom, 0.0f }, { 0.0f, 1.0f } };   // ���ϴ�
        vertices[3] = { { data_scene->normal_rect.right, data_scene->normal_rect.bottom, 0.0f }, { 1.0f, 1.0f } };  // ���ϴ�

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
        data_command_list->vector_command_list.at(index_command_list)->ResourceBarrier(1, &transition_barrier);
        UpdateSubresources(data_command_list->vector_command_list.at(index_command_list), vertex_buffer, vertex_upload_buffer, 0, 0, 1, &vertex_data);
        transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertex_buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        data_command_list->vector_command_list.at(index_command_list)->ResourceBarrier(1, &transition_barrier);

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

void create_vertex_buffer_cef(pst_device data_device, int index_command_list, int counter_next, NormalizedRect rect_normalized)
{
    _mutex_map_vertex_buffer->lock();
    auto it_map_vertex_buffer = _map_vertex_buffer.find(data_device->device_index);
    pst_vertex_buffer data_vertex_buffer = it_map_vertex_buffer->second;
    _mutex_map_vertex_buffer->unlock();

    _mutex_map_vertex_upload_buffer->lock();
    auto it_map_vertex_upload_buffer = _map_vertex_upload_buffer.find(data_device->device_index);
    pst_vertex_upload_buffer data_vertex_upload_buffer = it_map_vertex_upload_buffer->second;
    _mutex_map_vertex_upload_buffer->unlock();

    _mutex_map_vertex_buffer_view->lock();
    auto it_map_vertex_buffer_view = _map_vertex_buffer_view.find(data_device->device_index);
    pst_vertex_buffer_view data_vertex_buffer_view = it_map_vertex_buffer_view->second;
    _mutex_map_vertex_buffer_view->unlock();

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

    // position�� -1.0 - 1.0 ������ ���� ������ uv�� 0.0 - 1.0 ������ ���� ����
    vertices[0] = { { rect_normalized.left, rect_normalized.top, 0.0f }, { 0.0f, 0.0f } };  // �»��
    vertices[1] = { { rect_normalized.right, rect_normalized.top, 0.0f }, { 1.0f, 0.0f } }; // ����
    vertices[2] = { { rect_normalized.left, rect_normalized.bottom, 0.0f }, { 0.0f, 1.0f } };   // ���ϴ�
    vertices[3] = { { rect_normalized.right, rect_normalized.bottom, 0.0f }, { 1.0f, 1.0f } };  // ���ϴ�

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
    NAME_D3D12_OBJECT_INDEXED_2(vertex_buffer, data_device->device_index, counter_next, L"ID3D12Resource_vertex_buffer");

    hr = data_device->device->CreateCommittedResource(
        &vertex_upload_buffer_properties,
        D3D12_HEAP_FLAG_NONE,
        &vertex_buffer_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertex_upload_buffer)
    );
    NAME_D3D12_OBJECT_INDEXED_2(vertex_upload_buffer, data_device->device_index, counter_next, L"ID3D12Resource_vertex_upload_buffer");

    D3D12_SUBRESOURCE_DATA vertex_data{};
    vertex_data.pData = vertices;
    vertex_data.RowPitch = sizeof(Vertex);
    vertex_data.SlicePitch = sizeof(vertices);

    auto it_command_list = _map_command_list.find(data_device->device_index);
    pst_command_list data_command_list = it_command_list->second;

    CD3DX12_RESOURCE_BARRIER transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertex_buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    data_command_list->vector_command_list.at(index_command_list)->ResourceBarrier(1, &transition_barrier);
    UpdateSubresources(data_command_list->vector_command_list.at(index_command_list), vertex_buffer, vertex_upload_buffer, 0, 0, 1, &vertex_data);
    transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertex_buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    data_command_list->vector_command_list.at(index_command_list)->ResourceBarrier(1, &transition_barrier);

    vertex_buffer_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
    vertex_buffer_view.StrideInBytes = sizeof(Vertex);
    vertex_buffer_view.SizeInBytes = vertex_buffer_size;

    data_vertex_buffer->vector_vertex_buffer.push_back(vertex_buffer);
    data_vertex_upload_buffer->vector_vertex_upload_buffer.push_back(vertex_upload_buffer);
    data_vertex_buffer_view->vector_vertex_buffer_view.push_back(vertex_buffer_view);
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

void create_index_buffer(pst_device data_device, int index_command_list)
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
    data_command_list->vector_command_list.at(index_command_list)->ResourceBarrier(1, &transition_barrier);
    UpdateSubresources(data_command_list->vector_command_list.at(index_command_list), index_buffer, index_upload_buffer, 0, 0, 1, &index_data);
    transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(index_buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    data_command_list->vector_command_list.at(index_command_list)->ResourceBarrier(1, &transition_barrier);

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

        if (data_index_buffer->index_buffer != nullptr)
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

        if (data_index_upload_buffer->index_upload_buffer != nullptr)
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

void create_srv_handles_texture_default(pst_device data_device)
{
    HRESULT hr = S_OK;

    auto it_srv_heap = _map_srv_heap.find(data_device->device_index);
    pst_srv_heap data_srv_heap = it_srv_heap->second;

    // CPU���� ������ ������ srv heap�� �޸� ���� ��ġ
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu(data_srv_heap->srv_heap->GetCPUDescriptorHandleForHeapStart());
    // GPU���� ������ ������ srv heap�� �޸� ���� ��ġ
    D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu(data_srv_heap->srv_heap->GetGPUDescriptorHandleForHeapStart());

    const UINT srv_descriptor_size = data_device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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


    {
        // CPU, GPU DESCRIPTOR HANDLE�� ù��° ��ġ�� default texture�� �Ҵ���
        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_luminance = srv_handle_cpu;
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);
        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_chrominance = srv_handle_cpu;
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);

        data_srv_handle_luminance->vector_handle_cpu.push_back(srv_handle_cpu_luminance);

        data_srv_handle_chrominance->vector_handle_cpu.push_back(srv_handle_cpu_chrominance);


        D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_luminance = srv_handle_gpu;
        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);
        data_srv_handle_luminance->vector_handle_gpu.push_back(srv_handle_gpu_luminance);

        D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_chrominance = srv_handle_gpu;
        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);
        data_srv_handle_chrominance->vector_handle_gpu.push_back(srv_handle_gpu_chrominance);
    }
}

void create_srv_handles(pst_device data_device, int counter_texture)
{
    HRESULT hr = S_OK;

    auto it_srv_heap = _map_srv_heap.find(data_device->device_index);
    pst_srv_heap data_srv_heap = it_srv_heap->second;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu(data_srv_heap->srv_heap->GetCPUDescriptorHandleForHeapStart());
    D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu(data_srv_heap->srv_heap->GetGPUDescriptorHandleForHeapStart());

    const UINT srv_descriptor_size = data_device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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


    // ù��° ��ġ�� default texture�� �Ҵ��߱� ������ 1ĭ�� �ڷ� �а� ������
    srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);
    srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);

    srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);
    srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);

    for (int i = 0; i < (_count_texture_store * counter_texture); i++)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_luminance = srv_handle_cpu;
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);
        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_chrominance = srv_handle_cpu;
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);

        data_srv_handle_luminance->vector_handle_cpu.push_back(srv_handle_cpu_luminance);

        data_srv_handle_chrominance->vector_handle_cpu.push_back(srv_handle_cpu_chrominance);


        D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_luminance = srv_handle_gpu;
        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);
        data_srv_handle_luminance->vector_handle_gpu.push_back(srv_handle_gpu_luminance);

        D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_chrominance = srv_handle_gpu;
        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);
        data_srv_handle_chrominance->vector_handle_gpu.push_back(srv_handle_gpu_chrominance);
    }
}

void create_srv_handles_cef_paint(pst_device data_device, int counter_texture, int counter_cef)
{
    HRESULT hr = S_OK;

    auto it_srv_heap = _map_srv_heap.find(data_device->device_index);
    pst_srv_heap data_srv_heap = it_srv_heap->second;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu(data_srv_heap->srv_heap->GetCPUDescriptorHandleForHeapStart());
    D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu(data_srv_heap->srv_heap->GetGPUDescriptorHandleForHeapStart());

    const UINT srv_descriptor_size = data_device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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


    // ù��° ��ġ�� default texture�� �Ҵ��߱� ������ 1ĭ�� �ڷ� �а� ������
    srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);
    srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);

    srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);
    srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);

    for (int i = 0; i < (_count_texture_store * counter_texture); i++)
    {
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);

        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);
        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);
    }
    
    for (int i = 0; i < (_count_texture_store * counter_cef); i++)
    {
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);

        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);
        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);
    }

    for (int i = 0; i < _count_texture_store; i++)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_luminance = srv_handle_cpu;
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);
        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_chrominance = srv_handle_cpu;
        srv_handle_cpu.ptr = SIZE_T(INT64(srv_handle_cpu.ptr) + srv_descriptor_size);

        data_srv_handle_luminance->vector_handle_cpu.push_back(srv_handle_cpu_luminance);

        data_srv_handle_chrominance->vector_handle_cpu.push_back(srv_handle_cpu_chrominance);


        D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_luminance = srv_handle_gpu;
        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);
        data_srv_handle_luminance->vector_handle_gpu.push_back(srv_handle_gpu_luminance);

        D3D12_GPU_DESCRIPTOR_HANDLE srv_handle_gpu_chrominance = srv_handle_gpu;
        srv_handle_gpu.ptr = SIZE_T(INT64(srv_handle_gpu.ptr) + srv_descriptor_size);
        data_srv_handle_chrominance->vector_handle_gpu.push_back(srv_handle_gpu_chrominance);
    }
}

void delete_textures()
{
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

void upload_texture(pst_device data_device, AVFrame* frame, int counter_texture, int srv_index)
{
    // ù��°�� default texture�� �Ҵ��߱� ������ ��ġ�� 1 ��ŭ �߰��� �̵���
    int texture_index = (_count_texture_store * counter_texture) + srv_index + 1;

    ID3D12Resource* srv_resource = ((AVD3D12VAFrame*)frame->data[0])->texture;

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

    // Y ä�� �Ҵ�
    srv_desc.Format = DXGI_FORMAT_R8_UNORM;
    srv_desc.Texture2D.PlaneSlice = 0;
    data_device->device->CreateShaderResourceView(srv_resource, &srv_desc, srv_handle_cpu_luminance);

    // UV ä�� �Ҵ�
    srv_desc.Format = DXGI_FORMAT_R8G8_UNORM;
    srv_desc.Texture2D.PlaneSlice = 1;
    data_device->device->CreateShaderResourceView(srv_resource, &srv_desc, srv_handle_cpu_chrominance);
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

        auto logger = spdlog::get("wplayer_logger");

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

        auto logger = spdlog::get("wplayer_logger");
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

        auto logger = spdlog::get("wplayer_logger");

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


int create_scenes()
{
    bool flag_open_file_fail = false;

    for (int i = 0; i < _count_scene; i++)
    {
        bool flag_open_file_fail_internal = false;

        auto it_scene_url = _map_scene_url.find(i);
        auto it_scene_rect_left = _map_scene_rect_left.find(i);
        auto it_scene_rect_top = _map_scene_rect_top.find(i);
        auto it_scene_rect_width = _map_scene_rect_width.find(i);
        auto it_scene_rect_height = _map_scene_rect_height.find(i);

        std::string url = it_scene_url->second;
        int left = it_scene_rect_left->second;
        int top = it_scene_rect_top->second;
        int width = it_scene_rect_width->second;
        int height = it_scene_rect_height->second;

        RECT rect{ 0, 0, 0, 0 };
        rect.left = left;
        rect.top = top;
        rect.right = left + width;
        rect.bottom = top + height;

        UINT device_index = UINT_MAX;
        UINT output_index = UINT_MAX;

        for (auto it_output = _map_output.begin(); it_output != _map_output.end(); it_output++)
        {
            pst_output data_output = it_output->second;

            // rect�� data_output->output_desc.DesktopCoordinates �� ��ġ�� �κ��� ������ Ȯ����
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

        int result = 0;
        void* instance_0 = nullptr;
        UINT scene_index = UINT_MAX;

        create_ffmpeg_instance_set_data(instance_0, device_index, url);

        bool flag_is_realtime = false;
        cpp_ffmpeg_wrapper_get_is_realtime(instance_0, flag_is_realtime);

        int result_on_live_stream = 0;

        if (flag_is_realtime)
        {
            result_on_live_stream = create_ffmpeg_instance_check_open_file_on_live_stream(instance_0);
        }
        else
        {
            result = create_ffmpeg_instance_check_open_file(instance_0);
            if (result != 0)
            {
                flag_open_file_fail = true;
                flag_open_file_fail_internal = true;

                if (_flag_set_logger)
                {
                    std::string str = "";
                    str.append("file is not exist, ");
                    str.append(", url = ");
                    str.append(url);

                    auto logger = spdlog::get("wplayer_logger");
                    logger->warn(str.c_str());
                }
            }
        }

        void* instance_1 = nullptr;
        void* instance_2 = nullptr;
        if (flag_open_file_fail_internal == false)
        {
            if (flag_is_realtime == true)
            {
                create_ffmpeg_instance_play_start_set_data(instance_0, scene_index, rect);

                if (result_on_live_stream == 0)
                {
                    create_ffmpeg_instance_play_start_on_live_stream(instance_0);
                }
            }

            if (flag_is_realtime == false)
            {
                create_ffmpeg_instance_play_start(instance_0, scene_index, rect);
                create_ffmpeg_instance_with_scene_index(instance_1, device_index, url, scene_index, rect);
                create_ffmpeg_instance_with_scene_index(instance_2, device_index, url, scene_index, rect);
            }
        }
        else
        {
            scene_index = _next_scene_index;
            _next_scene_index++;
        }

        pst_scene data_scene = new st_scene();

        if (flag_open_file_fail_internal == false)
        {
            data_scene->map_ffmpeg_instance.insert({ 0, instance_0 });

            if (flag_is_realtime == false)
            {
                data_scene->map_ffmpeg_instance.insert({ 1, instance_1 });
                data_scene->map_ffmpeg_instance.insert({ 2, instance_2 });
            }
        }

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
        event_scene_to_upload.object_type = e_object_type::object_event;
        event_scene_to_upload.wait_type = e_wait_type::scene_to_upload;

        vector_input(event_scene_to_upload);


        st_input_object condition_variable_window_to_scene{};
        condition_variable_window_to_scene.object = data_scene->condition_variable_window_to_scene;
        condition_variable_window_to_scene.object_type = e_object_type::object_condition_variable;
        condition_variable_window_to_scene.wait_type = e_wait_type::window_to_scene;

        st_input_object mutex_window_to_scene{};
        mutex_window_to_scene.object = data_scene->mutex_window_to_scene;
        mutex_window_to_scene.object_type = e_object_type::object_mutex;
        mutex_window_to_scene.wait_type = e_wait_type::window_to_scene;

        st_input_object flag_window_to_scene{};
        flag_window_to_scene.object = &data_scene->flag_window_to_scene;
        flag_window_to_scene.object_type = e_object_type::object_flag;
        flag_window_to_scene.wait_type = e_wait_type::window_to_scene;

        vector_input(condition_variable_window_to_scene);
        vector_input(mutex_window_to_scene);
        vector_input(flag_window_to_scene);

        RECT base_rect_window{ 0, 0, 0, 0 };
        for (auto it_window = _map_window.begin(); it_window != _map_window.end(); it_window++)
        {
            pst_window data_window = it_window->second;

            // rect�� data_window->rect �� ��ġ�� �κ��� ������ Ȯ����
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

        data_scene->mutex_deque_index_used = new std::mutex();

        data_scene->mutex_deque_index_unref = new std::mutex();

        _map_scene.insert({ scene_index, data_scene });
    }

    if (flag_open_file_fail == true)
    {
        return 1;
    }

    return 0;
}

void delete_scenes()
{
    for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end();)
    {
        pst_scene data_scene = it_scene->second;

        for (auto it = data_scene->map_ffmpeg_instance.begin(); it != data_scene->map_ffmpeg_instance.end();)
        {
            if (it->second != nullptr)
            {
                cpp_ffmpeg_wrapper_play_stop(it->second, nullptr);
                cpp_ffmpeg_wrapper_shutdown(it->second);
                cpp_ffmpeg_wrapper_delete(it->second);
                it->second = nullptr;
            }

            it = data_scene->map_ffmpeg_instance.erase(it);
        }

        for (auto it_vector = data_scene->vector_frame.begin(); it_vector != data_scene->vector_frame.end();)
        {
            AVFrame* frame = *it_vector;

            av_frame_free(&frame);

            it_vector = data_scene->vector_frame.erase(it_vector);
        }

        if (data_scene->mutex_deque_index_used != nullptr)
        {
            data_scene->mutex_deque_index_used->lock();
            for (auto it_vector = data_scene->deque_index_used.begin(); it_vector != data_scene->deque_index_used.end();)
            {
                it_vector = data_scene->deque_index_used.erase(it_vector);
            }
            data_scene->mutex_deque_index_used->unlock();
            delete data_scene->mutex_deque_index_used;
            data_scene->mutex_deque_index_used = nullptr;
        }

        if (data_scene->mutex_deque_index_unref != nullptr)
        {
            data_scene->mutex_deque_index_unref->lock();
            for (auto it_vector = data_scene->deque_index_unref.begin(); it_vector != data_scene->deque_index_unref.end();)
            {
                it_vector = data_scene->deque_index_unref.erase(it_vector);
            }
            data_scene->mutex_deque_index_unref->unlock();
            delete data_scene->mutex_deque_index_unref;
            data_scene->mutex_deque_index_unref = nullptr;
        }

        if (data_scene->event_scene_to_upload != nullptr)
        {
            CloseHandle(data_scene->event_scene_to_upload);
            data_scene->event_scene_to_upload = nullptr;
        }

        if (data_scene->condition_variable_window_to_scene != nullptr)
        {
            delete data_scene->condition_variable_window_to_scene;
            data_scene->condition_variable_window_to_scene = nullptr;
        }

        if (data_scene->mutex_window_to_scene != nullptr)
        {
            delete data_scene->mutex_window_to_scene;
            data_scene->mutex_window_to_scene = nullptr;
        }

        if (data_scene != nullptr)
        {
            delete data_scene;
            data_scene = nullptr;
        }

        it_scene = _map_scene.erase(it_scene);
    }

    _vector_event_scene_to_upload.clear();
    _vector_condition_variable_window_to_scene.clear();
    _vector_mutex_window_to_scene.clear();
    _vector_flag_window_to_scene.clear();
}

void callback_ffmpeg_wrapper_ptr(void* param)
{
    // ffmpeg �ν��Ͻ��� �ݹ��� ������ �ʿ䰡 ���� ������ �� �Լ���
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

void get_asset_path(wchar_t* path, uint32_t path_size)
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
    std::wstring str_ini_path_w = str_path_w + L"\\WPlayer_Alpha.ini";

    char path_a[260] = { 0, };
    GetModuleFileNameA(nullptr, path_a, 260);
    std::string str_path_a = path_a;
    str_path_a = str_path_a.substr(0, str_path_a.find_last_of("\\/"));
    std::string str_ini_path_a = str_path_a + "\\WPlayer_Alpha.ini";

    char result_a[255];
    wchar_t result_w[255];
    int result_i = 0;

    GetPrivateProfileStringA("WPlayer", "IP", "", result_a, 255, str_ini_path_a.c_str());
    _ip = result_a;

    GetPrivateProfileString(L"WPlayer", L"PORT", L"0", result_w, 255, str_ini_path_w.c_str());
    _port = _ttoi(result_w);

    GetPrivateProfileString(L"WPlayer", L"flag_set_logger", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _flag_set_logger = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"log_level", L"6", result_w, 255, str_ini_path_w.c_str());
    _log_level = _ttoi(result_w);

    GetPrivateProfileString(L"WPlayer", L"log_file_size", L"1", result_w, 255, str_ini_path_w.c_str());
    _log_file_size = _ttoi(result_w);

    GetPrivateProfileString(L"WPlayer", L"log_file_rotation_count", L"3", result_w, 255, str_ini_path_w.c_str());
    _log_file_rotation_count = _ttoi(result_w);

    GetPrivateProfileString(L"WPlayer", L"wait_for_multiple_objects_wait_time", L"1000", result_w, 255, str_ini_path_w.c_str());
    _wait_for_multiple_objects_wait_time = _ttoi(result_w);

    GetPrivateProfileString(L"WPlayer", L"count_use_last_frame_at_repeat", L"30", result_w, 255, str_ini_path_w.c_str());
    _count_use_last_frame_at_repeat = _ttoi(result_w);
    
    GetPrivateProfileString(L"WPlayer", L"background_color_r", L"0", result_w, 255, str_ini_path_w.c_str());
    _background_color_r = _ttoi(result_w);
    GetPrivateProfileString(L"WPlayer", L"background_color_g", L"0", result_w, 255, str_ini_path_w.c_str());
    _background_color_g = _ttoi(result_w);
    GetPrivateProfileString(L"WPlayer", L"background_color_b", L"0", result_w, 255, str_ini_path_w.c_str());
    _background_color_b = _ttoi(result_w);

    if (_background_color_r > 255 || _background_color_r < 0)
    {
        _background_color_r = 0;
    }
    if (_background_color_g > 255 || _background_color_g < 0)
    {
        _background_color_g = 0;
    }
    if (_background_color_b > 255 || _background_color_b < 0)
    {
        _background_color_b = 0;
    }

    _background_color_r_float = _background_color_r / 255.0f;
    _background_color_g_float = _background_color_g / 255.0f;
    _background_color_b_float = _background_color_b / 255.0f;

    GetPrivateProfileStringA("WPlayer", "default_texture_url", "", result_a, 255, str_ini_path_a.c_str());
    _default_texture_url = result_a;

    GetPrivateProfileString(L"WPlayer", L"use_nvapi", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _use_nvapi = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"block_swap_group_present", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _block_swap_group_present = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"count_texture_store", L"30", result_w, 255, str_ini_path_w.c_str());
    _count_texture_store = _ttoi(result_w);

    GetPrivateProfileString(L"WPlayer", L"hw_device_type", L"12", result_w, 255, str_ini_path_w.c_str());
    _hw_device_type = _ttoi(result_w);

    GetPrivateProfileString(L"WPlayer", L"flag_use_cef", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _flag_use_cef = result_i == 0 ? false : true;

    GetPrivateProfileStringA("WPlayer", "cef_browser_subprocess_path", "", result_a, 255, str_ini_path_a.c_str());
    _cef_browser_subprocess_path = result_a;

    GetPrivateProfileString(L"WPlayer", L"use_control_output", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _use_control_output = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"control_output_left", L"0", result_w, 255, str_ini_path_w.c_str());
    _control_output_left = _ttoi(result_w);
    GetPrivateProfileString(L"WPlayer", L"control_output_top", L"0", result_w, 255, str_ini_path_w.c_str());
    _control_output_top = _ttoi(result_w);
    GetPrivateProfileString(L"WPlayer", L"control_output_width", L"0", result_w, 255, str_ini_path_w.c_str());
    _control_output_width = _ttoi(result_w);
    GetPrivateProfileString(L"WPlayer", L"control_output_height", L"0", result_w, 255, str_ini_path_w.c_str());
    _control_output_height = _ttoi(result_w);

    GetPrivateProfileString(L"WPlayer", L"use_manual_window_create", L"0", result_w, 255, str_ini_path_w.c_str());
    result_i = _ttoi(result_w);
    _use_manual_window_create = result_i == 0 ? false : true;

    GetPrivateProfileString(L"WPlayer", L"count_manual_window", L"0", result_w, 255, str_ini_path_w.c_str());
    _count_manual_window = _ttoi(result_w);

    for (UINT i = 0; i < _count_manual_window; i++)
    {
        int left;
        int top;
        int width;
        int height;

        std::wstring option_string_left = L"manual_window_rect_left";
        std::wstring option_string_top = L"manual_window_rect_top";
        std::wstring option_string_width = L"manual_window_rect_width";
        std::wstring option_string_height = L"manual_window_rect_height";

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

        _map_manual_window_rect_left.insert({ i, left });
        _map_manual_window_rect_top.insert({ i, top });
        _map_manual_window_rect_width.insert({ i, width });
        _map_manual_window_rect_height.insert({ i, height });
    }

    GetPrivateProfileString(L"WPlayer", L"count_scene", L"0", result_w, 255, str_ini_path_w.c_str());
    _count_scene = _ttoi(result_w);

    for (int i = 0; i < _count_scene; i++)
    {
        std::string option_string_url = "scene_url";
        option_string_url.append(std::to_string(i));
        GetPrivateProfileStringA("WPlayer", option_string_url.c_str(), "", result_a, 255, str_ini_path_a.c_str());
        _map_scene_url.insert({ i, result_a });

        int left;
        int top;
        int width;
        int height;

        std::wstring option_string_left = L"scene_rect_left";
        std::wstring option_string_top = L"scene_rect_top";
        std::wstring option_string_width = L"scene_rect_width";
        std::wstring option_string_height = L"scene_rect_height";

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

        _map_scene_rect_left.insert({ i, left });
        _map_scene_rect_top.insert({ i, top });
        _map_scene_rect_width.insert({ i, width });
        _map_scene_rect_height.insert({ i, height });
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
            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_processing));
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
        case e_command_type::program_quit:
        {
            _is_running = false;
        }
        break;

        case e_command_type::font_create:
        {
            packet_font_create_from_client* packet = (packet_font_create_from_client*)data_pair.first;

            create_text(packet->index_font);

            std::wstring text_string;
            convert_ansi_to_unicode_string(text_string, packet->content_string, packet->content_size);

            set_text_string(packet->index_font, text_string);
            set_text_color(packet->index_font, packet->font_color_r, packet->font_color_g, packet->font_color_b, packet->font_color_a);
            set_text_size(packet->index_font, packet->font_size);

            std::wstring text_font_family;
            convert_ansi_to_unicode_string(text_font_family, packet->font_family, packet->font_family_size);

            set_text_font_family(packet->index_font, text_font_family);
            set_text_color_background(packet->index_font, packet->background_color_r, packet->background_color_g, packet->background_color_b, packet->background_color_a);
            set_text_start_coordinate_left(packet->index_font, packet->font_start_coordinate_left);
            set_text_start_coordinate_top(packet->index_font, packet->font_start_coordinate_top);
            set_text_background_width(packet->index_font, packet->backgound_rectangle_width);
            set_text_background_height(packet->index_font, packet->backgound_rectangle_height);
            set_text_weight(packet->index_font, (e_dwrite_font_weight)packet->font_weight);
            set_text_style(packet->index_font, (e_dwrite_font_style)packet->font_style);
            set_text_stretch(packet->index_font, (e_dwrite_font_stretch)packet->font_stretch);
            set_text_movement_type_horizontal(packet->index_font, (e_movement_type_horizontal)packet->movement_type_horizontal);
            set_text_movement_speed_horizontal(packet->index_font, packet->movement_speed_horizontal);
            set_text_movement_threshold_horizontal(packet->index_font, packet->movement_threshold_horizontal);
            set_text_movement_type_horizontal_background(packet->index_font, (e_movement_type_horizontal)packet->movement_type_horizontal_background);
            set_text_movement_speed_horizontal_background(packet->index_font, packet->movement_speed_horizontal_background);
            set_text_movement_threshold_horizontal_background(packet->index_font, packet->movement_threshold_horizontal_background);
            set_text_movement_type_vertical(packet->index_font, (e_movement_type_vertical)packet->movement_type_vertical);
            set_text_movement_speed_vertical(packet->index_font, packet->movement_speed_vertical);
            set_text_movement_threshold_vertical(packet->index_font, packet->movement_threshold_vertical);
            set_text_movement_type_vertical_background(packet->index_font, (e_movement_type_vertical)packet->movement_type_vertical_background);
            set_text_movement_speed_vertical_background(packet->index_font, packet->movement_speed_vertical_background);
            set_text_movement_threshold_vertical_background(packet->index_font, packet->movement_threshold_vertical_background);

            cppsocket_struct_server_send_font_create* data = new cppsocket_struct_server_send_font_create();
            data->result = (uint16_t)e_packet_result::ok;
            data->index_font = packet->index_font;
            data->font_size = packet->font_size;
            
            data->font_color_r = packet->font_color_r;
            data->font_color_g = packet->font_color_g;
            data->font_color_b = packet->font_color_b;
            data->font_color_a = packet->font_color_a;

            data->background_color_r = packet->background_color_r;
            data->background_color_g = packet->background_color_g;
            data->background_color_b = packet->background_color_b;
            data->background_color_a = packet->background_color_a;

            data->movement_type_horizontal = packet->movement_type_horizontal;
            data->movement_speed_horizontal = packet->movement_speed_horizontal;
            data->movement_threshold_horizontal = packet->movement_threshold_horizontal;

            data->movement_type_horizontal_background = packet->movement_type_horizontal_background;
            data->movement_speed_horizontal_background = packet->movement_speed_horizontal_background;
            data->movement_threshold_horizontal_background = packet->movement_threshold_horizontal_background;

            data->movement_type_vertical = packet->movement_type_vertical;
            data->movement_speed_vertical = packet->movement_speed_vertical;
            data->movement_threshold_vertical = packet->movement_threshold_vertical;

            data->movement_type_vertical_background = packet->movement_type_vertical_background;
            data->movement_speed_vertical_background = packet->movement_speed_vertical_background;
            data->movement_threshold_vertical_background = packet->movement_threshold_vertical_background;

            data->font_start_coordinate_left = packet->font_start_coordinate_left;
            data->font_start_coordinate_top = packet->font_start_coordinate_top;

            data->backgound_rectangle_width = packet->backgound_rectangle_width;
            data->backgound_rectangle_height = packet->backgound_rectangle_height;

            data->font_weight = packet->font_weight;
            data->font_style = packet->font_style;
            data->font_stretch = packet->font_stretch;

            data->content_size = packet->content_size;
            memcpy(data->content_string, packet->content_string, packet->content_size);

            data->font_family_size = packet->font_family_size;
            memcpy(data->font_family, packet->font_family, packet->font_family_size);

            cppsocket_server_send_font_create(_tcp_server, data_pair.second, *data);

            delete data;
        }
        break;
        case e_command_type::font_delete:
        {
            packet_font_delete_from_client* packet = (packet_font_delete_from_client*)data_pair.first;

            delete_text(packet->index_font);

            cppsocket_struct_server_send_font_delete* data = new cppsocket_struct_server_send_font_delete();
            data->result = (uint16_t)e_packet_result::ok;
            data->index_font = packet->index_font;

            cppsocket_server_send_font_delete(_tcp_server, data_pair.second, *data);

            delete data;
        }
        break;
        case e_command_type::font_blink_turn_on_off:
        {
            packet_font_blink_turn_on_off_from_client* packet = (packet_font_blink_turn_on_off_from_client*)data_pair.first;

            set_text_blink_turn_on_off(packet->index_font, packet->flag_blink_turn_on_off);

            cppsocket_struct_server_send_font_blink_turn_on_off* data = new cppsocket_struct_server_send_font_blink_turn_on_off();
            data->result = (uint16_t)e_packet_result::ok;
            data->index_font = packet->index_font;
            data->flag_blink_turn_on_off = packet->flag_blink_turn_on_off;

            cppsocket_server_send_font_blink_turn_on_off(_tcp_server, data_pair.second, *data);

            delete data;
        }
        break;
        case e_command_type::font_blink_interval:
        {
            packet_font_blink_interval_from_client* packet = (packet_font_blink_interval_from_client*)data_pair.first;

            set_text_blink_interval(packet->interval_blink_in_miliseconds);

            cppsocket_struct_server_send_font_blink_interval* data = new cppsocket_struct_server_send_font_blink_interval();
            data->result = (uint16_t)e_packet_result::ok;
            data->interval_blink_in_miliseconds = packet->interval_blink_in_miliseconds;

            cppsocket_server_send_font_blink_interval(_tcp_server, data_pair.second, *data);

            delete data;
        }
        break;
        case e_command_type::font_blink_duration:
        {
            packet_font_blink_duration_from_client* packet = (packet_font_blink_duration_from_client*)data_pair.first;

            set_text_blink_duration(packet->duration_blink_in_miliseconds);

            cppsocket_struct_server_send_font_blink_duration* data = new cppsocket_struct_server_send_font_blink_duration();
            data->result = (uint16_t)e_packet_result::ok;
            data->duration_blink_in_miliseconds = packet->duration_blink_in_miliseconds;

            cppsocket_server_send_font_blink_duration(_tcp_server, data_pair.second, *data);

            delete data;
        }
        break;

        case e_command_type::cef_create:
        {
            packet_cef_create_from_client* packet = (packet_cef_create_from_client*)data_pair.first;

            std::string url_str;
            url_str.assign(packet->url, packet->url_size);

            HWND handle = nullptr;
            RECT rect{ packet->left, packet->top, packet->left + packet->width, packet->top + packet->height };

            for (auto it_window = _map_window.begin(); it_window != _map_window.end(); it_window++)
            {
                pst_window data_window = it_window->second;

                if (rect.left < data_window->rect.right
                    && rect.top < data_window->rect.bottom
                    && rect.right > data_window->rect.left
                    && rect.bottom > data_window->rect.top)
                {
                    handle = data_window->handle;
                    break;
                }
            }

            if (_flag_use_cef)
            {
                create_cef(packet->index_cef, handle, url_str, rect);
            }

            cppsocket_struct_server_send_cef_create* data = new cppsocket_struct_server_send_cef_create();
            data->result = (uint16_t)e_packet_result::ok;
            data->index_cef = packet->index_cef;
            data->left = packet->left;
            data->top = packet->top;
            data->width = packet->width;
            data->height = packet->height;
            data->url_size = packet->url_size;
            memcpy(data->url, packet->url, packet->url_size);

            cppsocket_server_send_cef_create(_tcp_server, data_pair.second, *data);

            delete data;
        }
        break;
        case e_command_type::cef_delete:
        {
            packet_cef_delete_from_client* packet = (packet_cef_delete_from_client*)data_pair.first;

            if (_flag_use_cef)
            {
                delete_cef(packet->index_cef);
            }

            cppsocket_struct_server_send_cef_delete* data = new cppsocket_struct_server_send_cef_delete();
            data->result = (uint16_t)e_packet_result::ok;
            data->index_cef = packet->index_cef;

            cppsocket_server_send_cef_delete(_tcp_server, data_pair.second, *data);

            delete data;
        }
        break;

        default:
            break;
        }

        delete[] data_pair.first;
    }
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
            case e_object_type::object_event:
            {
                object_event = (HANDLE)object.object;
            }
            break;
            case e_object_type::object_condition_variable:
            {
                object_condition_variable = (std::condition_variable*)object.object;
            }
            break;
            case e_object_type::object_mutex:
            {
                object_mutex = (std::mutex*)object.object;
            }
            break;
            case e_object_type::object_flag:
            {
                object_flag = (bool*)object.object;
            }
            break;
            default:
                break;
            }

            switch (object.wait_type)
            {
            case e_wait_type::scene_to_upload:
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
            case e_wait_type::upload_to_device:
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
            case e_wait_type::device_to_window:
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
            case e_wait_type::window_to_scene:
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

void thread_wait_for_multiple_objects(e_wait_type wait_type, bool* flag_thread)
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
        case e_wait_type::scene_to_upload:
        {
            vector_handle = &_vector_event_scene_to_upload;
            _mutex_vector_event_scene_to_upload.lock();
            if (vector_handle->size() < _map_scene.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_event_scene_to_upload.unlock();
            for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
            {
                pst_scene data_scene = it_scene->second;

                if (data_scene->map_ffmpeg_instance.size() == 0)
                {
                    flag_vector_need_more = false;
                    break;
                }
            }

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
        case e_wait_type::upload_to_device:
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
        case e_wait_type::device_to_window:
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
        case e_wait_type::window_to_scene:
        {
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

            for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
            {
                pst_scene data_scene = it_scene->second;

                if (data_scene->map_ffmpeg_instance.size() == 0)
                {
                    flag_vector_need_more = false;
                    break;
                }
            }

            vector_handle = &_vector_event_window_to_scene;
            _mutex_vector_event_window_to_scene.lock();
            if (vector_handle->size() < _map_window.size())
            {
                flag_vector_need_more = true;
            }
            _mutex_vector_event_window_to_scene.unlock();
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
        if (n_count != 0)
        {
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
        }

        if (flag_timeout == true)
        {
            flag_timeout = false;
            continue;
        }

        if (wait_type == e_wait_type::scene_to_upload)
        {
            int count_scene = _map_scene.size();
            bool flag_use_last_frame = false;

            for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
            {
                pst_scene data_scene = it_scene->second;

                if (data_scene->flag_ready_to_frame_use == true)
                {
                    count_scene--;
                }

                if (data_scene->flag_use_last_frame == true)
                {
                    flag_use_last_frame = true;
                }
            }

            if (count_scene == 0)
            {
                for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
                {
                    pst_device data_device = it_device->second;

                    data_device->flag_ready_to_device_use = true;

                    if (flag_use_last_frame == true)
                    {
                        data_device->flag_use_last_frame = true;
                    }
                    else
                    {
                        data_device->flag_use_last_frame = false;
                    }
                }
            }
            else
            {
                for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
                {
                    pst_device data_device = it_device->second;

                    data_device->flag_ready_to_device_use = false;

                    if (flag_use_last_frame == true)
                    {
                        data_device->flag_use_last_frame = true;
                    }
                    else
                    {
                        data_device->flag_use_last_frame = false;
                    }
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

    pst_viewport data_viewport = nullptr;
    for (auto it_viewport = _map_viewport.begin(); it_viewport != _map_viewport.end(); it_viewport++)
    {
        pst_viewport data_viewport_temp = it_viewport->second;

        if (data_viewport_temp->device_index == data_device->device_index)
        {
            data_viewport = data_viewport_temp;
            break;
        }
    }

    auto it_command_queue = _map_command_queue.find(data_device->device_index);
    pst_command_queue data_command_queue = it_command_queue->second;

    auto it_fence = _map_fence.find(data_device->device_index);
    pst_fence data_fence = it_fence->second;

    pst_swap_chain data_swap_chain = nullptr;
    for (auto it_swapchain = _map_swap_chain.begin(); it_swapchain != _map_swap_chain.end(); it_swapchain++)
    {
        pst_swap_chain data_swap_chain_temp = it_swapchain->second;

        if (data_swap_chain_temp->device_index == data_device->device_index)
        {
            data_swap_chain = data_swap_chain_temp;
            break;
        }
    }

    bool flag_vector_ready = false;

    int index_command_allocator = -1;
    int srv_index = -1;

    std::vector<pst_scene> vector_scene;

    int index_command_list = -1;

    while (data_device->flag_thread_device)
    {
        // flag_ready_to_device_use false - index to 0

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

            auto logger = spdlog::get("wplayer_logger");
            logger->debug(str.c_str());
        }

        index_command_allocator += 1;
        if (!(index_command_allocator < _count_command_allocator))
        {
            index_command_allocator = 0;
        }

        index_command_list += 1;
        if (!(index_command_list < _count_command_list))
        {
            index_command_list = 0;
        }

        if (data_device->flag_ready_to_device_use)
        {
            if (data_device->flag_use_last_frame == false)
            {
                srv_index += 1;
                if (!(srv_index < _count_texture_store))
                {
                    srv_index = 0;
                }
            }
        }

        if (!data_device->flag_thread_device)
        {
            break;
        }

        ID3D12CommandAllocator* command_allocator = data_command_allocator->vector_command_allocator.at(index_command_allocator);
        ID3D12GraphicsCommandList* command_list = data_command_list->vector_command_list.at(index_command_list);
        ID3D12PipelineState* pso = data_pso->pso;

        command_allocator->Reset();
        command_list->Reset(command_allocator, pso);

        if (flag_vector_ready == false)
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

            flag_vector_ready = true;
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

        IDXGISwapChain3* swapchain3 = nullptr;
        data_swap_chain->swap_chain->QueryInterface(IID_PPV_ARGS(&swapchain3));

        int backbuffer_index = swapchain3->GetCurrentBackBufferIndex();
        swapchain3->Release();

        CD3DX12_RESOURCE_BARRIER barrier_before = CD3DX12_RESOURCE_BARRIER::Transition(data_rtv->vector_rtv.at(backbuffer_index), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        command_list->ResourceBarrier(1, &barrier_before);

        command_list->OMSetRenderTargets(1, &data_rtv->vector_rtv_handle.at(backbuffer_index), FALSE, nullptr);

        float color[4] = { _background_color_r_float, _background_color_g_float, _background_color_b_float, 1.0f };
        command_list->ClearRenderTargetView(data_rtv->vector_rtv_handle.at(backbuffer_index), color, 0, nullptr);
        command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        command_list->IASetIndexBuffer(&data_index_buffer_view->index_buffer_view);

        bool flag_play_started = true;
        uint64_t counter = 0;
        for (auto it_vector = vector_scene.begin(); it_vector != vector_scene.end(); it_vector++)
        {
            pst_scene data_scene = *it_vector;

            void* ffmpeg_instance = nullptr;
            auto it_ffmpeg_instance = data_scene->map_ffmpeg_instance.find(data_scene->index_ffmpeg_instance_current);
            if (it_ffmpeg_instance != data_scene->map_ffmpeg_instance.end())
            {
                ffmpeg_instance = it_ffmpeg_instance->second;
            }

            if (ffmpeg_instance && flag_play_started)
            {
                cpp_ffmpeg_wrapper_get_flag_play_started(ffmpeg_instance, flag_play_started);
            }

            command_list->IASetVertexBuffers(0, 1, &data_vertex_buffer_view->vector_vertex_buffer_view.at(counter));

            command_list->RSSetViewports(1, &data_viewport->viewport);
            command_list->RSSetScissorRects(1, &data_viewport->scissor_rect);

            if (data_scene->map_ffmpeg_instance.size() != 0 && flag_play_started)
            {
                command_list->SetGraphicsRootDescriptorTable(0, data_srv_handle_luminance->vector_handle_gpu.at((_count_texture_store * counter) + srv_index + 1));
                command_list->SetGraphicsRootDescriptorTable(1, data_srv_handle_chrominance->vector_handle_gpu.at((_count_texture_store * counter) + srv_index + 1));
            }
            else
            {
                // default texture ���
                command_list->SetGraphicsRootDescriptorTable(0, data_srv_handle_luminance->vector_handle_gpu.at(0));
                command_list->SetGraphicsRootDescriptorTable(1, data_srv_handle_chrominance->vector_handle_gpu.at(0));
            }

            command_list->DrawIndexedInstanced(6, 1, 0, 0, 0);

            counter++;
        }

        // draw cef texture buffer data
        if (_flag_use_cef)
        {
            for (auto it_cef = _map_cef.begin(); it_cef != _map_cef.end(); it_cef++)
            {
                pst_cef data_cef = it_cef->second;

                for (auto it_cef_device = data_cef->map_cef_device.begin(); it_cef_device != data_cef->map_cef_device.end(); it_cef_device++)
                {
                    pst_cef_device data_cef_device = it_cef_device->second;

                    if (data_cef_device->index_cef_device != data_device->device_index)
                    {
                        continue;
                    }

                    if (data_cef_device->flag_texture_created == false)
                    {
                        continue;
                    }

                    command_list->IASetVertexBuffers(0, 1, &data_vertex_buffer_view->vector_vertex_buffer_view.at(counter + data_cef_device->count_cef_device));

                    command_list->RSSetViewports(1, &data_viewport->viewport);
                    command_list->RSSetScissorRects(1, &data_viewport->scissor_rect);

                    if (is_queue_empty(data_cef->index_map_input, data_cef->index_map_output) == false)
                    {
                        command_list->SetGraphicsRootDescriptorTable(0, data_srv_handle_luminance->vector_handle_gpu.at((_count_texture_store * (counter + data_cef_device->count_cef_device)) + data_cef_device->index_srv_draw + 1));
                        command_list->SetGraphicsRootDescriptorTable(1, data_srv_handle_chrominance->vector_handle_gpu.at((_count_texture_store * (counter + data_cef_device->count_cef_device)) + data_cef_device->index_srv_draw + 1));

                        data_cef_device->index_srv_draw++;
                        if (data_cef_device->index_srv_draw >= _count_texture_store)
                        {
                            data_cef_device->index_srv_draw = 0;
                        }

                        data_cef->mutex_deque_index_used->lock();
                        data_cef->deque_index_used->push_back(data_cef->index_upload_texture);
                        data_cef->mutex_deque_index_used->unlock();
                    }

                    command_list->DrawIndexedInstanced(6, 1, 0, 0, 0);
                }
            }
        }

        CD3DX12_RESOURCE_BARRIER barrier_after = CD3DX12_RESOURCE_BARRIER::Transition(data_rtv->vector_rtv.at(backbuffer_index), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        command_list->ResourceBarrier(1, &barrier_after);

        command_list->Close();

        ID3D12CommandList* command_lists[] = { command_list };

        ID3D12CommandQueue* command_queue = data_command_queue->command_queue;

        command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

        // ----------------------------------------------------------------
#pragma region DWrite ���
        // DWrite ���
        auto it_text = _map_text.find(data_device->device_index);
        pst_text data_text = it_text->second;

        if (data_text->flag_blink_status == true)
        {
            data_text->counter_duration_blink += 1;
            if (data_text->counter_duration_blink > data_text->duration_blink_in_miliseconds / 1'000.0 * 60.0)
            {
                data_text->counter_duration_blink = 0;
                data_text->flag_blink_status = false;
            }
        }
        else
        {
            data_text->counter_interval_blink += 1;
            if (data_text->counter_interval_blink > data_text->interval_blink_in_miliseconds / 1'000.0 * 60.0)
            {
                data_text->counter_interval_blink = 0;
                data_text->flag_blink_status = true;
            }
        }

        {
            std::unique_lock<std::shared_mutex> lk(*data_device->mutex_map_text_internal);
            for (auto it_text_internal = data_text->map_text_internal.begin(); it_text_internal != data_text->map_text_internal.end(); )
            {
                pst_text_internal data_text_internal = it_text_internal->second;

                if (data_text_internal->flag_deleted == true
                    && data_text_internal->flag_created == true)
                {
                    delete_text_instance(data_text_internal->index_text_internal, data_text);
                    it_text_internal = data_text->map_text_internal.erase(it_text_internal);
                }
                else
                {
                    it_text_internal++;
                }
            }
        }

        {
            std::unique_lock<std::shared_mutex> lk(*data_device->mutex_map_text_internal);
            for (auto it_text_internal = data_text->map_text_internal.begin(); it_text_internal != data_text->map_text_internal.end(); it_text_internal++)
            {
                pst_text_internal data_text_internal = it_text_internal->second;

                if (data_text_internal->flag_created == false)
                {
                    create_text_instance(data_text_internal->index_text_internal, data_text);
                }
            }
        }
        
        data_device->device_11_on_12->AcquireWrappedResources(&data_rtv->vector_wrapped_back_buffer.at(backbuffer_index), 1);

        data_device->device_context_2d->SetTarget(data_rtv->vector_rtv_2d.at(backbuffer_index));
        data_device->device_context_2d->BeginDraw();

        {
            std::shared_lock<std::shared_mutex> lk(*data_device->mutex_map_text_internal);
            for (auto it_text_internal = data_text->map_text_internal.begin(); it_text_internal != data_text->map_text_internal.end(); it_text_internal++)
            {
                pst_text_internal data_text_internal = it_text_internal->second;

                if (data_text_internal->flag_created == false)
                {
                    continue;
                }

                if (data_text_internal->flag_deleted == true)
                {
                    continue;
                }

                bool flag_draw_text = false;

                if (*data_text_internal->text_start_coordinate_left + *data_text_internal->movement_translation_horizontal_background < data_device->rect_connected.right
                    && *data_text_internal->text_start_coordinate_top + *data_text_internal->movement_translation_vertical_background < data_device->rect_connected.bottom
                    && data_text_internal->text_matrics->layoutWidth + *data_text_internal->text_start_coordinate_left + *data_text_internal->movement_translation_horizontal_background > data_device->rect_connected.left
                    && data_text_internal->text_matrics->layoutHeight + *data_text_internal->text_start_coordinate_top + *data_text_internal->movement_translation_vertical_background > data_device->rect_connected.top
                    )
                {
                    flag_draw_text = true;
                }

                if (flag_draw_text == true)
                {
                    D2D1_RECT_F rect_background = D2D1::RectF(*data_text_internal->text_start_coordinate_left + *data_text_internal->movement_translation_horizontal_background - data_device->rect_connected.left,
                        *data_text_internal->text_start_coordinate_top + *data_text_internal->movement_translation_vertical_background - data_device->rect_connected.top,
                        data_text_internal->text_matrics->layoutWidth + *data_text_internal->text_start_coordinate_left + *data_text_internal->movement_translation_horizontal_background - data_device->rect_connected.left,
                        data_text_internal->text_matrics->layoutHeight + *data_text_internal->text_start_coordinate_top + *data_text_internal->movement_translation_vertical_background - data_device->rect_connected.top);

                    if (data_text_internal->text_color_background->r != 0 
                        || data_text_internal->text_color_background->g != 0
                        || data_text_internal->text_color_background->b != 0
                        || data_text_internal->text_color_background->a != 0)
                    {
                        data_device->device_context_2d->FillRectangle(rect_background, data_text_internal->text_brush_background);
                    }

                    if (!(data_text_internal->flag_blink == true
                        && data_text->flag_blink_status == true))
                    {
                        data_device->device_context_2d->DrawTextLayout(
                            D2D1::Point2F(*data_text_internal->movement_translation_horizontal + *data_text_internal->text_start_coordinate_left + *data_text_internal->movement_translation_horizontal_background - data_device->rect_connected.left,
                                *data_text_internal->movement_translation_vertical + *data_text_internal->text_start_coordinate_top + *data_text_internal->movement_translation_vertical_background - data_device->rect_connected.top),
                            data_text_internal->text_layout,
                            data_text_internal->text_brush
                        );
                    }
                }

                if (*data_text_internal->movement_type_horizontal == e_movement_type_horizontal::left)
                {
                    *data_text_internal->movement_translation_horizontal -= (*data_text_internal->movement_speed_horizontal / PRESENT_COUNT_PER_SECOND);
                    if (*data_text_internal->movement_type_horizontal_background == e_movement_type_horizontal::left)
                    {
                        *data_text_internal->movement_translation_horizontal -= (*data_text_internal->movement_speed_horizontal_background / PRESENT_COUNT_PER_SECOND);
                    }
                    else if (*data_text_internal->movement_type_horizontal_background == e_movement_type_horizontal::right)
                    {
                        *data_text_internal->movement_translation_horizontal -= (*data_text_internal->movement_speed_horizontal_background / PRESENT_COUNT_PER_SECOND);
                    }

                    if (*data_text_internal->movement_threshold_horizontal <= 0)
                    {
                        if (*data_text_internal->text_start_coordinate_left + *data_text_internal->movement_translation_horizontal + data_text_internal->text_matrics->width < *data_text_internal->text_start_coordinate_left + *data_text_internal->movement_threshold_horizontal)
                        {
                            *data_text_internal->movement_translation_horizontal = data_text_internal->text_matrics->layoutWidth;
                        }
                    }
                    else
                    {
                        if (*data_text_internal->text_start_coordinate_left + *data_text_internal->movement_translation_horizontal < *data_text_internal->text_start_coordinate_left + *data_text_internal->movement_threshold_horizontal)
                        {
                            *data_text_internal->movement_translation_horizontal = data_text_internal->text_matrics->layoutWidth;
                        }
                    }
                }
                else if (*data_text_internal->movement_type_horizontal == e_movement_type_horizontal::right)
                {
                    *data_text_internal->movement_translation_horizontal += (*data_text_internal->movement_speed_horizontal / PRESENT_COUNT_PER_SECOND);
                    if (*data_text_internal->movement_type_horizontal_background == e_movement_type_horizontal::left)
                    {
                        *data_text_internal->movement_translation_horizontal += (*data_text_internal->movement_speed_horizontal_background / PRESENT_COUNT_PER_SECOND);
                    }
                    else if (*data_text_internal->movement_type_horizontal_background == e_movement_type_horizontal::right)
                    {
                        *data_text_internal->movement_translation_horizontal += (*data_text_internal->movement_speed_horizontal_background / PRESENT_COUNT_PER_SECOND);
                    }

                    if (*data_text_internal->movement_translation_horizontal <= 0)
                    {
                        if (*data_text_internal->text_start_coordinate_left + *data_text_internal->movement_translation_horizontal > *data_text_internal->text_start_coordinate_left + data_text_internal->text_matrics->layoutWidth - *data_text_internal->movement_threshold_horizontal)
                        {
                            *data_text_internal->movement_translation_horizontal = DEFAULT_TEXT_MOVEMENT_TRANSLATION;
                        }
                    }
                    else
                    {
                        if (*data_text_internal->text_start_coordinate_left + *data_text_internal->movement_translation_horizontal + data_text_internal->text_matrics->width > *data_text_internal->text_start_coordinate_left + data_text_internal->text_matrics->layoutWidth - *data_text_internal->movement_threshold_horizontal)
                        {
                            *data_text_internal->movement_translation_horizontal = DEFAULT_TEXT_MOVEMENT_TRANSLATION;
                        }
                    }
                }
                if (*data_text_internal->movement_type_vertical == e_movement_type_vertical::top)
                {
                    *data_text_internal->movement_translation_vertical -= (*data_text_internal->movement_speed_vertical / PRESENT_COUNT_PER_SECOND);
                    if (*data_text_internal->movement_type_vertical_background == e_movement_type_vertical::top)
                    {
                        *data_text_internal->movement_translation_vertical -= (*data_text_internal->movement_speed_vertical_background / PRESENT_COUNT_PER_SECOND);
                    }
                    else if (*data_text_internal->movement_type_vertical_background == e_movement_type_vertical::bottom)
                    {
                        *data_text_internal->movement_translation_vertical -= (*data_text_internal->movement_speed_vertical_background / PRESENT_COUNT_PER_SECOND);
                    }

                    if (*data_text_internal->movement_threshold_vertical <= 0)
                    {
                        if (*data_text_internal->text_start_coordinate_top + *data_text_internal->movement_translation_vertical + data_text_internal->text_matrics->height < *data_text_internal->text_start_coordinate_top + *data_text_internal->movement_threshold_vertical)
                        {
                            *data_text_internal->movement_translation_vertical = data_text_internal->text_matrics->layoutHeight;
                        }
                    }
                    else
                    {
                        if (*data_text_internal->text_start_coordinate_top + *data_text_internal->movement_translation_vertical < *data_text_internal->text_start_coordinate_top + *data_text_internal->movement_threshold_vertical)
                        {
                            *data_text_internal->movement_translation_vertical = data_text_internal->text_matrics->layoutHeight;
                        }
                    }
                }
                else if (*data_text_internal->movement_type_vertical == e_movement_type_vertical::bottom)
                {
                    *data_text_internal->movement_translation_vertical += (*data_text_internal->movement_speed_vertical / PRESENT_COUNT_PER_SECOND);
                    if (*data_text_internal->movement_type_vertical_background == e_movement_type_vertical::top)
                    {
                        *data_text_internal->movement_translation_vertical += (*data_text_internal->movement_speed_vertical_background / PRESENT_COUNT_PER_SECOND);
                    }
                    else if (*data_text_internal->movement_type_vertical_background == e_movement_type_vertical::bottom)
                    {
                        *data_text_internal->movement_translation_vertical += (*data_text_internal->movement_speed_vertical_background / PRESENT_COUNT_PER_SECOND);
                    }

                    if (*data_text_internal->movement_threshold_vertical <= 0)
                    {
                        if (*data_text_internal->text_start_coordinate_top + *data_text_internal->movement_translation_vertical > *data_text_internal->text_start_coordinate_top + data_text_internal->text_matrics->layoutHeight - *data_text_internal->movement_threshold_vertical)
                        {
                            *data_text_internal->movement_translation_vertical = DEFAULT_TEXT_MOVEMENT_TRANSLATION;
                        }
                    }
                    else
                    {
                        if (*data_text_internal->text_start_coordinate_top + *data_text_internal->movement_translation_vertical + data_text_internal->text_matrics->height > *data_text_internal->text_start_coordinate_top + data_text_internal->text_matrics->layoutHeight - *data_text_internal->movement_threshold_vertical)
                        {
                            *data_text_internal->movement_translation_vertical = DEFAULT_TEXT_MOVEMENT_TRANSLATION;
                        }
                    }
                }
                
                if (*data_text_internal->movement_type_horizontal_background == e_movement_type_horizontal::left)
                {
                    *data_text_internal->movement_translation_horizontal_background -= (*data_text_internal->movement_speed_horizontal_background / PRESENT_COUNT_PER_SECOND);
                    if (*data_text_internal->text_start_coordinate_left + *data_text_internal->movement_translation_horizontal_background < *data_text_internal->movement_threshold_horizontal_background)
                    {
                        *data_text_internal->movement_translation_horizontal_background = DEFAULT_TEXT_MOVEMENT_TRANSLATION;
                    }
                }
                else if (*data_text_internal->movement_type_horizontal_background == e_movement_type_horizontal::right)
                {
                    *data_text_internal->movement_translation_horizontal_background += (*data_text_internal->movement_speed_horizontal_background / PRESENT_COUNT_PER_SECOND);
                    if (data_text_internal->text_matrics->layoutWidth + *data_text_internal->text_start_coordinate_left + *data_text_internal->movement_translation_horizontal_background > *data_text_internal->movement_threshold_horizontal_background)
                    {
                        *data_text_internal->movement_translation_horizontal_background = DEFAULT_TEXT_MOVEMENT_TRANSLATION;
                    }
                }
                if (*data_text_internal->movement_type_vertical_background == e_movement_type_vertical::top)
                {
                    *data_text_internal->movement_translation_vertical_background -= (*data_text_internal->movement_speed_vertical_background / PRESENT_COUNT_PER_SECOND);
                    if (*data_text_internal->text_start_coordinate_top + *data_text_internal->movement_translation_vertical_background < *data_text_internal->movement_threshold_vertical_background)
                    {
                        *data_text_internal->movement_translation_vertical_background = DEFAULT_TEXT_MOVEMENT_TRANSLATION;
                    }
                }
                else if (*data_text_internal->movement_type_vertical_background == e_movement_type_vertical::bottom)
                {
                    *data_text_internal->movement_translation_vertical_background += (*data_text_internal->movement_speed_vertical_background / PRESENT_COUNT_PER_SECOND);
                    if (data_text_internal->text_matrics->layoutHeight + *data_text_internal->text_start_coordinate_top + *data_text_internal->movement_translation_vertical_background > *data_text_internal->movement_threshold_vertical_background)
                    {
                        *data_text_internal->movement_translation_vertical_background = DEFAULT_TEXT_MOVEMENT_TRANSLATION;
                    }
                }
            }
        }

        data_device->device_context_2d->EndDraw();

        data_device->device_11_on_12->ReleaseWrappedResources(&data_rtv->vector_wrapped_back_buffer.at(backbuffer_index), 1);
        data_device->device_context_11->Flush();
#pragma endregion
        // ----------------------------------------------------------------

        data_fence->fence_value_device++;
        if (data_fence->fence_value_device == UINT64_MAX)
        {
            data_fence->fence_value_device = 0;
        }
        data_fence->fence_device->SetEventOnCompletion(data_fence->fence_value_device, data_fence->fence_event_device);
        command_queue->Signal(data_fence->fence_device, data_fence->fence_value_device);

        WaitForSingleObject(data_fence->fence_event_device, INFINITE);

        SetEvent(data_device->event_device_to_window);

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_device, device index = ");
            str.append(std::to_string(data_device->device_index));
            str.append(", set event, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get("wplayer_logger");
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

    bool flag_buffer_created = false;

    int index_command_allocator_upload = _count_command_allocator - 1;
    int srv_index = -1;

    std::vector<pst_scene> vector_scene;

    bool flag_vector_ready = false;

    int index_command_list_upload = _count_command_list - 1;

    while (data_device->flag_thread_upload)
    {
        // flag_ready_to_frame_use false - index to 0

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

            auto logger = spdlog::get("wplayer_logger");
            logger->debug(str.c_str());
        }

        if (flag_vector_ready == false)
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

            flag_vector_ready = true;
        }

        if (data_device->flag_ready_to_device_use == false)
        {
            for (auto it = vector_scene.begin(); it != vector_scene.end(); it++)
            {
                pst_scene data_scene = *it;

                data_scene->flag_frame_unref = false;
            }
        }

        index_command_allocator_upload += 1;
        if (!(index_command_allocator_upload < _count_command_allocator * 2))
        {
            index_command_allocator_upload = _count_command_allocator;
        }

        index_command_list_upload += 1;
        if (!(index_command_list_upload < _count_command_list * 2))
        {
            index_command_list_upload = _count_command_list;
        }

        if (data_device->flag_ready_to_device_use)
        {
            if (data_device->flag_use_last_frame == false)
            {
                srv_index += 1;
                if (!(srv_index < _count_texture_store))
                {
                    srv_index = 0;
                }

                for (auto it = vector_scene.begin(); it != vector_scene.end(); it++)
                {
                    pst_scene data_scene = *it;

                    data_scene->flag_frame_unref = true;

                    data_scene->mutex_deque_index_used->lock();
                    data_scene->deque_index_used.push_back(srv_index);
                    data_scene->mutex_deque_index_used->unlock();
                }
            }
        }

        if (!data_device->flag_thread_upload)
        {
            break;
        }

        ID3D12CommandAllocator* command_allocator = data_command_allocator->vector_command_allocator.at(index_command_allocator_upload);
        ID3D12GraphicsCommandList* command_list = data_command_list->vector_command_list.at(index_command_list_upload);
        ID3D12PipelineState* pso = data_pso->pso;

        command_allocator->Reset();
        command_list->Reset(command_allocator, pso);

        // Device�� �޸� �Ҵ�
        if (flag_buffer_created == false)
        {
            create_vertex_buffer(data_device, index_command_list_upload);
            create_index_buffer(data_device, index_command_list_upload);

            create_srv_handles_texture_default(data_device);

            int counter_scene = vector_scene.size();
            create_srv_handles(data_device, counter_scene);

            flag_buffer_created = true;

            create_texture_default(data_device);

            upload_texture_default(data_device, index_command_list_upload);
        }

        // upload cef texture buffer data
        if (_flag_use_cef)
        {
            for (auto it_cef = _map_cef.begin(); it_cef != _map_cef.end(); it_cef++)
            {
                pst_cef data_cef = it_cef->second;
            
                for (auto it_cef_device = data_cef->map_cef_device.begin(); it_cef_device != data_cef->map_cef_device.end(); it_cef_device++)
                {
                    pst_cef_device data_cef_device = it_cef_device->second;

                    if (data_cef_device->index_cef_device != data_device->device_index)
                    {
                        continue;
                    }

                    int counter_scene = vector_scene.size();

                    if (is_queue_empty(data_cef->index_map_input, data_cef->index_map_output) == false)
                    {
                        if (data_cef_device->flag_texture_created == false)
                        {
                            create_vertex_buffer_cef(data_device, index_command_list_upload, counter_scene + data_cef_device->count_cef_device, data_cef_device->rect_normalized);

                            auto it_cef_data_paint = data_cef->map_cef_data_paint->find(data_cef_device->index_srv_upload);
                            pst_cef_data_paint data_cef_data_paint = it_cef_data_paint->second;

                            create_nv12_texture(data_device, data_cef_data_paint->width, data_cef_data_paint->height, data_cef->index_cef);

                            create_srv_handles_cef_paint(data_device, counter_scene, data_cef_device->count_cef_device);

                            data_cef_device->flag_texture_created = true;
                        }

                        if (data_cef->flag_use_last_frame == false)
                        {
                            data_cef->index_upload_texture++;
                            if (data_cef->index_upload_texture >= _count_texture_store)
                            {
                                data_cef->index_upload_texture = 0;
                            }
                        }

                        upload_texture_cef(data_device, data_cef, index_command_list_upload, counter_scene + data_cef_device->count_cef_device);

                        data_cef_device->index_srv_upload++;
                        if (data_cef_device->index_srv_upload >= _count_texture_store)
                        {
                            data_cef_device->index_srv_upload = 0;
                        }
                    }
                }
            }
        }

        if (data_device->flag_ready_to_device_use)
        {
            int counter_texture = 0;
            for (auto it_vector = vector_scene.begin(); it_vector != vector_scene.end(); it_vector++)
            {
                pst_scene data_scene = *it_vector;
                upload_texture(data_device, data_scene->vector_frame.at(srv_index), counter_texture, srv_index);

                counter_texture++;
            }
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

            auto logger = spdlog::get("wplayer_logger");
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

            auto logger = spdlog::get("wplayer_logger");
            logger->debug(str.c_str());
        }

        if (data_window->flag_thread_window == false)
        {
            break;
        }

        if (_nvapi_initialized && _flag_use_default_image == false)
        {
            NvAPI_Status status = NvAPI_D3D1x_Present(device, swap_chain, (UINT)1, (UINT)0);
        }
        else
        {
            swap_chain->Present(1, 0);
        }

        // get cef texture buffer data
        if (_flag_use_cef)
        {
            _mutex_deque_cef_delete.lock();
            for (auto it_index_cef = _deque_cef_delete.begin(); it_index_cef != _deque_cef_delete.end();)
            {
                int index_cef = *it_index_cef;

                auto it_cef = _map_cef.find(index_cef);
                if (it_cef != _map_cef.end())
                {
                    pst_cef data_cef = it_cef->second;

                    delete_cef_internal(data_cef);

                    _map_cef.erase(it_cef);
                }

                it_index_cef = _deque_cef_delete.erase(it_index_cef);
            }
            _mutex_deque_cef_delete.unlock();

            for (auto it_cef = _map_cef.begin(); it_cef != _map_cef.end(); it_cef++)
            {
                pst_cef data_cef = it_cef->second;

                if (data_cef->flag_use_last_frame == false)
                {
                    data_cef->mutex_deque_index_used->lock();

                    // cef texture, use ���� üũ �� delete �� �̵�
                    // deque�� ���ڰ� ���� �����ϸ� unref ���� ����.
                    // ���ڰ� ���� �����ϸ� 2���� ���⵵�� ��.
                    if (data_cef->deque_index_used->size() > data_cef->count_used_frame_store)
                    {
                        int count_used_index = data_cef->deque_index_used->size();
                        int check_same_index = data_cef->deque_index_used->front();
                        int counter_same_index = 0;

                        for (auto it = data_cef->deque_index_used->begin(); it != data_cef->deque_index_used->end(); it++)
                        {
                            if (check_same_index == *it)
                            {
                                count_used_index--;
                                counter_same_index++;
                            }
                            else
                            {
                                break;
                            }
                        }

                        //if (counter_same_index > data_cef->count_used_frame_store)
                        if (counter_same_index > 1)
                        {
                            //for (size_t i = 0; i < counter_same_index - data_cef->count_used_frame_store; i++)
                            for (size_t i = 0; i < counter_same_index - 1; i++)
                            {
                                data_cef->deque_index_used->pop_front();
                            }
                        }

                        if (count_used_index != 0)
                        {
                            int index_used_frame = data_cef->deque_index_used->front();

                            {
                                std::lock_guard<std::mutex> lk(*data_cef->mutex_deque_index_unref);
                                data_cef->deque_index_unref->push_back(index_used_frame);
                            }

                            data_cef->deque_index_used->pop_front();
                        }
                    }

                    data_cef->mutex_deque_index_used->unlock();
                }

                data_cef->flag_use_last_frame = false;

                int size = 0;
                cpp_cef_wrapper_get_deque_size(data_cef->instance, size);

                void* buffer = nullptr;
                int width = 0;
                int height = 0;

                if (size != 0)
                {
                    if (is_queue_full(data_cef->index_map_input, data_cef->index_map_output, _count_texture_store) == false)
                    {
                        cpp_cef_wrapper_get_deque_data(data_cef->instance, buffer, width, height);
                        if (buffer != nullptr && width != 0 && height != 0)
                        {
                            auto it_map_cef_data_paint = data_cef->map_cef_data_paint->find(data_cef->index_map_input);
                            if (it_map_cef_data_paint != data_cef->map_cef_data_paint->end())
                            {
                                pst_cef_data_paint data = it_map_cef_data_paint->second;

                                data->buffer = buffer;
                                data->width = width;
                                data->height = height;

                                data_cef->index_map_input++;
                                if (data_cef->index_map_input == _count_texture_store)
                                {
                                    data_cef->index_map_input = 0;
                                }
                            }
                        }
                        else
                        {
                            data_cef->flag_use_last_frame = true;
                        }
                    }
                    else
                    {
                        data_cef->flag_use_last_frame = true;
                    }
                }
                else
                {
                    data_cef->flag_use_last_frame = true;
                }
            }
        }

        SetEvent(data_window->event_window_to_scene);

        if (_flag_set_logger)
        {
            std::string str = "";
            str.append("thread_window, device index = ");
            str.append(std::to_string(data_device->device_index));
            str.append(", set event, ");
            str.append(std::to_string(av_gettime_relative()));

            auto logger = spdlog::get("wplayer_logger");
            logger->debug(str.c_str());
        }
    }
}

void thread_scene(pst_scene data_scene)
{
    int32_t result = INT32_MIN;

    int64_t counter_scene_fps = -1;

    void* ffmpeg_instance_current = nullptr;
    auto it_ffmpeg_instance_current = data_scene->map_ffmpeg_instance.find(data_scene->index_ffmpeg_instance_current);
    if (it_ffmpeg_instance_current != data_scene->map_ffmpeg_instance.end())
    {
        ffmpeg_instance_current = it_ffmpeg_instance_current->second;
    }
    
    bool flag_first = true;

    bool flag_repeat = false;

    int count_use_last_frame = 0;

    AVFrame* frame = nullptr;
    int index_frame_check_delay = 0;

    bool flag_is_realtime = false;
    if (ffmpeg_instance_current != nullptr)
    {
        cpp_ffmpeg_wrapper_get_is_realtime(ffmpeg_instance_current, flag_is_realtime);
    }

    bool flag_succeed_open_input = false;
    if (ffmpeg_instance_current != nullptr)
    {
        cpp_ffmpeg_wrapper_get_flag_succeed_open_input(ffmpeg_instance_current, flag_succeed_open_input);
    }

    bool flag_play_started = false;
    if (ffmpeg_instance_current != nullptr)
    {
        cpp_ffmpeg_wrapper_get_flag_play_started(ffmpeg_instance_current, flag_play_started);
    }

    while (data_scene->flag_thread_scene)
    {
        if (_flag_use_default_image == false)
        {
            // ����� ����� frame�� unref ���� queue�� ���
            //if (data_scene->flag_frame_unref == true && data_scene->flag_use_last_frame == false)
            if (data_scene->flag_use_last_frame == false)
            {
                data_scene->mutex_deque_index_used->lock();
            
                // deque�� ���ڰ� ���� �����ϸ� unref ���� ����.
                // ���ڰ� ���� �����ϸ� 2���� ���⵵�� ��.
                if (data_scene->deque_index_used.size() > data_scene->count_used_frame_store)
                {
                    int count_used_index = data_scene->deque_index_used.size();
                    int check_same_index = data_scene->deque_index_used.front();
                    int counter_same_index = 0;

                    for (auto it = data_scene->deque_index_used.begin(); it != data_scene->deque_index_used.end(); it++)
                    {
                        if (check_same_index == *it)
                        {
                            count_used_index--;
                            counter_same_index++;
                        }
                        else
                        {
                            break;
                        }
                    }

                    if (counter_same_index > data_scene->count_used_frame_store)
                    {
                        for (size_t i = 0; i < counter_same_index - data_scene->count_used_frame_store; i++)
                        {
                            data_scene->deque_index_used.pop_front();
                        }
                    }

                    if (count_used_index != 0)
                    {
                        int index_used_frame = data_scene->deque_index_used.front();

                        {
                            std::lock_guard<std::mutex> lk(*data_scene->mutex_deque_index_unref);
                            data_scene->deque_index_unref.push_back(index_used_frame);
                        }

                        data_scene->deque_index_used.pop_front();
                    }
                }

                data_scene->mutex_deque_index_used->unlock();
            }

            // last frame ��� �÷��� �ʱ�ȭ
            if (data_scene->flag_use_last_frame == true)
            {
                data_scene->flag_use_last_frame = false;
            }

            if (ffmpeg_instance_current != nullptr
                && flag_is_realtime == true
                && flag_succeed_open_input == false)
            {
                create_ffmpeg_instance_check_open_file_on_live_stream(ffmpeg_instance_current);
                cpp_ffmpeg_wrapper_get_flag_succeed_open_input(ffmpeg_instance_current, flag_succeed_open_input);
            }

            if (ffmpeg_instance_current != nullptr
                && flag_is_realtime == true
                && flag_succeed_open_input == true
                && flag_play_started == false)
            {
                create_ffmpeg_instance_play_start_on_live_stream(ffmpeg_instance_current);
                cpp_ffmpeg_wrapper_get_flag_play_started(ffmpeg_instance_current, flag_play_started);
            }

            // vector_frame is not full - is_queue_full() == false
            if (is_queue_full(data_scene->index_input, data_scene->index_output, _count_texture_store) == false
                && flag_succeed_open_input
                && flag_play_started)
            {
                // CppFFmpegWrapper get_frame
                //
                // return == -1 : continue
                // return >=  0 : ok
                // return == -2 : EOS, repeat
                result = cpp_ffmpeg_wrapper_get_frame(ffmpeg_instance_current, data_scene->vector_frame.at(data_scene->frame_index));

                // queue empty
                if (result == -1)
                {
                    data_scene->flag_use_last_frame = true;

                    if (flag_first == true)
                    {
                        continue;
                    }
                }
                // EOS / EOF
                else if (result == -2)
                {
                    // ����ϴ� ffmpeg instance�� index ����

                    if (flag_is_realtime == false)
                    {
                        if (flag_repeat == false)
                        {
                            data_scene->index_ffmpeg_instance_current++;
                            if (data_scene->index_ffmpeg_instance_current == data_scene->map_ffmpeg_instance_capacity)
                            {
                                data_scene->index_ffmpeg_instance_current = 0;
                            }

                            if (data_scene->index_ffmpeg_instance_current == 0)
                            {
                                data_scene->index_ffmpeg_instance_last = data_scene->map_ffmpeg_instance_capacity - 1;
                            }
                            else
                            {
                                data_scene->index_ffmpeg_instance_last = data_scene->index_ffmpeg_instance_current - 1;
                            }

                            if (data_scene->index_ffmpeg_instance_last == 0)
                            {
                                data_scene->index_ffmpeg_instance_delete = data_scene->map_ffmpeg_instance_capacity - 1;
                            }
                            else
                            {
                                data_scene->index_ffmpeg_instance_delete = data_scene->index_ffmpeg_instance_last - 1;
                            }
                
                            it_ffmpeg_instance_current = data_scene->map_ffmpeg_instance.find(data_scene->index_ffmpeg_instance_current);
                            ffmpeg_instance_current = it_ffmpeg_instance_current->second;

                            check_map_ffmpeg_instance_repeat(data_scene);

                            flag_repeat = true;

                            count_use_last_frame = _count_use_last_frame_at_repeat;
                        }
                    }
                    else
                    {
                        if (flag_repeat == false)
                        {
                            cpp_ffmpeg_wrapper_play_stop(ffmpeg_instance_current, nullptr);
                            cpp_ffmpeg_wrapper_shutdown_small(ffmpeg_instance_current);
                            cpp_ffmpeg_wrapper_initialize_small(ffmpeg_instance_current, callback_ffmpeg_wrapper_ptr);
                            cpp_ffmpeg_wrapper_get_flag_succeed_open_input(ffmpeg_instance_current, flag_succeed_open_input);
                            cpp_ffmpeg_wrapper_get_flag_play_started(ffmpeg_instance_current, flag_play_started);

                            flag_repeat = true;
                        }
                    }

                    data_scene->flag_use_last_frame = true;
                }
                // return >= 0
                // success
                else
                {
                    if (flag_first == true)
                    {
                        flag_first = false;
                    }

                    data_scene->frame_index += 1;
                    if (!(data_scene->frame_index < _count_texture_store))
                    {
                        data_scene->frame_index = 0;
                    }

                    data_scene->index_input += 1;
                    if (!(data_scene->index_input < _count_texture_store))
                    {
                        data_scene->index_input = 0;
                    }

                    data_scene->flag_ready_to_frame_use = true;

                    if (flag_repeat == true)
                    {
                        flag_repeat = false;
                    }

                    do
                    {
                        // CppFFmpegWrapper frame_to_next
                        result = cpp_ffmpeg_wrapper_frame_to_next_non_waiting(ffmpeg_instance_current);
                    } while (result != 0);
                }
            }

            if (flag_play_started)
            {
                frame = data_scene->vector_frame.at(index_frame_check_delay);
                if (frame->data[0] != nullptr)
                {
                    // ��� �ӵ� ���� ����

                    data_scene->time_now = av_gettime_relative();
                    AVRational timebase{};
                    cpp_ffmpeg_wrapper_get_timebase(ffmpeg_instance_current, timebase);
                    data_scene->pts_in_milliseconds_now = av_rescale_q(frame->pts, timebase, AVRational{ 1, 1'000'000 });

                    int64_t delay = data_scene->pts_in_milliseconds_now - data_scene->pts_in_milliseconds_last - (data_scene->time_now - data_scene->time_last);

                    if (delay < 14'000 || flag_is_realtime == true)
                    {
                        data_scene->time_last = data_scene->time_now;
                        data_scene->pts_in_milliseconds_last = data_scene->pts_in_milliseconds_now;

                        data_scene->flag_use_last_frame = false;

                        index_frame_check_delay += 1;
                        if (!(index_frame_check_delay < _count_texture_store))
                        {
                            index_frame_check_delay = 0;
                        }
                    }
                    else
                    {
                        data_scene->flag_use_last_frame = true;
                    }
                }

                if (count_use_last_frame > 0)
                {
                    count_use_last_frame--;

                    data_scene->flag_use_last_frame = true;
                }
            }
            else
            {
                data_scene->flag_use_last_frame = true;
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

            auto logger = spdlog::get("wplayer_logger");
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

            auto logger = spdlog::get("wplayer_logger");
            logger->debug(str.c_str());
        }
    }
}

void thread_scene_unref(pst_scene data_scene)
{
    while (data_scene->flag_thread_scene_unref)
    {
        bool flag_deque_index_unref_is_empty = false;
        {
            std::lock_guard<std::mutex> lk(*data_scene->mutex_deque_index_unref);
            flag_deque_index_unref_is_empty = data_scene->deque_index_unref.empty();
        }

        if (flag_deque_index_unref_is_empty)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_processing));
            continue;
        }

        int index_unref = -1;
        {
            std::lock_guard<std::mutex> lk(*data_scene->mutex_deque_index_unref);
            index_unref = data_scene->deque_index_unref.front();
            data_scene->deque_index_unref.pop_front();
        }

        av_frame_unref(data_scene->vector_frame.at(index_unref));

        if (is_queue_empty(data_scene->index_input, data_scene->index_output) == false)
        {
            data_scene->index_output += 1;
            if (!(data_scene->index_output < _count_texture_store))
            {
                data_scene->index_output = 0;
            }
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
    auto max_size = 1024 * 1024 * _log_file_size;
    auto max_files = _log_file_rotation_count;

    spdlog::init_thread_pool(8192, 1);
    //auto logger = spdlog::rotating_logger_mt<spdlog::async_factory>("wplayer_logger", "spdlogs/wplayer_log.txt", max_size, max_files);
    auto logger = spdlog::rotating_logger_mt("wplayer_logger", "spdlogs/wplayer_log.txt", max_size, max_files);

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

    hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

    // ���� ���ڿ��� �ʱ�ȭ�մϴ�.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WPLAYER, szWindowClass, MAX_LOADSTRING);
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
    
    _mutex_map_srv_handle_luminance = new std::mutex();
    _mutex_map_srv_handle_chrominance = new std::mutex();

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

    create_factory();
    enum_adapters();
    enum_outputs();

    if (_use_manual_window_create)
    {
        create_windows_manual();
    }
    else
    {
        create_windows();
    }
    delete_adapter_has_none_window();

    {
        create_devices();
        create_command_queues();
        create_command_allocators();
        create_device_11_on_12();

        // NV12
        create_root_sigs();
        
        create_pipeline_state_objects();
        create_command_lists();
        create_fences();
        create_rtv_heaps();
        create_srv_heaps();

        create_swap_chains();
        create_rtvs();
        create_viewports();

        create_texts();
    }

    // default texture �غ�
    {
        int result = 0;
        result = create_ffmpeg_instance_default_image(_ffmpeg_instance_default_image);
        if (result == 0)
        {
            _frame_default_image = av_frame_alloc();

            do
            {
                result = cpp_ffmpeg_wrapper_get_frame(_ffmpeg_instance_default_image, _frame_default_image);
            } while (result != 0);

            cpp_ffmpeg_wrapper_play_stop(_ffmpeg_instance_default_image, nullptr);
            cpp_ffmpeg_wrapper_shutdown(_ffmpeg_instance_default_image);
            cpp_ffmpeg_wrapper_delete(_ffmpeg_instance_default_image);
        }
    }

    // swap_lock
    if (_nvapi_initialized)
    {
        initialize_swap_locks();
    }

    if (_flag_use_cef)
    {
        cef_initialize();
    }

    start_playback();

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

    if (_flag_use_cef)
    {
        if (_flag_cef_succeed)
        {
            delete_cefs();
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

    for (auto it_thread_scene_unref = _map_thread_scene_unref.begin(); it_thread_scene_unref != _map_thread_scene_unref.end();)
    {
        std::thread* data_thread_scene_unref = it_thread_scene_unref->second;

        auto it_scene = _map_scene.find(it_thread_scene_unref->first);
        pst_scene data_scene = it_scene->second;

        if (data_thread_scene_unref->joinable())
        {
            data_scene->flag_thread_scene_unref = false;
            data_thread_scene_unref->join();
        }

        delete data_thread_scene_unref;
        data_thread_scene_unref = nullptr;

        it_thread_scene_unref = _map_thread_scene_unref.erase(it_thread_scene_unref);
    }

    wait_gpus_end();

    if (_thread_vector_input.joinable())
    {
        _flag_vector_input = false;
        _thread_vector_input.join();
    }

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

    if (_flag_use_cef)
    {
        if (_flag_cef_succeed)
        {
            cef_shutdown();
        }
    }

    delete_scenes();

    // NV12
    delete_textures();

    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        delete_texture_default(data_device);

        delete_nv12_texture(data_device);
    }
    delete_vertex_buffers();
    delete_index_buffers();

    // swap_lock
    if (_nvapi_initialized)
    {
        delete_swap_locks();
    }

    delete_texts();

    delete_viewports();
    delete_rtvs();
    delete_swap_chains();

    delete_srv_heaps();
    delete_rtv_heaps();
    delete_fences();
    delete_command_lists();
    delete_pipeline_state_objects();
    delete_root_sigs();
    delete_device_11_on_12();
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

    delete _mutex_map_srv_handle_luminance;
    _mutex_map_srv_handle_luminance = nullptr;
    delete _mutex_map_srv_handle_chrominance;
    _mutex_map_srv_handle_chrominance = nullptr;

    if (_frame_default_image != nullptr)
    {
        av_frame_free(&_frame_default_image);
    }

#if _DEBUG
    d3d_memory_check();
#endif

    return (int)msg.wParam;
}

int start_playback()
{
    int result = 0;

    result = create_scenes();
    if (result != 0)
    {
        delete_scenes_data();
        _flag_use_default_image = true;
    }

    // wait_for_multiple_objects ��������� ����
    _thread_wait_for_multiple_objects_scene_to_upload = std::thread(thread_wait_for_multiple_objects, e_wait_type::scene_to_upload, &_flag_wait_for_multiple_objects_scene_to_upload);
    _thread_wait_for_multiple_objects_upload_to_device = std::thread(thread_wait_for_multiple_objects, e_wait_type::upload_to_device, &_flag_wait_for_multiple_objects_upload_to_device);
    _thread_wait_for_multiple_objects_device_to_window = std::thread(thread_wait_for_multiple_objects, e_wait_type::device_to_window, &_flag_wait_for_multiple_objects_device_to_window);;
    _thread_wait_for_multiple_objects_window_to_scene = std::thread(thread_wait_for_multiple_objects, e_wait_type::window_to_scene, &_flag_wait_for_multiple_objects_window_to_scene);

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

        // NV12
        data_thread_device = new std::thread(thread_device, data_device);

        _map_thread_device.insert({ data_device->device_index, data_thread_device });
    }

    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        std::thread* data_thread_upload = nullptr;

        // NV12
        data_thread_upload = new std::thread(thread_upload, data_device);

        _map_thread_upload.insert({ data_device->device_index, data_thread_upload });
    }

    for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
    {
        pst_scene data_scene = it_scene->second;

        std::thread* data_thread_scene_unref = new std::thread(thread_scene_unref, data_scene);

        _map_thread_scene_unref.insert({ data_scene->scene_index, data_thread_scene_unref });
    }

    for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
    {
        pst_scene data_scene = it_scene->second;

        std::thread* data_thread_scene = new std::thread(thread_scene, data_scene);

        _map_thread_scene.insert({ data_scene->scene_index, data_thread_scene });
    }

    return result;
}

bool is_queue_full(int index_input, int index_output, int queue_size)
{
    return (index_output == 0 && index_input == queue_size - 1) || (index_output == index_input + 1);
}

bool is_queue_empty(int index_input, int index_output)
{
    return index_output == index_input;
}

int get_queue_size(int index_input, int index_output, int queue_size)
{
    if (is_queue_empty(index_input, index_output))
    {
        return 0;
    }
    else if (index_output <= index_input)
    {
        return index_input - index_output + 1;
    }
    else
    {
        return queue_size - index_output + index_input + 1;
    }
}

int create_ffmpeg_instance_with_scene_index(void*& instance, UINT device_index, std::string url, UINT scene_index, RECT rect)
{
    instance = cpp_ffmpeg_wrapper_create();
    cpp_ffmpeg_wrapper_initialize(instance, callback_ffmpeg_wrapper_ptr);

    // NV12
    cpp_ffmpeg_wrapper_set_hw_decode(instance);
    cpp_ffmpeg_wrapper_set_hw_device_type(instance, _hw_device_type);
    cpp_ffmpeg_wrapper_set_hw_decode_adapter_index(instance, device_index);

    cpp_ffmpeg_wrapper_set_scale(instance, false);

    cpp_ffmpeg_wrapper_set_file_path(instance, (char*)url.c_str());
    if (cpp_ffmpeg_wrapper_open_file(instance) != 0)
    {
        cpp_ffmpeg_wrapper_shutdown(instance);
        cpp_ffmpeg_wrapper_delete(instance);

        return 1;
    }

    cpp_ffmpeg_wrapper_set_scene_index(instance, scene_index);
    cpp_ffmpeg_wrapper_set_rect(instance, rect);
    cpp_ffmpeg_wrapper_play_start(instance, nullptr);

    return 0;
}

int check_map_ffmpeg_instance_repeat(pst_scene data_scene)
{
    void* ffmpeg_instance_delete = nullptr;

    auto it_ffmpeg_instance_delete = data_scene->map_ffmpeg_instance.find(data_scene->index_ffmpeg_instance_delete);
    if (it_ffmpeg_instance_delete->second != nullptr)
    {
        ffmpeg_instance_delete = it_ffmpeg_instance_delete->second;
    }

    if (ffmpeg_instance_delete != nullptr)
    {
        cpp_ffmpeg_wrapper_start_thread_repeat(ffmpeg_instance_delete);
    }

    return 0;
}

int create_ffmpeg_instance_check_open_file(void* instance)
{
    if (cpp_ffmpeg_wrapper_open_file(instance) != 0)
    {
        cpp_ffmpeg_wrapper_shutdown(instance);
        cpp_ffmpeg_wrapper_delete(instance);

        return 1;
    }

    return 0;
}

int create_ffmpeg_instance_play_start(void* instance, UINT& index_scene, RECT rect)
{
    index_scene = _next_scene_index;
    _next_scene_index++;

    cpp_ffmpeg_wrapper_set_scene_index(instance, index_scene);
    cpp_ffmpeg_wrapper_set_rect(instance, rect);
    cpp_ffmpeg_wrapper_play_start(instance, nullptr);

    return 0;
}

int create_texture_default(pst_device data_device)
{
    HRESULT hr = S_OK;

    ID3D12Device* device = data_device->device;

    D3D12_RESOURCE_DESC desc_texture{};
    desc_texture.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc_texture.Alignment = 0;

    if (_frame_default_image != nullptr)
    {
        desc_texture.Width = _frame_default_image->width;
        desc_texture.Height = _frame_default_image->height;
    }
    else
    {
        desc_texture.Width = 640;
        desc_texture.Height = 480;
    }

    desc_texture.Width = (desc_texture.Width + 1) & ~1;  // �ʺ� ¦���� ����
    desc_texture.Height = (desc_texture.Height + 1) & ~1; // ���̸� ¦���� ����

    desc_texture.DepthOrArraySize = 1;
    desc_texture.MipLevels = 1;
    desc_texture.Format = DXGI_FORMAT_NV12;
    desc_texture.SampleDesc.Count = 1;
    desc_texture.SampleDesc.Quality = 0;
    desc_texture.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc_texture.Flags = D3D12_RESOURCE_FLAG_NONE;

    CD3DX12_HEAP_PROPERTIES texture_default_properties(D3D12_HEAP_TYPE_DEFAULT);

    hr = device->CreateCommittedResource(
        &texture_default_properties,
        D3D12_HEAP_FLAG_NONE,
        &desc_texture,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&data_device->texture_default)
    );

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout_luminance{};
    UINT numRows_luminance = 0;
    UINT64 rowSizeInBytes_luminance = 0;
    UINT64 totalBytes_luminance = 0;

    device->GetCopyableFootprints(&desc_texture, 0, 1, 0, &layout_luminance, &numRows_luminance, &rowSizeInBytes_luminance, &totalBytes_luminance);

    D3D12_RESOURCE_DESC desc_upload{};
    desc_upload.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc_upload.Alignment = 0;
    desc_upload.Width = totalBytes_luminance;
    desc_upload.Height = 1;
    desc_upload.DepthOrArraySize = 1;
    desc_upload.MipLevels = 1;
    desc_upload.Format = DXGI_FORMAT_UNKNOWN;
    desc_upload.SampleDesc.Count = 1;
    desc_upload.SampleDesc.Quality = 0;
    desc_upload.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc_upload.Flags = D3D12_RESOURCE_FLAG_NONE;

    CD3DX12_HEAP_PROPERTIES texture_default_upload_properties(D3D12_HEAP_TYPE_UPLOAD);

    hr = device->CreateCommittedResource(
        &texture_default_upload_properties,
        D3D12_HEAP_FLAG_NONE,
        &desc_upload,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&data_device->upload_heap_texture_default_luminance)
    );

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout_chrominance{};
    UINT numRows_chrominance = 0;
    UINT64 rowSizeInBytes_chrominance = 0;
    UINT64 totalBytes_chrominance = 0;

    device->GetCopyableFootprints(&desc_texture, 1, 1, 0, &layout_chrominance, &numRows_chrominance, &rowSizeInBytes_chrominance, &totalBytes_chrominance);

    desc_upload.Width = totalBytes_chrominance;

    hr = device->CreateCommittedResource(
        &texture_default_upload_properties,
        D3D12_HEAP_FLAG_NONE,
        &desc_upload,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&data_device->upload_heap_texture_default_chrominance)
    );

    return 0;
}

int delete_texture_default(pst_device data_device)
{
    if (data_device->texture_default != nullptr)
    {
        data_device->texture_default->Release();
        data_device->texture_default = nullptr;
    }

    if (data_device->upload_heap_texture_default_luminance != nullptr)
    {
        data_device->upload_heap_texture_default_luminance->Release();
        data_device->upload_heap_texture_default_luminance = nullptr;
    }

    if (data_device->upload_heap_texture_default_chrominance != nullptr)
    {
        data_device->upload_heap_texture_default_chrominance->Release();
        data_device->upload_heap_texture_default_chrominance = nullptr;
    }

    return 0;
}

int upload_texture_default(pst_device data_device, int index_command_list)
{
    ID3D12Device* device = data_device->device;

    if (_frame_default_image != nullptr)
    {
        D3D12_SUBRESOURCE_DATA texture_data_luminance{};
        texture_data_luminance.pData = _frame_default_image->data[0];
        texture_data_luminance.RowPitch = _frame_default_image->linesize[0];
        texture_data_luminance.SlicePitch = texture_data_luminance.RowPitch * _frame_default_image->height;

        D3D12_SUBRESOURCE_DATA texture_data_chrominance{};
        texture_data_chrominance.pData = _frame_default_image->data[1];
        texture_data_chrominance.RowPitch = _frame_default_image->linesize[1];
        texture_data_chrominance.SlicePitch = texture_data_chrominance.RowPitch * _frame_default_image->height / 2;

        auto it_command_list = _map_command_list.find(data_device->device_index);
        pst_command_list data_command_list = it_command_list->second;

        CD3DX12_RESOURCE_BARRIER barrier_upload_texture_default_before = CD3DX12_RESOURCE_BARRIER::Transition(data_device->texture_default, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
        data_command_list->vector_command_list.at(index_command_list)->ResourceBarrier(1, &barrier_upload_texture_default_before);
        UpdateSubresources(data_command_list->vector_command_list.at(index_command_list), data_device->texture_default, data_device->upload_heap_texture_default_luminance, 0, 0, 1, &texture_data_luminance);
        UpdateSubresources(data_command_list->vector_command_list.at(index_command_list), data_device->texture_default, data_device->upload_heap_texture_default_chrominance, 0, 1, 1, &texture_data_chrominance);
        CD3DX12_RESOURCE_BARRIER barrier_upload_texture_default_after = CD3DX12_RESOURCE_BARRIER::Transition(data_device->texture_default, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        data_command_list->vector_command_list.at(index_command_list)->ResourceBarrier(1, &barrier_upload_texture_default_after);
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC desc_srv{};
    desc_srv.Format = DXGI_FORMAT_R8_UNORM;
    desc_srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    desc_srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc_srv.Texture2D.MostDetailedMip = 0;
    desc_srv.Texture2D.MipLevels = 1;
    desc_srv.Texture2D.PlaneSlice = 0;
    desc_srv.Texture2D.ResourceMinLODClamp = 0.0f;

    _mutex_map_srv_handle_luminance->lock();
    auto it_srv_handle_luminance = _map_srv_handle_luminance.find(data_device->device_index);
    _mutex_map_srv_handle_luminance->unlock();

    pst_srv_handle data_srv_handle_luminance = it_srv_handle_luminance->second;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_luminance = data_srv_handle_luminance->vector_handle_cpu.at(0);

    _mutex_map_srv_handle_chrominance->lock();
    auto it_srv_handle_chrominance = _map_srv_handle_chrominance.find(data_device->device_index);
    _mutex_map_srv_handle_chrominance->unlock();

    pst_srv_handle data_srv_handle_chrominance = it_srv_handle_chrominance->second;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_chrominance = data_srv_handle_chrominance->vector_handle_cpu.at(0);

    desc_srv.Format = DXGI_FORMAT_R8_UNORM;
    desc_srv.Texture2D.PlaneSlice = 0;
    device->CreateShaderResourceView(data_device->texture_default, &desc_srv, srv_handle_cpu_luminance);
    
    desc_srv.Format = DXGI_FORMAT_R8G8_UNORM;
    desc_srv.Texture2D.PlaneSlice = 1;
    device->CreateShaderResourceView(data_device->texture_default, &desc_srv, srv_handle_cpu_chrominance);

    return 0;
}

int create_nv12_texture(pst_device data_device, int width, int height, int index_cef)
{
    HRESULT hr = S_OK;

    ID3D12Device* device = data_device->device;

    pst_texture_cef data_texture_cef = new st_texture_cef();

    D3D12_RESOURCE_DESC desc_texture{};
    desc_texture.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc_texture.Alignment = 0;

    desc_texture.Width = width;
    desc_texture.Height = height;

    desc_texture.Width = (desc_texture.Width + 1) & ~1;  // �ʺ� ¦���� ����
    desc_texture.Height = (desc_texture.Height + 1) & ~1; // ���̸� ¦���� ����

    desc_texture.DepthOrArraySize = 1;
    desc_texture.MipLevels = 1;
    desc_texture.Format = DXGI_FORMAT_NV12;
    desc_texture.SampleDesc.Count = 1;
    desc_texture.SampleDesc.Quality = 0;
    desc_texture.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc_texture.Flags = D3D12_RESOURCE_FLAG_NONE;

    CD3DX12_HEAP_PROPERTIES texture_properties(D3D12_HEAP_TYPE_DEFAULT);

    
    for (UINT i = 0; i < _count_texture_store; i++)
    {
        ID3D12Resource* texture = nullptr;

        hr = device->CreateCommittedResource(
            &texture_properties,
            D3D12_HEAP_FLAG_NONE,
            &desc_texture,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr,
            IID_PPV_ARGS(&texture)
        );

        data_texture_cef->map_texture.insert({ i, texture });
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout_luminance{};
    UINT numRows_luminance = 0;
    UINT64 rowSizeInBytes_luminance = 0;
    UINT64 totalBytes_luminance = 0;

    device->GetCopyableFootprints(&desc_texture, 0, 1, 0, &layout_luminance, &numRows_luminance, &rowSizeInBytes_luminance, &totalBytes_luminance);

    D3D12_RESOURCE_DESC desc_upload{};
    desc_upload.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc_upload.Alignment = 0;
    desc_upload.Width = totalBytes_luminance;
    desc_upload.Height = 1;
    desc_upload.DepthOrArraySize = 1;
    desc_upload.MipLevels = 1;
    desc_upload.Format = DXGI_FORMAT_UNKNOWN;
    desc_upload.SampleDesc.Count = 1;
    desc_upload.SampleDesc.Quality = 0;
    desc_upload.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc_upload.Flags = D3D12_RESOURCE_FLAG_NONE;

    CD3DX12_HEAP_PROPERTIES texture_default_upload_properties(D3D12_HEAP_TYPE_UPLOAD);

    for (UINT i = 0; i < _count_texture_store; i++)
    {
        ID3D12Resource* upload_heap_luminance = nullptr;

        hr = device->CreateCommittedResource(
            &texture_default_upload_properties,
            D3D12_HEAP_FLAG_NONE,
            &desc_upload,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&upload_heap_luminance)
        );

        data_texture_cef->map_upload_heap_luminance.insert({ i, upload_heap_luminance });
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout_chrominance{};
    UINT numRows_chrominance = 0;
    UINT64 rowSizeInBytes_chrominance = 0;
    UINT64 totalBytes_chrominance = 0;

    device->GetCopyableFootprints(&desc_texture, 1, 1, 0, &layout_chrominance, &numRows_chrominance, &rowSizeInBytes_chrominance, &totalBytes_chrominance);

    desc_upload.Width = totalBytes_chrominance;

    for (UINT i = 0; i < _count_texture_store; i++)
    {
        ID3D12Resource* upload_heap_chrominance = nullptr;

        hr = device->CreateCommittedResource(
            &texture_default_upload_properties,
            D3D12_HEAP_FLAG_NONE,
            &desc_upload,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&upload_heap_chrominance)
        );

        data_texture_cef->map_upload_heap_chrominance.insert({ i, upload_heap_chrominance });
    }

    data_device->map_texture_cef.insert({ index_cef, data_texture_cef });

    return 0;
}

int delete_nv12_texture(pst_device data_device)
{
    for (auto it_map_texture_cef = data_device->map_texture_cef.begin(); it_map_texture_cef != data_device->map_texture_cef.end();)
    {
        pst_texture_cef data_texture_cef = it_map_texture_cef->second;

        for (auto it_map_texture = data_texture_cef->map_texture.begin(); it_map_texture != data_texture_cef->map_texture.end();)
        {
            ID3D12Resource* resource = it_map_texture->second;

            if (resource)
            {
                resource->Release();
                resource = nullptr;
            }

            it_map_texture = data_texture_cef->map_texture.erase(it_map_texture);
        }

        for (auto it_map_upload_heap_luminance = data_texture_cef->map_upload_heap_luminance.begin(); it_map_upload_heap_luminance != data_texture_cef->map_upload_heap_luminance.end();)
        {
            ID3D12Resource* resource = it_map_upload_heap_luminance->second;

            if (resource)
            {
                resource->Release();
                resource = nullptr;
            }

            it_map_upload_heap_luminance = data_texture_cef->map_upload_heap_luminance.erase(it_map_upload_heap_luminance);
        }

        for (auto it_map_upload_heap_chrominance = data_texture_cef->map_upload_heap_chrominance.begin(); it_map_upload_heap_chrominance != data_texture_cef->map_upload_heap_chrominance.end();)
        {
            ID3D12Resource* resource = it_map_upload_heap_chrominance->second;

            if (resource)
            {
                resource->Release();
                resource = nullptr;
            }

            it_map_upload_heap_chrominance = data_texture_cef->map_upload_heap_chrominance.erase(it_map_upload_heap_chrominance);
        }

        delete data_texture_cef;
        data_texture_cef = nullptr;

        it_map_texture_cef = data_device->map_texture_cef.erase(it_map_texture_cef);
    }
    
    return 0;
}

int upload_texture_cef(pst_device data_device, pst_cef data_cef, int index_command_list_upload, int counter_texture)
{
    auto it_command_list = _map_command_list.find(data_device->device_index);
    pst_command_list data_command_list = it_command_list->second;
    ID3D12GraphicsCommandList* cmd_list = data_command_list->vector_command_list.at(index_command_list_upload);

    ID3D12Device* device = data_device->device;

    pst_cef_device data_cef_device = nullptr;
    for (auto it_cef_device = data_cef->map_cef_device.begin(); it_cef_device != data_cef->map_cef_device.end(); it_cef_device++)
    {
        data_cef_device = it_cef_device->second;

        if (data_cef_device->index_cef_device == data_device->device_index)
        {
            break;
        }
    }

    pst_cef_data_paint data_paint = data_cef->map_cef_data_paint->find(data_cef->index_upload_texture)->second;

    int index_texture = (_count_texture_store * counter_texture) + data_cef_device->index_srv_upload + 1;
    
    int rgba_size = data_paint->width * data_paint->height * 4;

    int y_plane_size = data_paint->width * data_paint->height;
    int uv_plane_size = (data_paint->width / 2) * (data_paint->height / 2) * 2;
    int nv12_size = y_plane_size + uv_plane_size;

    if (data_paint->nv12_buffer == nullptr)
    {
        data_paint->nv12_buffer = new uint8_t[nv12_size];
    }

    uint8_t* src_data[4] = { (uint8_t*)data_paint->buffer, nullptr, nullptr, nullptr };
    int src_linesize[4] = { data_paint->width * 4, 0, 0, 0 };

    uint8_t* dst_data[4] = { (uint8_t*)data_paint->nv12_buffer, (uint8_t*)data_paint->nv12_buffer + y_plane_size, nullptr, nullptr };
    int dst_linesize[4] = { data_paint->width, data_paint->width, 0, 0 };

    SwsContext* sws_ctx = sws_getContext(
        data_paint->width, data_paint->height, AV_PIX_FMT_BGRA,
        data_paint->width, data_paint->height, AV_PIX_FMT_NV12,
        SWS_BICUBIC, nullptr, nullptr, nullptr
    );

    if (sws_ctx)
    {
        sws_scale(sws_ctx, src_data, src_linesize, 0, data_paint->height, dst_data, dst_linesize);
        sws_freeContext(sws_ctx);
    }

    D3D12_SUBRESOURCE_DATA texture_data_luminance{};
    texture_data_luminance.pData = dst_data[0];
    texture_data_luminance.RowPitch = dst_linesize[0];
    texture_data_luminance.SlicePitch = texture_data_luminance.RowPitch * data_paint->height;

    D3D12_SUBRESOURCE_DATA texture_data_chrominance{};
    texture_data_chrominance.pData = dst_data[1];
    texture_data_chrominance.RowPitch = dst_linesize[1];
    texture_data_chrominance.SlicePitch = texture_data_chrominance.RowPitch * data_paint->height / 2;

    auto it_texture_cef = data_device->map_texture_cef.find(data_cef->index_cef);
    pst_texture_cef data_texture_cef = it_texture_cef->second;
    ID3D12Resource* resource = data_texture_cef->map_texture.at(data_cef_device->index_srv_upload);
    ID3D12Resource* upload_heap_luminance = data_texture_cef->map_upload_heap_luminance.at(data_cef_device->index_srv_upload);
    ID3D12Resource* upload_heap_chrominance = data_texture_cef->map_upload_heap_chrominance.at(data_cef_device->index_srv_upload);

    CD3DX12_RESOURCE_BARRIER transition_barrier_nv12 = CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    cmd_list->ResourceBarrier(1, &transition_barrier_nv12);

    UpdateSubresources(cmd_list, resource, upload_heap_luminance, 0, 0, 1, &texture_data_luminance);
    UpdateSubresources(cmd_list, resource, upload_heap_chrominance, 0, 1, 1, &texture_data_chrominance);

    transition_barrier_nv12 = CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmd_list->ResourceBarrier(1, &transition_barrier_nv12);

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
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_luminance = data_srv_handle_luminance->vector_handle_cpu.at(index_texture);

    _mutex_map_srv_handle_chrominance->lock();
    auto it_srv_handle_chrominance = _map_srv_handle_chrominance.find(data_device->device_index);
    _mutex_map_srv_handle_chrominance->unlock();

    pst_srv_handle data_srv_handle_chrominance = it_srv_handle_chrominance->second;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle_cpu_chrominance = data_srv_handle_chrominance->vector_handle_cpu.at(index_texture);

    // Y ä�� �Ҵ�
    srv_desc.Format = DXGI_FORMAT_R8_UNORM;
    srv_desc.Texture2D.PlaneSlice = 0;
    data_device->device->CreateShaderResourceView(resource, &srv_desc, srv_handle_cpu_luminance);

    // UV ä�� �Ҵ�
    srv_desc.Format = DXGI_FORMAT_R8G8_UNORM;
    srv_desc.Texture2D.PlaneSlice = 1;
    data_device->device->CreateShaderResourceView(resource, &srv_desc, srv_handle_cpu_chrominance);

    return 0;
}

int create_ffmpeg_instance_default_image(void*& instance)
{
    instance = cpp_ffmpeg_wrapper_create();
    cpp_ffmpeg_wrapper_initialize(instance, callback_ffmpeg_wrapper_ptr);

    cpp_ffmpeg_wrapper_set_scale(instance, true);

    cpp_ffmpeg_wrapper_set_file_path(instance, (char*)_default_texture_url.c_str());
    if (cpp_ffmpeg_wrapper_open_file(instance) != 0)
    {
        cpp_ffmpeg_wrapper_shutdown(instance);
        cpp_ffmpeg_wrapper_delete(instance);

        return 1;
    }

    cpp_ffmpeg_wrapper_play_start(instance, nullptr);

    return 0;
}

void delete_scenes_data()
{
    for (auto it_scene = _map_scene.begin(); it_scene != _map_scene.end(); it_scene++)
    {
        pst_scene data_scene = it_scene->second;

        for (auto it = data_scene->map_ffmpeg_instance.begin(); it != data_scene->map_ffmpeg_instance.end();)
        {
            if (it->second != nullptr)
            {
                cpp_ffmpeg_wrapper_play_stop(it->second, nullptr);
                cpp_ffmpeg_wrapper_shutdown(it->second);
                cpp_ffmpeg_wrapper_delete(it->second);
                it->second = nullptr;
            }

            it = data_scene->map_ffmpeg_instance.erase(it);
        }

        for (auto it_vector = data_scene->vector_frame.begin(); it_vector != data_scene->vector_frame.end();)
        {
            AVFrame* frame = *it_vector;

            av_frame_free(&frame);

            it_vector = data_scene->vector_frame.erase(it_vector);
        }
    }
}

int create_ffmpeg_instance_set_data(void*& instance, UINT index_device, std::string url)
{
    instance = cpp_ffmpeg_wrapper_create();
    cpp_ffmpeg_wrapper_initialize(instance, callback_ffmpeg_wrapper_ptr);

    // NV12
    cpp_ffmpeg_wrapper_set_hw_decode(instance);
    cpp_ffmpeg_wrapper_set_hw_device_type(instance, _hw_device_type);
    cpp_ffmpeg_wrapper_set_hw_decode_adapter_index(instance, index_device);

    cpp_ffmpeg_wrapper_set_scale(instance, false);

    cpp_ffmpeg_wrapper_set_file_path(instance, (char*)url.c_str());

    return 0;
}

int create_ffmpeg_instance_check_open_file_on_live_stream(void* instance)
{
    if (cpp_ffmpeg_wrapper_open_file(instance) != 0)
    {
        return 1;
    }

    return 0;
}

int create_ffmpeg_instance_play_start_set_data(void* instance, UINT& index_scene, RECT rect)
{
    index_scene = _next_scene_index;
    _next_scene_index++;

    cpp_ffmpeg_wrapper_set_scene_index(instance, index_scene);
    cpp_ffmpeg_wrapper_set_rect(instance, rect);

    return 0;
}

int create_ffmpeg_instance_play_start_on_live_stream(void* instance)
{
    cpp_ffmpeg_wrapper_play_start(instance, nullptr);

    return 0;
}

void create_texts()
{
    for (auto it_device = _map_device.begin(); it_device != _map_device.end(); it_device++)
    {
        pst_device data_device = it_device->second;

        pst_text data_text = new st_text();
        data_text->index_device = data_device->device_index;

        data_text->interval_blink_in_miliseconds = 5'000;
        data_text->counter_interval_blink = 0;

        data_text->duration_blink_in_miliseconds = 500;
        data_text->counter_duration_blink = 0;

        data_text->flag_blink_status = false;

        _map_text.insert({ data_text->index_device, data_text });
    }
}

void delete_texts()
{
    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); )
    {
        pst_text data_text = it_text->second;

        {
            auto it_device = _map_device.find(data_text->index_device);
            pst_device data_device = it_device->second;

            std::unique_lock<std::shared_mutex> lk(*data_device->mutex_map_text_internal);
            for (auto it_text_internal = data_text->map_text_internal.begin(); it_text_internal != data_text->map_text_internal.end(); )
            {
                pst_text_internal data_text_internal = it_text_internal->second;

                if (data_text_internal != nullptr)
                {
                    if (data_text_internal->text_string.empty() == false)
                    {
                        data_text_internal->text_string.clear();
                        data_text_internal->text_string.shrink_to_fit();
                    }
                    if (data_text_internal->text_color != nullptr)
                    {
                        delete data_text_internal->text_color;
                        data_text_internal->text_color = nullptr;
                    }
                    if (data_text_internal->text_size != nullptr)
                    {
                        delete data_text_internal->text_size;
                        data_text_internal->text_size = nullptr;
                    }
                    if (data_text_internal->text_font_family.empty() == false)
                    {
                        data_text_internal->text_font_family.clear();
                        data_text_internal->text_font_family.shrink_to_fit();
                    }

                    if (data_text_internal->text_color_background != nullptr)
                    {
                        delete data_text_internal->text_color_background;
                        data_text_internal->text_color_background = nullptr;
                    }
                    if (data_text_internal->text_start_coordinate_left != nullptr)
                    {
                        delete data_text_internal->text_start_coordinate_left;
                        data_text_internal->text_start_coordinate_left = nullptr;
                    }
                    if (data_text_internal->text_start_coordinate_top != nullptr)
                    {
                        delete data_text_internal->text_start_coordinate_top;
                        data_text_internal->text_start_coordinate_top = nullptr;
                    }
                    if (data_text_internal->text_background_width != nullptr)
                    {
                        delete data_text_internal->text_background_width;
                        data_text_internal->text_background_width = nullptr;
                    }
                    if (data_text_internal->text_background_height != nullptr)
                    {
                        delete data_text_internal->text_background_height;
                        data_text_internal->text_background_height = nullptr;
                    }

                    if (data_text_internal->text_weight != nullptr)
                    {
                        delete data_text_internal->text_weight;
                        data_text_internal->text_weight = nullptr;
                    }
                    if (data_text_internal->text_style != nullptr)
                    {
                        delete data_text_internal->text_style;
                        data_text_internal->text_style = nullptr;
                    }
                    if (data_text_internal->text_stretch != nullptr)
                    {
                        delete data_text_internal->text_stretch;
                        data_text_internal->text_stretch = nullptr;
                    }

                    if (data_text_internal->text_format != nullptr)
                    {
                        data_text_internal->text_format->Release();
                        data_text_internal->text_format = nullptr;
                    }
                    if (data_text_internal->text_brush != nullptr)
                    {
                        data_text_internal->text_brush->Release();
                        data_text_internal->text_brush = nullptr;
                    }
                    if (data_text_internal->text_layout != nullptr)
                    {
                        data_text_internal->text_layout->Release();
                        data_text_internal->text_layout = nullptr;
                    }
                    if (data_text_internal->text_matrics != nullptr)
                    {
                        delete data_text_internal->text_matrics;
                        data_text_internal->text_matrics = nullptr;
                    }
                    if (data_text_internal->text_brush_background != nullptr)
                    {
                        data_text_internal->text_brush_background->Release();
                        data_text_internal->text_brush_background = nullptr;
                    }

                    if (data_text_internal->movement_type_horizontal != nullptr)
                    {
                        delete data_text_internal->movement_type_horizontal;
                        data_text_internal->movement_type_horizontal = nullptr;
                    }
                    if (data_text_internal->movement_speed_horizontal != nullptr)
                    {
                        delete data_text_internal->movement_speed_horizontal;
                        data_text_internal->movement_speed_horizontal = nullptr;
                    }
                    if (data_text_internal->movement_threshold_horizontal != nullptr)
                    {
                        delete data_text_internal->movement_threshold_horizontal;
                        data_text_internal->movement_threshold_horizontal = nullptr;
                    }
                    if (data_text_internal->movement_translation_horizontal != nullptr)
                    {
                        delete data_text_internal->movement_translation_horizontal;
                        data_text_internal->movement_translation_horizontal = nullptr;
                    }

                    if (data_text_internal->movement_type_horizontal_background != nullptr)
                    {
                        delete data_text_internal->movement_type_horizontal_background;
                        data_text_internal->movement_type_horizontal_background = nullptr;
                    }
                    if (data_text_internal->movement_speed_horizontal_background != nullptr)
                    {
                        delete data_text_internal->movement_speed_horizontal_background;
                        data_text_internal->movement_speed_horizontal_background = nullptr;
                    }
                    if (data_text_internal->movement_threshold_horizontal_background != nullptr)
                    {
                        delete data_text_internal->movement_threshold_horizontal_background;
                        data_text_internal->movement_threshold_horizontal_background = nullptr;
                    }
                    if (data_text_internal->movement_translation_horizontal_background != nullptr)
                    {
                        delete data_text_internal->movement_translation_horizontal_background;
                        data_text_internal->movement_translation_horizontal_background = nullptr;
                    }

                    if (data_text_internal->movement_type_vertical != nullptr)
                    {
                        delete data_text_internal->movement_type_vertical;
                        data_text_internal->movement_type_vertical = nullptr;
                    }
                    if (data_text_internal->movement_speed_vertical != nullptr)
                    {
                        delete data_text_internal->movement_speed_vertical;
                        data_text_internal->movement_speed_vertical = nullptr;
                    }
                    if (data_text_internal->movement_threshold_vertical != nullptr)
                    {
                        delete data_text_internal->movement_threshold_vertical;
                        data_text_internal->movement_threshold_vertical = nullptr;
                    }
                    if (data_text_internal->movement_translation_vertical != nullptr)
                    {
                        delete data_text_internal->movement_translation_vertical;
                        data_text_internal->movement_translation_vertical = nullptr;
                    }

                    if (data_text_internal->movement_type_vertical_background != nullptr)
                    {
                        delete data_text_internal->movement_type_vertical_background;
                        data_text_internal->movement_type_vertical_background = nullptr;
                    }
                    if (data_text_internal->movement_speed_vertical_background != nullptr)
                    {
                        delete data_text_internal->movement_speed_vertical_background;
                        data_text_internal->movement_speed_vertical_background = nullptr;
                    }
                    if (data_text_internal->movement_threshold_vertical_background != nullptr)
                    {
                        delete data_text_internal->movement_threshold_vertical_background;
                        data_text_internal->movement_threshold_vertical_background = nullptr;
                    }
                    if (data_text_internal->movement_translation_vertical_background != nullptr)
                    {
                        delete data_text_internal->movement_translation_vertical_background;
                        data_text_internal->movement_translation_vertical_background = nullptr;
                    }

                    delete data_text_internal;
                    data_text_internal = nullptr;
                }

                it_text_internal = data_text->map_text_internal.erase(it_text_internal);
            }
        }

        delete data_text;
        data_text = nullptr;

        it_text = _map_text.erase(it_text);
    }
}

void create_text(int index_text)
{
    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_device = _map_device.find(data_text->index_device);
        pst_device data_device = it_device->second;

        std::unique_lock<std::shared_mutex> lk(*data_device->mutex_map_text_internal);

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal != data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = new st_text_internal();
        
        data_text_internal->index_text_internal = index_text;

        data_text_internal->text_string = DEFAULT_TEXT_STRING;

        data_text_internal->text_color = new st_color();
        data_text_internal->text_color->r = DEFAULT_TEXT_COLOR;
        data_text_internal->text_color->g = DEFAULT_TEXT_COLOR;
        data_text_internal->text_color->b = DEFAULT_TEXT_COLOR;
        data_text_internal->text_color->a = DEFAULT_TEXT_COLOR;

        data_text_internal->text_size = new int();
        *data_text_internal->text_size = DEFAULT_TEXT_SIZE;

        data_text_internal->text_font_family = DEFAULT_TEXT_FONT_FAMILY;

        data_text_internal->text_color_background = new st_color();
        data_text_internal->text_color_background->r = DEFAULT_TEXT_COLOR_BACKGROUND;
        data_text_internal->text_color_background->g = DEFAULT_TEXT_COLOR_BACKGROUND;
        data_text_internal->text_color_background->b = DEFAULT_TEXT_COLOR_BACKGROUND;
        data_text_internal->text_color_background->a = DEFAULT_TEXT_COLOR_BACKGROUND;

        data_text_internal->text_start_coordinate_left = new float();
        *data_text_internal->text_start_coordinate_left = DEFAULT_TEXT_BACKGROUND_RECTANGLE;
        data_text_internal->text_start_coordinate_top = new float();
        *data_text_internal->text_start_coordinate_top = DEFAULT_TEXT_BACKGROUND_RECTANGLE;
        data_text_internal->text_background_width = new float();
        *data_text_internal->text_background_width = DEFAULT_TEXT_BACKGROUND_RECTANGLE;
        data_text_internal->text_background_height = new float();
        *data_text_internal->text_background_height = DEFAULT_TEXT_BACKGROUND_RECTANGLE;

        data_text_internal->text_weight = new e_dwrite_font_weight();
        *data_text_internal->text_weight = DEFAULT_TEXT_WEIGHT;
        data_text_internal->text_style = new e_dwrite_font_style();
        *data_text_internal->text_style = DEFAULT_TEXT_STYLE;
        data_text_internal->text_stretch = new e_dwrite_font_stretch();
        *data_text_internal->text_stretch = DEFAULT_TEXT_STRETCH;

        data_text_internal->text_matrics = new DWRITE_TEXT_METRICS();

        data_text_internal->movement_type_horizontal = new e_movement_type_horizontal();
        *data_text_internal->movement_type_horizontal = (e_movement_type_horizontal)DEFAULT_TEXT_MOVEMENT_TYPE;
        data_text_internal->movement_speed_horizontal = new float();
        *data_text_internal->movement_speed_horizontal = DEFAULT_TEXT_MOVEMENT_SPEED;
        data_text_internal->movement_threshold_horizontal = new float();
        *data_text_internal->movement_threshold_horizontal = DEFAULT_TEXT_MOVEMENT_THRESHOLD;
        data_text_internal->movement_translation_horizontal = new float();
        *data_text_internal->movement_translation_horizontal = DEFAULT_TEXT_MOVEMENT_TRANSLATION;

        data_text_internal->movement_type_horizontal_background = new e_movement_type_horizontal();
        *data_text_internal->movement_type_horizontal_background = (e_movement_type_horizontal)DEFAULT_TEXT_MOVEMENT_TYPE;
        data_text_internal->movement_speed_horizontal_background = new float();
        *data_text_internal->movement_speed_horizontal_background = DEFAULT_TEXT_MOVEMENT_SPEED;
        data_text_internal->movement_threshold_horizontal_background = new float();
        *data_text_internal->movement_threshold_horizontal_background = DEFAULT_TEXT_MOVEMENT_THRESHOLD;
        data_text_internal->movement_translation_horizontal_background = new float();
        *data_text_internal->movement_translation_horizontal_background = DEFAULT_TEXT_MOVEMENT_TRANSLATION;

        data_text_internal->movement_type_vertical = new e_movement_type_vertical();
        *data_text_internal->movement_type_vertical = (e_movement_type_vertical)DEFAULT_TEXT_MOVEMENT_TYPE;
        data_text_internal->movement_speed_vertical = new float();
        *data_text_internal->movement_speed_vertical = DEFAULT_TEXT_MOVEMENT_SPEED;
        data_text_internal->movement_threshold_vertical = new float();
        *data_text_internal->movement_threshold_vertical = DEFAULT_TEXT_MOVEMENT_THRESHOLD;
        data_text_internal->movement_translation_vertical = new float();
        *data_text_internal->movement_translation_vertical = DEFAULT_TEXT_MOVEMENT_TRANSLATION;

        data_text_internal->movement_type_vertical_background = new e_movement_type_vertical();
        *data_text_internal->movement_type_vertical_background = (e_movement_type_vertical)DEFAULT_TEXT_MOVEMENT_TYPE;
        data_text_internal->movement_speed_vertical_background = new float();
        *data_text_internal->movement_speed_vertical_background = DEFAULT_TEXT_MOVEMENT_SPEED;
        data_text_internal->movement_threshold_vertical_background = new float();
        *data_text_internal->movement_threshold_vertical_background = DEFAULT_TEXT_MOVEMENT_THRESHOLD;
        data_text_internal->movement_translation_vertical_background = new float();
        *data_text_internal->movement_translation_vertical_background = DEFAULT_TEXT_MOVEMENT_TRANSLATION;

        data_text_internal->flag_created = false;
        data_text_internal->flag_deleted = false;

        data_text_internal->flag_blink = false;

        data_text->map_text_internal.insert({ index_text, data_text_internal });
    }
}

void delete_text(int index_text)
{
    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal != data_text->map_text_internal.end())
        {
            pst_text_internal data_text_internal = it_text_internal->second;

            data_text_internal->flag_deleted = true;
        }
    }
}

void set_text_string(int index_text, std::wstring text_string)
{
    if (text_string == DEFAULT_TEXT_STRING)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        data_text_internal->text_string = text_string;
    }
}

void set_text_color(int index_text, int color_r, int color_g, int color_b, int color_a)
{
    if ((float)color_r == DEFAULT_TEXT_COLOR 
        && (float)color_g == DEFAULT_TEXT_COLOR 
        && (float)color_b == DEFAULT_TEXT_COLOR 
        && (float)color_a == DEFAULT_TEXT_COLOR)
    {
        return;
    }

    if (color_r < 0)
    {
        color_r = 0;
    }
    else if (color_r > 255)
    {
        color_r = 255;
    }

    if (color_g < 0)
    {
        color_g = 0;
    }
    else if (color_g > 255)
    {
        color_g = 255;
    }

    if (color_b < 0)
    {
        color_b = 0;
    }
    else if (color_b > 255)
    {
        color_b = 255;
    }

    if (color_a < 0)
    {
        color_a = 0;
    }
    else if (color_a > 255)
    {
        color_a = 255;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        data_text_internal->text_color->r = color_r / 255.0f;
        data_text_internal->text_color->g = color_g / 255.0f;
        data_text_internal->text_color->b = color_b / 255.0f;
        data_text_internal->text_color->a = color_a / 255.0f;
    }
}

void set_text_size(int index_text, int font_size)
{
    if (font_size == DEFAULT_TEXT_SIZE)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->text_size = font_size;
    }
}

void set_text_font_family(int index_text, std::wstring font_family)
{
    if (font_family == DEFAULT_TEXT_FONT_FAMILY)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        data_text_internal->text_font_family = font_family;
    }
}

void set_text_color_background(int index_text, int color_r, int color_g, int color_b, int color_a)
{
    if ((float)color_r == DEFAULT_TEXT_COLOR_BACKGROUND 
        && (float)color_g == DEFAULT_TEXT_COLOR_BACKGROUND 
        && (float)color_b == DEFAULT_TEXT_COLOR_BACKGROUND 
        && (float)color_a == DEFAULT_TEXT_COLOR_BACKGROUND)
    {
        return;
    }

    if (color_r < 0)
    {
        color_r = 0;
    }
    else if (color_r > 255)
    {
        color_r = 255;
    }

    if (color_g < 0)
    {
        color_g = 0;
    }
    else if (color_g > 255)
    {
        color_g = 255;
    }

    if (color_b < 0)
    {
        color_b = 0;
    }
    else if (color_b > 255)
    {
        color_b = 255;
    }

    if (color_a < 0)
    {
        color_a = 0;
    }
    else if (color_a > 255)
    {
        color_a = 255;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        data_text_internal->text_color_background->r = color_r / 255.0f;
        data_text_internal->text_color_background->g = color_g / 255.0f;
        data_text_internal->text_color_background->b = color_b / 255.0f;
        data_text_internal->text_color_background->a = color_a / 255.0f;
    }
}

void set_text_start_coordinate_left(int index_text, int left)
{
    if (left == DEFAULT_TEXT_BACKGROUND_RECTANGLE)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->text_start_coordinate_left = (float)left;
    }
}

void set_text_start_coordinate_top(int index_text, int top)
{
    if (top == DEFAULT_TEXT_BACKGROUND_RECTANGLE)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->text_start_coordinate_top = (float)top;
    }
}

void set_text_background_width(int index_text, int width)
{
    if (width == DEFAULT_TEXT_BACKGROUND_RECTANGLE)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->text_background_width = (float)width;
    }
}

void set_text_background_height(int index_text, int height)
{
    if (height == DEFAULT_TEXT_BACKGROUND_RECTANGLE)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->text_background_height = (float)height;
    }
}

void set_text_weight(int index_text, e_dwrite_font_weight weight)
{
    if (weight == DEFAULT_TEXT_WEIGHT)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->text_weight = weight;
    }
}

void set_text_style(int index_text, e_dwrite_font_style style)
{
    if (style == DEFAULT_TEXT_STYLE)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->text_style = style;
    }
}

void set_text_stretch(int index_text, e_dwrite_font_stretch stretch)
{
    if (stretch == DEFAULT_TEXT_STRETCH)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->text_stretch = stretch;
    }
}

void set_text_movement_type_horizontal(int index_text, e_movement_type_horizontal type)
{
    if (type == (e_movement_type_horizontal)DEFAULT_TEXT_MOVEMENT_TYPE)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->movement_type_horizontal = type;
    }
}

void set_text_movement_speed_horizontal(int index_text, int speed)
{
    if (speed == (int)DEFAULT_TEXT_MOVEMENT_SPEED)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->movement_speed_horizontal = (float)speed;
    }
}

void set_text_movement_threshold_horizontal(int index_text, int threshold)
{
    if (threshold == DEFAULT_TEXT_MOVEMENT_THRESHOLD_PACKET)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->movement_threshold_horizontal = (float)threshold;
    }
}

void set_text_movement_type_horizontal_background(int index_text, e_movement_type_horizontal type)
{
    if (type == (e_movement_type_horizontal)DEFAULT_TEXT_MOVEMENT_TYPE)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->movement_type_horizontal_background = type;
    }
}

void set_text_movement_speed_horizontal_background(int index_text, int speed)
{
    if (speed == (int)DEFAULT_TEXT_MOVEMENT_SPEED)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->movement_speed_horizontal_background = (float)speed;
    }
}

void set_text_movement_threshold_horizontal_background(int index_text, int threshold)
{
    if (threshold == DEFAULT_TEXT_MOVEMENT_THRESHOLD_PACKET)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->movement_threshold_horizontal_background = (float)threshold;
    }
}

void set_text_movement_type_vertical(int index_text, e_movement_type_vertical type)
{
    if (type == (e_movement_type_vertical)DEFAULT_TEXT_MOVEMENT_TYPE)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->movement_type_vertical = type;
    }
}

void set_text_movement_speed_vertical(int index_text, int speed)
{
    if (speed == (int)DEFAULT_TEXT_MOVEMENT_SPEED)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->movement_speed_vertical = (float)speed;
    }
}

void set_text_movement_threshold_vertical(int index_text, int threshold)
{
    if (threshold == DEFAULT_TEXT_MOVEMENT_THRESHOLD_PACKET)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->movement_threshold_vertical = (float)threshold;
    }
}

void set_text_movement_type_vertical_background(int index_text, e_movement_type_vertical type)
{
    if (type == (e_movement_type_vertical)DEFAULT_TEXT_MOVEMENT_TYPE)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->movement_type_vertical_background = type;
    }
}

void set_text_movement_speed_vertical_background(int index_text, int speed)
{
    if (speed == (int)DEFAULT_TEXT_MOVEMENT_SPEED)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->movement_speed_vertical_background = (float)speed;
    }
}

void set_text_movement_threshold_vertical_background(int index_text, int threshold)
{
    if (threshold == DEFAULT_TEXT_MOVEMENT_THRESHOLD_PACKET)
    {
        return;
    }

    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal == data_text->map_text_internal.end())
        {
            continue;
        }

        pst_text_internal data_text_internal = it_text_internal->second;
        *data_text_internal->movement_threshold_vertical_background = (float)threshold;
    }
}

void create_text_instance(int index_text, pst_text data_text)
{
    auto it_device = _map_device.find(data_text->index_device);
    if (it_device == _map_device.end())
    {
        return;
    }

    auto it_text_internal = data_text->map_text_internal.find(index_text);
    if (it_text_internal == data_text->map_text_internal.end())
    {
        return;
    }

    pst_device data_device = it_device->second;

    pst_text_internal data_text_internal = it_text_internal->second;

    if (data_text_internal->text_brush == nullptr)
    {
        float color_r = data_text_internal->text_color->r;
        float color_g = data_text_internal->text_color->g;
        float color_b = data_text_internal->text_color->b;
        float color_a = data_text_internal->text_color->a;
        data_device->device_context_2d->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(color_r, color_g, color_b, color_a)), &data_text_internal->text_brush);
    }

    if (data_text_internal->text_format == nullptr)
    {
        HRESULT hr = _factory_dwrite->CreateTextFormat(
            data_text_internal->text_font_family.c_str(),
            nullptr,
            DWRITE_FONT_WEIGHT(*data_text_internal->text_weight),
            DWRITE_FONT_STYLE(*data_text_internal->text_style),
            DWRITE_FONT_STRETCH(*data_text_internal->text_stretch),
            (float)(*data_text_internal->text_size),
            L"ko-KR",
            &data_text_internal->text_format
        );

        if (hr != S_OK)
        {
            _factory_dwrite->CreateTextFormat(
                L"Arial",
                nullptr,
                DWRITE_FONT_WEIGHT(*data_text_internal->text_weight),
                DWRITE_FONT_STYLE(*data_text_internal->text_style),
                DWRITE_FONT_STRETCH(*data_text_internal->text_stretch),
                (float)(*data_text_internal->text_size),
                L"ko-KR",
                &data_text_internal->text_format
            );
        }
    }
        
    bool flag_background_rectangle_setted = false;
    if (*data_text_internal->text_background_width != DEFAULT_TEXT_BACKGROUND_RECTANGLE 
        && *data_text_internal->text_background_height != DEFAULT_TEXT_BACKGROUND_RECTANGLE)
    {
        flag_background_rectangle_setted = true;
    }

    if (data_text_internal->text_layout == nullptr)
    {
        if (flag_background_rectangle_setted == false)
        {
            _factory_dwrite->CreateTextLayout(
                data_text_internal->text_string.c_str(),
                data_text_internal->text_string.size(),
                data_text_internal->text_format,
                FLT_MAX,
                FLT_MAX,
                &data_text_internal->text_layout
            );
        }
        else
        {
            _factory_dwrite->CreateTextLayout(
                data_text_internal->text_string.c_str(),
                data_text_internal->text_string.size(),
                data_text_internal->text_format,
                (*data_text_internal->text_background_width),
                (*data_text_internal->text_background_height),
                &data_text_internal->text_layout
            );
        }
    }

    if (data_text_internal->text_matrics != nullptr)
    {
        data_text_internal->text_layout->GetMetrics(data_text_internal->text_matrics);

        if (flag_background_rectangle_setted == false)
        {
            data_text_internal->text_layout->SetMaxWidth(data_text_internal->text_matrics->width);
            data_text_internal->text_layout->SetMaxHeight(data_text_internal->text_matrics->height);
            data_text_internal->text_layout->GetMetrics(data_text_internal->text_matrics);
        }
    }

    if (data_text_internal->text_brush_background == nullptr)
    {
        float color_background_r = data_text_internal->text_color_background->r;
        float color_background_g = data_text_internal->text_color_background->g;
        float color_background_b = data_text_internal->text_color_background->b;
        float color_background_a = data_text_internal->text_color_background->a;
        data_device->device_context_2d->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(color_background_r, color_background_g, color_background_b, color_background_a)), &data_text_internal->text_brush_background);
    }

    data_text_internal->flag_created = true;
}

void delete_text_instance(int index_text, pst_text data_text)
{
    auto it_text_internal = data_text->map_text_internal.find(index_text);
    if (it_text_internal != data_text->map_text_internal.end())
    {
        pst_text_internal data_text_internal = it_text_internal->second;

        if (data_text_internal->text_string.empty() == false)
        {
            data_text_internal->text_string.clear();
            data_text_internal->text_string.shrink_to_fit();
        }
        if (data_text_internal->text_color != nullptr)
        {
            delete data_text_internal->text_color;
            data_text_internal->text_color = nullptr;
        }
        if (data_text_internal->text_size != nullptr)
        {
            delete data_text_internal->text_size;
            data_text_internal->text_size = nullptr;
        }
        if (data_text_internal->text_font_family.empty() == false)
        {
            data_text_internal->text_font_family.clear();
            data_text_internal->text_font_family.shrink_to_fit();
        }

        if (data_text_internal->text_color_background != nullptr)
        {
            delete data_text_internal->text_color_background;
            data_text_internal->text_color_background = nullptr;
        }
        if (data_text_internal->text_start_coordinate_left != nullptr)
        {
            delete data_text_internal->text_start_coordinate_left;
            data_text_internal->text_start_coordinate_left = nullptr;
        }
        if (data_text_internal->text_start_coordinate_top != nullptr)
        {
            delete data_text_internal->text_start_coordinate_top;
            data_text_internal->text_start_coordinate_top = nullptr;
        }
        if (data_text_internal->text_background_width != nullptr)
        {
            delete data_text_internal->text_background_width;
            data_text_internal->text_background_width = nullptr;
        }
        if (data_text_internal->text_background_height != nullptr)
        {
            delete data_text_internal->text_background_height;
            data_text_internal->text_background_height = nullptr;
        }

        if (data_text_internal->text_weight != nullptr)
        {
            delete data_text_internal->text_weight;
            data_text_internal->text_weight = nullptr;
        }
        if (data_text_internal->text_style != nullptr)
        {
            delete data_text_internal->text_style;
            data_text_internal->text_style = nullptr;
        }
        if (data_text_internal->text_stretch != nullptr)
        {
            delete data_text_internal->text_stretch;
            data_text_internal->text_stretch = nullptr;
        }

        if (data_text_internal->text_format != nullptr)
        {
            data_text_internal->text_format->Release();
            data_text_internal->text_format = nullptr;
        }
        if (data_text_internal->text_brush != nullptr)
        {
            data_text_internal->text_brush->Release();
            data_text_internal->text_brush = nullptr;
        }
        if (data_text_internal->text_layout != nullptr)
        {
            data_text_internal->text_layout->Release();
            data_text_internal->text_layout = nullptr;
        }
        if (data_text_internal->text_matrics != nullptr)
        {
            delete data_text_internal->text_matrics;
            data_text_internal->text_matrics = nullptr;
        }

        if (data_text_internal->text_brush_background != nullptr)
        {
            data_text_internal->text_brush_background->Release();
            data_text_internal->text_brush_background = nullptr;
        }

        if (data_text_internal->movement_type_horizontal != nullptr)
        {
            delete data_text_internal->movement_type_horizontal;
            data_text_internal->movement_type_horizontal = nullptr;
        }
        if (data_text_internal->movement_speed_horizontal != nullptr)
        {
            delete data_text_internal->movement_speed_horizontal;
            data_text_internal->movement_speed_horizontal = nullptr;
        }
        if (data_text_internal->movement_threshold_horizontal != nullptr)
        {
            delete data_text_internal->movement_threshold_horizontal;
            data_text_internal->movement_threshold_horizontal = nullptr;
        }
        if (data_text_internal->movement_translation_horizontal != nullptr)
        {
            delete data_text_internal->movement_translation_horizontal;
            data_text_internal->movement_translation_horizontal = nullptr;
        }

        if (data_text_internal->movement_type_horizontal_background != nullptr)
        {
            delete data_text_internal->movement_type_horizontal_background;
            data_text_internal->movement_type_horizontal_background = nullptr;
        }
        if (data_text_internal->movement_speed_horizontal_background != nullptr)
        {
            delete data_text_internal->movement_speed_horizontal_background;
            data_text_internal->movement_speed_horizontal_background = nullptr;
        }
        if (data_text_internal->movement_threshold_horizontal_background != nullptr)
        {
            delete data_text_internal->movement_threshold_horizontal_background;
            data_text_internal->movement_threshold_horizontal_background = nullptr;
        }
        if (data_text_internal->movement_translation_horizontal_background != nullptr)
        {
            delete data_text_internal->movement_translation_horizontal_background;
            data_text_internal->movement_translation_horizontal_background = nullptr;
        }

        if (data_text_internal->movement_type_vertical != nullptr)
        {
            delete data_text_internal->movement_type_vertical;
            data_text_internal->movement_type_vertical = nullptr;
        }
        if (data_text_internal->movement_speed_vertical != nullptr)
        {
            delete data_text_internal->movement_speed_vertical;
            data_text_internal->movement_speed_vertical = nullptr;
        }
        if (data_text_internal->movement_threshold_vertical != nullptr)
        {
            delete data_text_internal->movement_threshold_vertical;
            data_text_internal->movement_threshold_vertical = nullptr;
        }
        if (data_text_internal->movement_translation_vertical != nullptr)
        {
            delete data_text_internal->movement_translation_vertical;
            data_text_internal->movement_translation_vertical = nullptr;
        }

        if (data_text_internal->movement_type_vertical_background != nullptr)
        {
            delete data_text_internal->movement_type_vertical_background;
            data_text_internal->movement_type_vertical_background = nullptr;
        }
        if (data_text_internal->movement_speed_vertical_background != nullptr)
        {
            delete data_text_internal->movement_speed_vertical_background;
            data_text_internal->movement_speed_vertical_background = nullptr;
        }
        if (data_text_internal->movement_threshold_vertical_background != nullptr)
        {
            delete data_text_internal->movement_threshold_vertical_background;
            data_text_internal->movement_threshold_vertical_background = nullptr;
        }
        if (data_text_internal->movement_translation_vertical_background != nullptr)
        {
            delete data_text_internal->movement_translation_vertical_background;
            data_text_internal->movement_translation_vertical_background = nullptr;
        }

        delete data_text_internal;
        data_text_internal = nullptr;
    }
}

void set_text_blink_turn_on_off(int index_text, bool flag_blink_turn_on_off)
{
    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        auto it_text_internal = data_text->map_text_internal.find(index_text);
        if (it_text_internal != data_text->map_text_internal.end())
        {
            pst_text_internal data_text_internal = it_text_internal->second;

            data_text_internal->flag_blink = flag_blink_turn_on_off;
        }
    }
}

void set_text_blink_interval(int interval_blink_in_miliseconds)
{
    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        data_text->interval_blink_in_miliseconds = interval_blink_in_miliseconds;
    }
}

void set_text_blink_duration(int duration_blink_in_miliseconds)
{
    for (auto it_text = _map_text.begin(); it_text != _map_text.end(); it_text++)
    {
        pst_text data_text = it_text->second;

        data_text->duration_blink_in_miliseconds = duration_blink_in_miliseconds;
    }
}

void cef_initialize()
{
    int result = 0;
    result = cpp_cef_wrapper_execute_process(hInst);
    if (result >= 0)
    {
        return;
    }

    result = cpp_cef_wrapper_initialize(hInst, _cef_browser_subprocess_path.c_str(), _cef_browser_subprocess_path.size());
    if (result != 0)
    {
        return;
    }

    _flag_cef_succeed = true;
}

void cef_shutdown()
{
    cpp_cef_wrapper_shutdown();
}

void create_cef(UINT index_cef, HWND handle, std::string url, RECT rect)
{
    pst_cef data_cef = new st_cef();

    data_cef->index_cef = index_cef;
    data_cef->handle = handle;
    data_cef->url = url;
    data_cef->rect_base = rect;

    data_cef->index_map_input = 0;
    data_cef->index_map_output = 0;
    
    data_cef->mutex_deque_index_unref = new std::mutex();
    data_cef->deque_index_unref = new std::deque<int>();

    data_cef->flag_thread_cef_unref = true;
    data_cef->thread_cef_unref = new std::thread(thread_cef_unref, data_cef);

    data_cef->instance = cpp_cef_wrapper_create(data_cef->handle, data_cef->url.c_str(), data_cef->url.size(), data_cef->rect_base);
    
    for (auto it_window = _map_window.begin(); it_window != _map_window.end(); it_window++)
    {
        pst_window data_window = it_window->second;

        if (data_cef->rect_base.left < data_window->rect.right
            && data_cef->rect_base.top < data_window->rect.bottom
            && data_cef->rect_base.right > data_window->rect.left
            && data_cef->rect_base.bottom > data_window->rect.top
            )
        {
            pst_cef_device data_cef_device = new st_cef_device();
            data_cef_device->index_cef_device = data_window->device_index;

            data_cef_device->rect_target.left = data_cef->rect_base.left >= data_window->rect.left ? data_cef->rect_base.left : data_window->rect.left;
            data_cef_device->rect_target.top = data_cef->rect_base.top >= data_window->rect.top ? data_cef->rect_base.top : data_window->rect.top;
            data_cef_device->rect_target.right = data_cef->rect_base.right <= data_window->rect.right ? data_cef->rect_base.right : data_window->rect.right;
            data_cef_device->rect_target.bottom = data_cef->rect_base.bottom <= data_window->rect.bottom ? data_cef->rect_base.bottom : data_window->rect.bottom;

            normalize_rect(data_window->rect, data_cef_device->rect_target, data_cef_device->rect_normalized);

            data_cef_device->index_srv_upload = 0;
            data_cef_device->index_srv_draw = 0;
            data_cef_device->flag_texture_created = false;
            data_cef_device->count_cef_device = data_window->counter_cef_create;
            data_window->counter_cef_create += 1;

            data_cef->map_cef_device.insert({ data_cef_device->index_cef_device, data_cef_device });
        }
    }

    data_cef->map_cef_data_paint = new std::map<UINT, pst_cef_data_paint>();
    data_cef->mutex_map_cef_data_paint = new std::mutex();

    for (UINT i = 0; i < _count_texture_store; i++)
    {
        data_cef->map_cef_data_paint->insert({ i, new st_cef_data_paint() });
    }

    data_cef->deque_index_used = new std::deque<int>();
    data_cef->mutex_deque_index_used = new std::mutex();

    _map_cef.insert({ data_cef->index_cef, data_cef });
}

void delete_cef(UINT index_cef)
{
    auto it_cef = _map_cef.find(index_cef);

    if (it_cef != _map_cef.end())
    {
        _mutex_deque_cef_delete.lock();
        _deque_cef_delete.push_back(it_cef->first);
        _mutex_deque_cef_delete.unlock();
    }
}

void delete_cefs()
{
    for (auto it_cef = _map_cef.begin(); it_cef != _map_cef.end(); it_cef++)
    {
        _mutex_deque_cef_delete.lock();
        _deque_cef_delete.push_back(it_cef->first);
        _mutex_deque_cef_delete.unlock();
    }
}

void delete_cef_internal(pst_cef data_cef)
{
    for (auto it_cef_device = data_cef->map_cef_device.begin(); it_cef_device != data_cef->map_cef_device.end(); )
    {
        pst_cef_device data_cef_device = it_cef_device->second;

        delete data_cef_device;
        data_cef_device = nullptr;

        it_cef_device = data_cef->map_cef_device.erase(it_cef_device);
    }

    cpp_cef_wrapper_close_browser(data_cef->instance);

    cpp_cef_wrapper_delete(data_cef->instance);

    data_cef->mutex_map_cef_data_paint->lock();
    for (auto it_map_cef_data_paint = data_cef->map_cef_data_paint->begin(); it_map_cef_data_paint != data_cef->map_cef_data_paint->end();)
    {
        pst_cef_data_paint data_cef_data_paint = it_map_cef_data_paint->second;

        cpp_cef_wrapper_delete_buffer(data_cef->instance, data_cef_data_paint->buffer);
        if (data_cef_data_paint->nv12_buffer != nullptr)
        {
            delete[] data_cef_data_paint->nv12_buffer;
            data_cef_data_paint->nv12_buffer = nullptr;
        }

        delete data_cef_data_paint;
        data_cef_data_paint = nullptr;

        it_map_cef_data_paint = data_cef->map_cef_data_paint->erase(it_map_cef_data_paint);
    }
    delete data_cef->map_cef_data_paint;
    data_cef->map_cef_data_paint = nullptr;
    data_cef->mutex_map_cef_data_paint->unlock();

    if (data_cef->thread_cef_unref->joinable())
    {
        data_cef->flag_thread_cef_unref = false;
        data_cef->thread_cef_unref->join();
    }
    delete data_cef->thread_cef_unref;
    data_cef->thread_cef_unref = nullptr;

    delete data_cef->mutex_map_cef_data_paint;
    data_cef->mutex_map_cef_data_paint = nullptr;

    data_cef->mutex_deque_index_used->lock();
    data_cef->deque_index_used->clear();
    delete data_cef->deque_index_used;
    data_cef->deque_index_used = nullptr;
    data_cef->mutex_deque_index_used->unlock();

    delete data_cef->mutex_deque_index_used;
    data_cef->mutex_deque_index_used = nullptr;

    data_cef->mutex_deque_index_unref->lock();
    data_cef->deque_index_unref->clear();
    delete data_cef->deque_index_unref;
    data_cef->deque_index_unref = nullptr;
    data_cef->mutex_deque_index_unref->unlock();

    delete data_cef->mutex_deque_index_unref;
    data_cef->mutex_deque_index_unref = nullptr;

    delete data_cef;
    data_cef = nullptr;
}

void thread_cef_unref(pst_cef data_cef)
{
    while (data_cef->flag_thread_cef_unref)
    {
        bool flag_deque_index_unref_is_empty = false;
        {
            std::lock_guard<std::mutex> lk(*data_cef->mutex_deque_index_unref);
            flag_deque_index_unref_is_empty = data_cef->deque_index_unref->empty();
        }

        if (flag_deque_index_unref_is_empty)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time_processing));
            continue;
        }

        int index_unref = -1;
        {
            std::lock_guard<std::mutex> lk(*data_cef->mutex_deque_index_unref);
            index_unref = data_cef->deque_index_unref->front();
            data_cef->deque_index_unref->pop_front();
        }

        if (data_cef->map_cef_data_paint->find(index_unref)->second->buffer != nullptr)
        {
            cpp_cef_wrapper_delete_buffer(data_cef->instance, data_cef->map_cef_data_paint->find(index_unref)->second->buffer);
            data_cef->map_cef_data_paint->find(index_unref)->second->buffer = nullptr;
            delete[] data_cef->map_cef_data_paint->find(index_unref)->second->nv12_buffer;
            data_cef->map_cef_data_paint->find(index_unref)->second->nv12_buffer = nullptr;
        }

        if (is_queue_empty(data_cef->index_map_input, data_cef->index_map_output) == false)
        {
            data_cef->index_map_output += 1;
            if (!(data_cef->index_map_output < _count_texture_store))
            {
                data_cef->index_map_output = 0;
            }
        }
    }
}
