#pragma once

#include "MediaHeaders.h"

class FFmpegCore
{
public:
    bool initialize(CALLBACK_INT32_UINT16_PTR_UINT16 cb, u32 scene_index);
    void shutdown();

    int open_file();

    void play_start(void* connection);
    void play_pause(void* connection);
    void play_stop(void* connection);

    void jump_forward(void* connection);
    void jump_backwards(void* connection);

    s32 get_frame(AVFrame*& frame);
    s32 frame_to_next();

    void file_path(std::string path) { _file_path = path; }

    void seek_pts(s64 pts);

private:

    const int _thread_count_fhd = 4;
    const int _thread_count_4k = 8;
    const int _thread_count_4k_higher = 16;

    int _logical_processor_count = 0;
    int _logical_processor_count_half = 0;

    void* _connection_play_start = nullptr;

    bool    _first_decode = false;
    int64_t _previous_frame_pts = 0;
    double _time_started = 0.0f;

#pragma region Read
    void read();
    error_type read_internal(__out AVPacket*& packet);
#pragma endregion

#pragma region Decode
    void open_codec();
    void decode();
    error_type decode_internal(__in AVPacket* packet, __out AVFrame*& frame);
    error_type flush_codec();
#pragma endregion

#pragma region Pause
    void pause();
    void play_continue();

    bool                _pause_flag = false;
    std::mutex          _pause_mutex;
#pragma endregion

#pragma region Seek
    void set_timestamp(s64 pts);

    bool                _seek_flag = false;

    bool                _seek_ready_flag_reader = false;
    std::mutex          _seek_mutex_reader;
    std::condition_variable _seek_condition_reader;

    bool                _seek_ready_flag_decoder = false;
    std::mutex          _seek_mutex_decoder;
    std::condition_variable _seek_condition_decoder;
#pragma endregion

    AVFormatContext*    _format_ctx = nullptr;
    AVCodecContext*     _codec_ctx = nullptr;
    s32                 _stream_index = -1;

    std::string         _file_path;

    AVDictionary*       _option = nullptr;

    AVRational          _time_base = { 0, 1 };
    double              _time_base_d = 0.0f;
    s64                 _duration = 0;  // 스트림 총 길이
    s64                 _duration_frame = 0; // 1 Frame의 길이
    double              _duration_frame_half = 0.0f;    // 1 Frame의 길이 절반
    s64                 _start_time = 0;

    bool                _read_flag = false;
    bool                _decode_flag = false;

    std::thread         _read_thread;
    std::thread         _decode_thread;


    bool                _codec_opened = false;
    bool                _eof_read = false;
    bool                _eof_decode = false;

    std::mutex          _play_mutex;


    CALLBACK_INT32_UINT16_PTR_UINT16 _callback_ffmpeg = nullptr;
    u32 _scene_index = 0;

#pragma region Scale
    void scale(AVFrame* frame);

    AVPixelFormat       _scale_dest_format = AVPixelFormat::AV_PIX_FMT_YUV420P;
    AVFrame*            _scale_frame = nullptr;
    SwsContext*         _sws_ctx = nullptr;

    int                 _scale_alloc_size = 0;
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
    error_type input_packet(__in AVPacket* packet);
    /// <summary>
    /// 
    /// </summary>
    /// <param name="packet"></param>
    /// <returns> queue_is_empty or ok </returns>
    error_type output_packet(__out AVPacket*& packet);
    /// <summary>
    /// 
    /// </summary>
    /// <param name="frame"></param>
    /// <returns> queue_is_full or ok </returns>
    error_type input_frame(__in AVFrame* frame);
    /// <summary>
    /// 
    /// </summary>
    /// <param name="frame"></param>
    /// <returns> queue_is_empty or ok </returns>
    error_type output_frame(__out AVFrame*& frame);

    error_type output_frame(__out AVFrame *& frame, s32& index);

    u32 initialize_packet_queue();
    u32 initialize_frame_queue();
    void failed_free_packet_queue(u32 size);
    void failed_free_frame_queue(u32 size);
    void clear_packet_queue();
    void clear_frame_queue();

    s32 get_packet_queue_size();
    s32 get_frame_queue_size();

    AVPacket*           _packet_queue[_packet_queue_size];
    s32                 _input_packet_index = 0;
    s32                 _output_packet_index = 0;
    std::mutex          _packet_mutex;
    bool                _packet_queue_free = false;

    AVFrame*            _frame_queue[_frame_queue_size];
    s32                 _input_frame_index = 0;
    s32                 _output_frame_index = 0;
    std::mutex          _frame_mutex;
    bool                _frame_queue_free = false;

#pragma endregion

};
