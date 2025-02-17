#pragma once

#include "MediaHeaders.h"

/// <summary>
/// 하드웨어 디코딩에 사용할 pixel format을 선택하는 콜백 함수 구현
/// </summary>
/// <param name="ctx"></param>
/// <param name=""></param>
/// <returns></returns>
static enum AVPixelFormat get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts);
/// <summary>
/// 하드웨어 디코딩에 사용할 pixel format 변수
/// </summary>
static AVPixelFormat _hw_pix_fmt;

class FFmpegCore
{
public:
    FFmpegCore();

    bool initialize(CALLBACK_PTR cb);
    void shutdown();

    /// <summary>
    /// _file_path 변수에 설정된 url로 스트림 연결 시도
    /// </summary>
    /// <returns> 0: 정상, 0 이외: 비정상 </returns>
    int open_file();

    void play_start(void* connection);
    void play_pause(void* connection);
    void play_stop(void* connection);

    void jump_forward(void* connection);
    void jump_backwards(void* connection);

    s32 get_frame(AVFrame*& frame);
    s32 frame_to_next();
    s32 frame_to_next_non_waiting();
    s32 check_frame_to_next_sync_group();

    void scene_index(u32 scene_index) { _scene_index = scene_index; }
    void rect(RECT rect) { _rect = rect; }
    void sync_group_index(u32 sync_group_index) { _sync_group_index = sync_group_index; }
    void sync_group_count(u16 sync_group_count) { _sync_group_count = sync_group_count; }
    void sync_group_time_started() { _time_started = 0.0; }
    void file_path(std::string path) { _file_path = path; }

    void sync_group_frame_numbering();

    void seek_pts(s64 pts);

    void frame_numbering();

    void hw_decode(bool hw_decode) { _hw_decode = hw_decode; }
    void hw_device_type(int hw_device_type) { _hw_device_type = (AVHWDeviceType)hw_device_type; }
    void hw_decode_adapter_index(int hw_decode_adapter_index);

    void set_repeat_flag();
    void unset_repeat_flag();

    void repeat_sync_group();

    void set_scale(bool scale);

    void thread_repeat();

    void start_thread_repeat();

    void get_timebase(AVRational& timebase);

    int is_realtime();

    void get_is_realtime(bool& is_realtime);
    void get_flag_succeed_open_input(bool& flag_succeed_open_input);
    void get_flag_play_started(bool& flag_play_started);

    void initialize_small(CALLBACK_PTR cb);
    void shutdown_small();

private:

    const int _thread_count_fhd = 4;
    const int _thread_count_4k = 8;
    const int _thread_count_4k_higher = 16;

    int _logical_processor_count;
    int _logical_processor_count_half;

    void* _connection_play_start;

    bool    _first_decode;
    int64_t _previous_frame_pts;
    double _time_started;
    
    bool _sync_group_frame_numbering;

    uint32_t _frame_numbering;  // sync_group_frame_numbering 호출되면 올라감
    uint32_t _frame_count;  // 다음 frame으로 이동할 때 올라감

    bool _repeat_flag;

#pragma region HW_Decode
    bool _hw_decode;
    AVHWDeviceType _hw_device_type = AVHWDeviceType::AV_HWDEVICE_TYPE_NONE;
    AVBufferRef* _hw_device_ctx;
    int _hw_decode_adapter_index;

    AVFrame* _hw_frame;
#pragma endregion

#pragma region Read
    void read();
    e_error_type read_internal(__out AVPacket*& packet);
#pragma endregion

#pragma region Decode
    void open_codec();
    void decode();
    e_error_type decode_internal(__in AVPacket* packet, __out AVFrame*& frame);
    e_error_type flush_codec();
#pragma endregion

#pragma region Pause
    void pause();
    void play_continue();

    bool                _pause_flag;
    std::mutex          _pause_mutex;
#pragma endregion

#pragma region Seek
    void set_timestamp(s64 pts);

    bool                _seek_flag;

    bool                _seek_ready_flag_reader;
    std::mutex          _seek_mutex_reader;
    std::condition_variable _seek_condition_reader;
    bool                _seek_flag_reader;

    bool                _seek_ready_flag_decoder;
    std::mutex          _seek_mutex_decoder;
    std::condition_variable _seek_condition_decoder;
    bool                _seek_flag_decoder;
#pragma endregion

    AVFormatContext*    _format_ctx;
    AVCodecContext*     _codec_ctx;

    int                 _stream_index = -1;

    const AVCodec*      _codec;

    std::string         _file_path;

    AVDictionary*       _option;

    AVRational          _time_base;
    double              _time_base_d;
    s64                 _duration;  // 스트림 총 길이
    s64                 _duration_frame; // 1 Frame의 길이
    double              _duration_frame_half;    // 1 Frame의 길이 절반
    s64                 _start_time;

    bool                _read_flag;
    bool                _decode_flag;

    std::thread         _read_thread;
    std::thread         _decode_thread;


    bool                _codec_opened;
    bool                _eof_read;
    bool                _eof_decode;
    bool                _eof_read2;

    std::mutex          _play_mutex;


    CALLBACK_PTR _callback_ffmpeg;
    u32 _scene_index;
    RECT _rect;
    u32 _sync_group_index;
    u16 _sync_group_count;
    u16 _url_size;

#pragma region Scale
    void scale(AVFrame* frame);

    AVPixelFormat       _scale_dest_format;
    AVFrame*            _scale_frame;
    SwsContext*         _sws_ctx;

    int                 _scale_alloc_size;

    bool                _scale;
#pragma endregion

#pragma region circular queue

    bool is_full_packet_queue();
    bool is_empty_packet_queue();
    bool is_full_frame_queue();
    bool is_empty_frame_queue();

    /// <summary>
    /// 
    /// </summary>
    /// <param name="packet"></param>
    /// <returns> queue_is_full or ok </returns>
    e_error_type input_packet(__in AVPacket* packet);
    /// <summary>
    /// 
    /// </summary>
    /// <param name="packet"></param>
    /// <returns> queue_is_empty or ok </returns>
    e_error_type output_packet(__out AVPacket*& packet);
    /// <summary>
    /// 
    /// </summary>
    /// <param name="frame"></param>
    /// <returns> queue_is_full or ok </returns>
    e_error_type input_frame(__in AVFrame* frame);
    /// <summary>
    /// 
    /// </summary>
    /// <param name="frame"></param>
    /// <returns> queue_is_empty or ok </returns>
    e_error_type output_frame(__out AVFrame*& frame);

    e_error_type output_frame(__out AVFrame*& frame, s32& index);

    u32 initialize_packet_queue();
    u32 initialize_frame_queue();
    void failed_free_packet_queue(u32 size);
    void failed_free_frame_queue(u32 size);
    void clear_packet_queue();
    void clear_frame_queue();

    s32 get_packet_queue_size();
    s32 get_frame_queue_size();

    AVPacket* _packet_queue[_packet_queue_size];
    s32                 _input_packet_index;
    s32                 _output_packet_index;
    std::mutex          _packet_mutex;
    bool                _packet_queue_free;

    AVFrame* _frame_queue[_frame_queue_size];
    s32                 _input_frame_index;
    s32                 _output_frame_index;
    std::mutex          _frame_mutex;
    bool                _frame_queue_free;

#pragma endregion

    void create_hw_codec();

    bool _flag_is_realtime;
    bool _flag_succeed_open_input;
    bool _flag_play_started;

};
